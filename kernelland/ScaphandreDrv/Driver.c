#include "driver.h"
#include "msr.h"
#include <intrin.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS status;
    PDEVICE_OBJECT device_object;
    UNICODE_STRING device_name;
    UNICODE_STRING sym_name;

    DbgPrint("Registry path address: %p\n", RegistryPath);

    /* Assign driver callbacks */
    DriverObject->DriverUnload = DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = DispatchCleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;

    RtlInitUnicodeString(&device_name, DEVICE_NAME);
    RtlInitUnicodeString(&sym_name, DEVICE_SYM_NAME);
    status = IoCreateDevice(DriverObject, 0, &device_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);
    if (NT_SUCCESS(status)) {
        IoCreateSymbolicLink(&sym_name, &device_name);
    }
    return status;
}

void DriverUnload(PDRIVER_OBJECT driver)
{
    UNICODE_STRING sym_name;

    RtlInitUnicodeString(&sym_name, DEVICE_NAME);
    IoDeleteSymbolicLink(&sym_name);
    IoDeleteDevice(driver->DeviceObject);
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT device, PIRP irp)
{
    int cpu_regs[4];
    char manufacturer[13];

    DbgPrint("Creating driver %s... \n", device->DriverObject->DriverName);

    /* Lookup CPU information */
    memset(manufacturer, 0, sizeof(manufacturer));
    __cpuid(cpu_regs, 0);
    memcpy(manufacturer, &cpu_regs[1], sizeof(unsigned __int32));
    memcpy(manufacturer + sizeof(unsigned __int32), &cpu_regs[3], sizeof(unsigned __int32));
    memcpy(manufacturer + 2 * sizeof(unsigned __int32), &cpu_regs[2], sizeof(unsigned __int32));

    if (strncmp(manufacturer, "GenuineIntel", sizeof(manufacturer) - 1) == 0)
        machine_type = E_MACHINE_INTEL;
    else if (strncmp(manufacturer, "AMDisbetter!", sizeof(manufacturer) - 1) == 0)
        machine_type = E_MACHINE_AMD;
    else if (strncmp(manufacturer, "AuthenticAMD", sizeof(manufacturer) - 1) == 0)
        machine_type = E_MACHINE_AMD;
    else
        machine_type = E_MACHINE_UNK;

    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = STATUS_SUCCESS;
    IofCompleteRequest(irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS DispatchClose(PDEVICE_OBJECT device, PIRP irp)
{
    DbgPrint("Closing driver %s... \n", device->DriverObject->DriverName);
    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = STATUS_SUCCESS;
    IofCompleteRequest(irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS DispatchCleanup(PDEVICE_OBJECT device, PIRP irp)
{
    DbgPrint("Cleanup driver %s... \n", device->DriverObject->DriverName);
    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = STATUS_SUCCESS;
    IofCompleteRequest(irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT device, PIRP irp)
{
    NTSTATUS ntStatus;
    UINT32 msrRegister;
    ULONG inputBufferLength;
    ULONG outputBufferLength;
    ULONGLONG msrResult;
    PIO_STACK_LOCATION stackLocation;

    stackLocation = irp->Tail.Overlay.CurrentStackLocation;
    inputBufferLength = stackLocation->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;

    DbgPrint("Received event for driver %s... \n", device->DriverObject->DriverName);

    /* METHOD_BUFFERED */
    if (inputBufferLength == sizeof(ULONGLONG))
    {
        /* MSR register codes provided by userland must not exceed 8 bytes */
        memcpy(&msrRegister, irp->AssociatedIrp.SystemBuffer, sizeof(ULONGLONG));
        if (validate_msr_lookup(msrRegister) != 0)
        {
            DbgPrint("Requested MSR register (%08x) access is not allowed!\n", msrRegister);
            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        }
        else
        {
            /* Call readmsr instruction */
            msrResult = __readmsr(msrRegister);
            memcpy(irp->AssociatedIrp.SystemBuffer, &msrResult, sizeof(ULONGLONG));
            ntStatus = STATUS_SUCCESS;
            irp->IoStatus.Information = sizeof(ULONGLONG);
        }
    }
    else
    {
        DbgPrint("Bad input length provided. Expected %u bytes, got %u.\n", sizeof(ULONGLONG), inputBufferLength);
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    }

    irp->IoStatus.Status = ntStatus;
    IofCompleteRequest(irp, IO_NO_INCREMENT);

    return ntStatus;
}
