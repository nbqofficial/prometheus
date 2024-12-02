// DRIVER

#pragma once

#include "../helper/helper.h"

typedef struct _DEVICE_EXTENSION
{
    PDEVICE_OBJECT  Self;
    PDEVICE_OBJECT  TrueClassDevice;
    PDEVICE_OBJECT  TopPort;
    PDEVICE_OBJECT  PDO;
    IO_REMOVE_LOCK  RemoveLock;
    BOOLEAN         PnP;
    BOOLEAN         Started;
    BOOLEAN OkayToLogOverflow;
    KSPIN_LOCK WaitWakeSpinLock;
    ULONG TrustedSubsystemCount;
    ULONG InputCount;
    UNICODE_STRING  SymbolicLinkName;
    PMOUSE_INPUT_DATA InputData;
    PMOUSE_INPUT_DATA DataIn;
    PMOUSE_INPUT_DATA DataOut;
    MOUSE_ATTRIBUTES  MouseAttributes;
    KSPIN_LOCK SpinLock;
    LIST_ENTRY ReadQueue;
    ULONG SequenceNumber;
    DEVICE_POWER_STATE DeviceState;
    SYSTEM_POWER_STATE SystemState;
    ULONG UnitId;
    WMILIB_CONTEXT WmiLibInfo;
    DEVICE_POWER_STATE SystemToDeviceState[PowerSystemHibernate];
    DEVICE_POWER_STATE MinDeviceWakeState;
    SYSTEM_POWER_STATE MinSystemWakeState;
    PIRP WaitWakeIrp;
    PIRP ExtraWaitWakeIrp;
    PVOID TargetNotifyHandle;
    LIST_ENTRY Link;
    PFILE_OBJECT File;
    BOOLEAN Enabled;
    BOOLEAN WaitWakeEnabled;
    BOOLEAN SurpriseRemoved;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

typedef struct _MOUSE_OBJECT
{
	PDEVICE_OBJECT mouse_device;
} MOUSE_OBJECT, * PMOUSE_OBJECT;

namespace mouse
{
    PIRP mouse_class_dequeue_read(PDEVICE_EXTENSION device_extension);

    VOID mouse_class_service_callback(PDEVICE_OBJECT device_object, PMOUSE_INPUT_DATA input_data_start, PMOUSE_INPUT_DATA input_data_end);

	NTSTATUS init_mouse(PMOUSE_OBJECT mouse_obj);

	bool mouse_event(MOUSE_OBJECT mouse_obj, long x, long y, unsigned short button_flags);
}