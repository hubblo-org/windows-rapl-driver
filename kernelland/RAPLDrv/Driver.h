/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>

#include "device.h"
#include "queue.h"
#include "trace.h"

#define DEVICE_NAME L"\\Device\\ScaphandreDriver"
#define DEVICE_SYM_NAME L"\\DosDevices\\ScaphandreDriver"

EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD RAPLDrvEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP RAPLDrvEvtDriverContextCleanup;

EXTERN_C_END

void DriverUnload(PDRIVER_OBJECT driver);
NTSTATUS DispatchCreate(PDEVICE_OBJECT device, PIRP irp);
NTSTATUS DispatchClose(PDEVICE_OBJECT device, PIRP irp);
NTSTATUS DispatchCleanup(PDEVICE_OBJECT device, PIRP irp);
NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT device, PIRP irp);
