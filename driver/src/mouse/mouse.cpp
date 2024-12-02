// DRIVER

#include "mouse.h"

PIRP mouse::mouse_class_dequeue_read(PDEVICE_EXTENSION device_extension)
{
	PIRP next_irp = 0;
	while (!next_irp && !IsListEmpty(&device_extension->ReadQueue))
	{
		PLIST_ENTRY list_entry = RemoveHeadList(&device_extension->ReadQueue);
		next_irp = CONTAINING_RECORD(list_entry, IRP, Tail.Overlay.ListEntry);
	}
	return next_irp;
}

VOID mouse::mouse_class_service_callback(PDEVICE_OBJECT device_object, PMOUSE_INPUT_DATA input_data_start, PMOUSE_INPUT_DATA input_data_end)
{
	PDEVICE_EXTENSION device_extension = (PDEVICE_EXTENSION)device_object->DeviceExtension;
	ULONG bytes_in_queue = (ULONG)((PCHAR)input_data_end - (PCHAR)input_data_start);
	
	KeAcquireSpinLockAtDpcLevel(&device_extension->SpinLock);

	PIRP irp = mouse_class_dequeue_read(device_extension);
	if (irp)
	{
		PIO_STACK_LOCATION pio = IoGetCurrentIrpStackLocation(irp);
		ULONG bytes_to_move = pio->Parameters.Read.Length;
		ULONG move_size = bytes_in_queue < bytes_to_move ? bytes_in_queue : bytes_to_move;
		RtlMoveMemory(irp->AssociatedIrp.SystemBuffer, (PCHAR)input_data_start, move_size);
		irp->IoStatus.Status = STATUS_SUCCESS;
		irp->IoStatus.Information = move_size;
		pio->Parameters.Read.Length = move_size;
	}

	KeReleaseSpinLockFromDpcLevel(&device_extension->SpinLock);
	if (irp) { IoCompleteRequest(irp, IO_MOUSE_INCREMENT); }
}

NTSTATUS mouse::init_mouse(PMOUSE_OBJECT mouse_obj)
{
	wchar_t wa[256] = { 0 };
	for (int i = 0; i < 256; i++) { helper::append_stringw(wa, (wchar_t)i - 1); }

	UNICODE_STRING class_string;
	wchar_t wc_class_string[] = { wa[92], wa[68], wa[114], wa[105], wa[118], wa[101], wa[114], wa[92], wa[77], wa[111], wa[117], wa[67], wa[108], wa[97], wa[115], wa[115], '\0' }; // \Driver\MouClass
	RtlInitUnicodeString(&class_string, wc_class_string);

	PDRIVER_OBJECT class_driver_object = NULL;
	NTSTATUS status = ObReferenceObjectByName(&class_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&class_driver_object);
	if (!NT_SUCCESS(status)) { return status; }

	UNICODE_STRING hid_string;
	wchar_t wc_hid_string[] = { wa[92], wa[68], wa[114], wa[105], wa[118], wa[101], wa[114], wa[92], wa[77], wa[111], wa[117], wa[72], wa[73], wa[68], '\0' }; // \Driver\MouHID
	RtlInitUnicodeString(&hid_string, wc_hid_string);

	PDRIVER_OBJECT hid_driver_object = NULL;
	status = ObReferenceObjectByName(&hid_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&hid_driver_object);
	if (!NT_SUCCESS(status))
	{
		if (class_driver_object) { ObDereferenceObject(class_driver_object); }
		return status;
	}

	if (!mouse_obj->mouse_device)
	{
		PDEVICE_OBJECT target_device_object = class_driver_object->DeviceObject;
		while (target_device_object)
		{
			if (!target_device_object->NextDevice)
			{
				mouse_obj->mouse_device = target_device_object;
				break;
			}
			target_device_object = target_device_object->NextDevice;
		}
	}

	ObDereferenceObject(class_driver_object);
	ObDereferenceObject(hid_driver_object);

	memset(wc_class_string, 0, sizeof(wc_class_string));
	memset(wc_hid_string, 0, sizeof(wc_hid_string));
	memset(wa, 0, sizeof(wa));

	return STATUS_SUCCESS;
}

bool mouse::mouse_event(MOUSE_OBJECT mouse_obj, long x, long y, unsigned short button_flags)
{
	if (!mouse_obj.mouse_device) { return false; }

	MOUSE_INPUT_DATA mid = { 0 };

	mid.UnitId = 1;
	mid.LastX = x;
	mid.LastY = y;
	mid.ButtonFlags = button_flags;

	KIRQL irql;
	KeRaiseIrql(DISPATCH_LEVEL, &irql);
	mouse_class_service_callback(mouse_obj.mouse_device, &mid, (PMOUSE_INPUT_DATA)&mid + 1);
	KeLowerIrql(irql);

	return true;
}