#include "driver.h"
// #include <>
// #include <dos.h>
// #include <random>
// #include <ctime>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, PortIOQueueInitialize)
#pragma alloc_text (PAGE, PortIOEvtIoDeviceControl)
#pragma alloc_text (PAGE, PortIOIoctlReadPort)
#pragma alloc_text (PAGE, PortIOIoctlWritePort)

#pragma alloc_text (PAGE, PortIOEvtAsynchRead)
#endif

NTSTATUS
PortIOQueueInitialize(
    __in WDFDEVICE Device
    )
{
    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG    queueConfig;

    PAGED_CODE();

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
        &queueConfig,
        WdfIoQueueDispatchParallel
        // WdfIoQueueDispatchSequential
        );

    queueConfig.EvtIoRead = PortIOEvtAsynchRead;

    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );

    if( !NT_SUCCESS(status) ) {
        KdPrint(("WdfIoQueueCreate failed 0x%x\n",status));
        return status;
    }

    return status;
}

VOID
PortIOEvtIoDeviceControl(
    __in WDFQUEUE     Queue,
    __in WDFREQUEST   Request,
    __in size_t       OutputBufferLength,
    __in size_t       InputBufferLength,
    __in ULONG        IoControlCode
    )
{

    PDEVICE_CONTEXT devContext = NULL;
    WDFDEVICE device;
    NTSTATUS status;

    PAGED_CODE();
    
    device = WdfIoQueueGetDevice(Queue);
    
    devContext = PortIOGetDeviceContext(device);
    
    switch(IoControlCode){
        case IOCTL_GPD_READ_PORT_UCHAR:
        case IOCTL_GPD_READ_PORT_USHORT:
        case IOCTL_GPD_READ_PORT_ULONG:
            PortIOIoctlReadPort(devContext,
                                Request,
                                OutputBufferLength,
                                InputBufferLength,
                                IoControlCode);
            break;


        case IOCTL_GPD_WRITE_PORT_UCHAR:
        case IOCTL_GPD_WRITE_PORT_USHORT:
        case IOCTL_GPD_WRITE_PORT_ULONG:    
            PortIOIoctlWritePort(devContext,
                                 Request,
                                 OutputBufferLength,
                                 InputBufferLength,
                                 IoControlCode);
            break;

        default:
            status = STATUS_INVALID_PARAMETER;
            WdfRequestComplete(Request, status);

            break;
    }
        
}

