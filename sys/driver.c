#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, PortIOEvtDeviceAdd)
#endif


NTSTATUS
DriverEntry(
    __in PDRIVER_OBJECT  DriverObject,
    __in PUNICODE_STRING RegistryPath
    )
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;

    WDF_DRIVER_CONFIG_INIT(&config,
                        PortIOEvtDeviceAdd
                        );

    status = WdfDriverCreate(DriverObject,
                            RegistryPath,
                            WDF_NO_OBJECT_ATTRIBUTES,
                            &config,
                            WDF_NO_HANDLE);
    if (!NT_SUCCESS(status)) {
        KdPrint(("Error: WdfDriverCreate failed 0x%x\n", status));
        return status;
    }

    return status;
}

NTSTATUS
PortIOEvtDeviceAdd(
    __in WDFDRIVER       Driver,
    __inout PWDFDEVICE_INIT DeviceInit
    )
{
    NTSTATUS status;

    const int portBase = 0x3f8;
    const int portLine = portBase + 3;
    const int portDivisor = portBase + 1;
    
    UNREFERENCED_PARAMETER(Driver);
    PAGED_CODE();

    KdPrint(("Enter PortIoDeviceAdd\n"));

    WRITE_PORT_UCHAR((PUCHAR)portLine, (UCHAR)0x80);
    WRITE_PORT_UCHAR((PUCHAR)portBase, (UCHAR)0x0C);
    WRITE_PORT_UCHAR((PUCHAR)portDivisor, (UCHAR)0x00);
    WRITE_PORT_UCHAR((PUCHAR)portLine, (UCHAR)0x03);

    status = PortIODeviceCreate(DeviceInit);
    KdPrint(("COM-port speed set as 9600\n"));
    return status;
}


