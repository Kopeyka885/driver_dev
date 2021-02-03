typedef struct _DEVICE_CONTEXT
{
    PVOID PortBase;       // base port address
    ULONG PortCount;      // Count of I/O addresses used.
    ULONG PortMemoryType;
    BOOLEAN PortWasMapped;  // If TRUE, we have to unmap on unload
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// Function to initialize the device and its callbacks
//
NTSTATUS
PortIODeviceCreate(
    PWDFDEVICE_INIT DeviceInit
    );

//
// Device events
//
EVT_WDF_DEVICE_PREPARE_HARDWARE PortIOEvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE PortIOEvtDeviceReleaseHardware;


