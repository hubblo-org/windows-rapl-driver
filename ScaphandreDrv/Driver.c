#include "Driver.h"
#include "msr.h"
#include <intrin.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

NTSTATUS DriverEntry(PDRIVER_OBJECT  DriverObject,
                     PUNICODE_STRING RegistryPath)
{
    PDEVICE_OBJECT device_object;
    UNICODE_STRING device_name;
    UNICODE_STRING sym_name;
    NTSTATUS status;

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
    memcpy(manufacturer + sizeof(unsigned __int32), &cpu_regs[3],
           sizeof(unsigned __int32));
    memcpy(manufacturer + 2 * sizeof(unsigned __int32), &cpu_regs[2],
           sizeof(unsigned __int32));

    if (!strncmp(manufacturer, "GenuineIntel", sizeof(manufacturer) - 1))
        machine_type = E_MACHINE_INTEL;
    else if (!strncmp(manufacturer, "AMDisbetter!", sizeof(manufacturer) - 1))
        machine_type = E_MACHINE_AMD;
    else if (!strncmp(manufacturer, "AuthenticAMD", sizeof(manufacturer) - 1))
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
    GROUP_AFFINITY affinity, old;
    PIO_STACK_LOCATION stackLoc;
    PROCESSOR_NUMBER pnumber;
    ULONGLONG msrResult;
    NTSTATUS ntStatus;
    struct data data;
    size_t inLength;

    stackLoc = irp->Tail.Overlay.CurrentStackLocation;
    inLength = stackLoc->Parameters.DeviceIoControl.InputBufferLength;

    DbgPrint("Received event for driver %s... \n", device->DriverObject->DriverName);

    if (inLength != sizeof(data)) {
        DbgPrint("Bad input length provided. Expected %zu bytes, got %zu.\n",
                 sizeof(data), inLength);
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        goto error;
    }

    /* Convert input data into structure */
    memcpy(&data, irp->AssociatedIrp.SystemBuffer, sizeof(data));
    if (validate_msr_lookup(data.msrRegister) != 0)
    {
        DbgPrint("Requested MSR register (%04x) access is not allowed!\n",
                 data.msrRegister);
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        goto error;
    }

    /* Run code on the specified socket */
    if ((ntStatus = KeGetProcessorNumberFromIndex(data.cpuIndex, &pnumber))
        != STATUS_SUCCESS) {
        DbgPrint("Failed to get processor info!\n");
        goto error;
    }

    /* Set affinity */
    memset(&affinity, 0, sizeof(GROUP_AFFINITY));
    affinity.Group = pnumber.Group;
    KeSetSystemGroupAffinityThread(&affinity, &old);

    /* Call readmsr instruction */
    msrResult = __readmsr(data.msrRegister);

    /* Restore affinity */
    KeRevertToUserGroupAffinityThread(&old);

    /* Save result */
    memcpy(irp->AssociatedIrp.SystemBuffer, &msrResult, sizeof(data));
    irp->IoStatus.Information = sizeof(data);
    ntStatus = STATUS_SUCCESS;

error:
    irp->IoStatus.Status = ntStatus;
    IofCompleteRequest(irp, IO_NO_INCREMENT);

    return ntStatus;
}