VOID
PortIOIoctlReadPort(
    __in PDEVICE_CONTEXT devContext,
    __in WDFREQUEST Request,
    __in size_t OutBufferSize,
    __in size_t InBufferSize,
    __in ULONG IoctlCode)   
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG minDataBufferSize; // minimum output buffer size
    ULONG nPort; // Port number to read                                                
    PULONG pOutBuffer;  // Pointer to transfer buffer
                                   //  (treated as an array of longs).
    PULONG pInBuffer;

    UNREFERENCED_PARAMETER(OutBufferSize);
    UNREFERENCED_PARAMETER(InBufferSize);
    
    PAGED_CODE();
                                   
    switch (IoctlCode){
        case IOCTL_GPD_READ_PORT_UCHAR:
            minDataBufferSize = sizeof(UCHAR);
            break;
        case IOCTL_GPD_READ_PORT_USHORT:
            minDataBufferSize = sizeof(USHORT);
            break;
        case IOCTL_GPD_READ_PORT_ULONG:
            minDataBufferSize = sizeof(ULONG);
            break;
        default:
            status = STATUS_INVALID_PARAMETER;
            goto exit;    
    }
    status = WdfRequestRetrieveInputBuffer(
                 Request,
                 sizeof(ULONG), 
                 &((PVOID) pInBuffer),
                 NULL);
    if (!NT_SUCCESS(status)) {
        goto exit;
        
    }
    status = WdfRequestRetrieveOutputBuffer(
                                                                Request,
                                                                minDataBufferSize,
                                                                &((PVOID) pOutBuffer),
                                                                NULL);
    if (!NT_SUCCESS(status)) {
        goto exit;        
    }
                                                                
    nPort = *pInBuffer;

    if (nPort >= devContext -> PortCount ||
        (nPort + minDataBufferSize) > devContext -> PortCount ||
        (((ULONG_PTR)devContext->PortBase + nPort) & (minDataBufferSize- 1)) != 0){

        status = STATUS_ACCESS_VIOLATION;
        goto exit;
    }

    if (devContext->PortMemoryType == 1){
        // Address is in I/O space

        switch (IoctlCode){
        case IOCTL_GPD_READ_PORT_UCHAR:
            *(PUCHAR)pOutBuffer = READ_PORT_UCHAR(
                            (PUCHAR)((ULONG_PTR)devContext->PortBase + nPort) );
            break;
        case IOCTL_GPD_READ_PORT_USHORT:
            *(PUSHORT)pOutBuffer = READ_PORT_USHORT(
                            (PUSHORT)((ULONG_PTR)devContext->PortBase + nPort) );
            break;
        case IOCTL_GPD_READ_PORT_ULONG:
            *(PULONG)pOutBuffer = READ_PORT_ULONG(
                            (PULONG)((ULONG_PTR)devContext->PortBase + nPort) );
            break;

        default:
            status =  STATUS_INVALID_PARAMETER;
            goto exit;


        }
    }
    else if (devContext->PortMemoryType == 0)
    {
        // Address is in Memory space

        switch (IoctlCode)
        {
        case IOCTL_GPD_READ_PORT_UCHAR:
            *(PUCHAR)pOutBuffer = READ_REGISTER_UCHAR(
                            (PUCHAR)((ULONG_PTR)devContext->PortBase + nPort) );
            break;
        case IOCTL_GPD_READ_PORT_USHORT:
            *(PUSHORT)pOutBuffer = READ_REGISTER_USHORT(
                            (PUSHORT)((ULONG_PTR)devContext->PortBase + nPort) );
            break;
        case IOCTL_GPD_READ_PORT_ULONG:
            *(PULONG)pOutBuffer = READ_REGISTER_ULONG(
                            (PULONG)((ULONG_PTR)devContext->PortBase + nPort) );
            break;
        default:
            status = STATUS_INVALID_PARAMETER;
            goto exit;

        }
    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    WdfRequestCompleteWithInformation(Request, status, minDataBufferSize);
    return;

exit:

    WdfRequestComplete(Request, status);
}

VOID
PortIOIoctlWritePort(
    __in PDEVICE_CONTEXT devContext,
    __in WDFREQUEST Request,
    __in size_t OutBufferSize,
    __in size_t InBufferSize,
    __in ULONG IoctlCode)
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG minDataBufferSize; // minimum output buffer size
    ULONG nPort; // Port number to read                                                
    PULONG pInBuffer;
    
    UNREFERENCED_PARAMETER(OutBufferSize);
    UNREFERENCED_PARAMETER(InBufferSize);

    PAGED_CODE();
                                   
    switch (IoctlCode){
        case IOCTL_GPD_WRITE_PORT_UCHAR:
            minDataBufferSize = sizeof(UCHAR);
            break;
        case IOCTL_GPD_WRITE_PORT_USHORT:
            minDataBufferSize = sizeof(USHORT);
            break;
        case IOCTL_GPD_WRITE_PORT_ULONG:
            minDataBufferSize = sizeof(ULONG);
            break;
        default:
            status = STATUS_INVALID_PARAMETER;
            goto exit;    
    }
    status = WdfRequestRetrieveInputBuffer(
                Request,
                sizeof(ULONG), 
                &((PVOID) pInBuffer),
                NULL);
    if (!NT_SUCCESS(status)) {
        goto exit;
        
    }
    
    nPort = *pInBuffer++;

    if (nPort >= devContext -> PortCount ||
        (nPort + minDataBufferSize) > devContext -> PortCount ||
        (((ULONG_PTR)devContext->PortBase + nPort) & (minDataBufferSize- 1)) != 0){

        status = STATUS_ACCESS_VIOLATION;
        goto exit;
    }

    if (devContext->PortMemoryType == 1){
        // Address is in I/O space

        switch (IoctlCode){
        case IOCTL_GPD_WRITE_PORT_UCHAR:
            WRITE_PORT_UCHAR(
                (PUCHAR)((ULONG_PTR)devContext->PortBase + nPort),
                *(PUCHAR)pInBuffer);
            break;
        case IOCTL_GPD_WRITE_PORT_USHORT:
            WRITE_PORT_USHORT(
                (PUSHORT)((ULONG_PTR)devContext->PortBase + nPort),
                *(PUSHORT)pInBuffer);
            break;
        case IOCTL_GPD_WRITE_PORT_ULONG:
            WRITE_PORT_ULONG(
                (PULONG)((ULONG_PTR)devContext->PortBase + nPort),
                *(PULONG)pInBuffer);
            break;

        default:
            status =  STATUS_INVALID_PARAMETER;
            goto exit;
        }
    }
    else if (devContext->PortMemoryType == 0)
    {
        // Address is in Memory space

        switch (IoctlCode)
        {
        case IOCTL_GPD_WRITE_PORT_UCHAR:
            WRITE_REGISTER_UCHAR(
                    (PUCHAR)((ULONG_PTR)devContext->PortBase + nPort),
                    *(PUCHAR)pInBuffer);
            break;
        case IOCTL_GPD_WRITE_PORT_USHORT:
            WRITE_REGISTER_USHORT(
                    (PUSHORT)((ULONG_PTR)devContext->PortBase + nPort),
                    *(PUSHORT)pInBuffer );
            break;
        case IOCTL_GPD_WRITE_PORT_ULONG:
            WRITE_REGISTER_ULONG(
                    (PULONG)((ULONG_PTR)devContext->PortBase + nPort),
                    *(PULONG)pInBuffer );
            break;
        default:
            status = STATUS_INVALID_PARAMETER;
            goto exit;

        }
    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    WdfRequestCompleteWithInformation(Request, status, minDataBufferSize);
    return;
    
exit:
    WdfRequestComplete(Request, status);
    return;
}



VOID 
PortIOEvtAsynchRead(
    WDFQUEUE Queue,
    WDFREQUEST Request,
    size_t Length)
{
    NTSTATUS status = STATUS_SUCCESS;
    UCHAR outputResult = 0;
    UCHAR check = 0;
    const int portBase = 0x3f8;
    int i = 0;
    PUCHAR outputBuffer;
    ULONG dataBufferSize;

    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Length);

    PAGED_CODE();

    KdPrint(("PortIOEvtAsynchRead started\n"));

    // srand(time(0));
    // Sleep(2000);
    status = WdfRequestRetrieveOutputBuffer(
        Request,
        sizeof(UCHAR),
        &((PVOID)outputBuffer),
        NULL);

    // __asm { //проверка готовности данных
    //     mov dx, 0x3f8; //перемещаемся по адресу СОМ-1
    //     add dx, 5; //смещаемся на регистр состояния линии
    //     in al, dx; // считываем данные 
    //     mov check, al; // перемещаем их в переменную check
    // }
    for (i = 0; i < 2500000; ++i)
    {
        check = READ_PORT_UCHAR((PUCHAR)(portBase + 5));
        if (check & 1)
            break;

    }

    // for (i = 0; i < 2500000; ++i)
    //     check = READ_PORT_UCHAR((PUCHAR)(portBase + 5));
    // delay(3000);
    if (!(check & 1)) {
        KdPrint(("There is no ready data\n"));
        WdfRequestComplete(Request, status);
        return;
    }

    // __asm { // чтение данных из порта
    //     mov dx, 0x3f8; // перемещаемся по адресу СОМ-1
    //     in al, dx; // считываем данные
    //     mov outputResult, al; // записываем их в переменную
    // }
    outputResult = READ_PORT_UCHAR((PUCHAR)(portBase));

    KdPrint(("Symbol: %c \n", outputResult));

    *(PUCHAR)outputBuffer = (UCHAR)outputResult;
    dataBufferSize = sizeof(UCHAR);

    WdfRequestCompleteWithInformation(Request, status, dataBufferSize);
    // printf("read operation completed");
    return;
}
