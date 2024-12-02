// DRIVER

#include "dispatch.h"

PDRIVER_DISPATCH original_dispatch = 0;
MOUSE_OBJECT mouse_obj = { 0 };
bool is_authorized = false;

NTSTATUS dispatch::handler(PDEVICE_OBJECT device_object, PIRP irp)
{
	NTSTATUS status = STATUS_INVALID_PARAMETER;

	KeEnterGuardedRegion();

	PIO_STACK_LOCATION pio = IoGetCurrentIrpStackLocation(irp);
	ULONG ioctl = pio->Parameters.DeviceIoControl.IoControlCode;

	PINFO_STRUCT info = (PINFO_STRUCT)irp->AssociatedIrp.SystemBuffer;

	if (ioctl == CODE_STATUS)
	{
		if (!is_authorized) { KeBugCheck(0x404); }
		info->status = 12345;
		status = STATUS_SUCCESS;
	}
	else if (ioctl == CODE_PROCESS_ID)
	{
		if (!is_authorized) { KeBugCheck(0x404); }
		info->process_id = memory::get_process_id_by_name(IoGetCurrentProcess(), info->process_name);
		if (info->process_id) { status = STATUS_SUCCESS; }
	}
	else if (ioctl == CODE_PML4)
	{
		if (!is_authorized) { KeBugCheck(0x404); }
		PEPROCESS process = 0;
		if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)info->process_id, &process)) && memory::is_process_running(process))
		{
			info->pml4 = memory::get_pml4(process);
			status = STATUS_SUCCESS;
		}
		if (process) { ObDereferenceObject(process); }
	}
	else if (ioctl == CODE_MODULE_BASE)
	{
		if (!is_authorized) { KeBugCheck(0x404); }
		PEPROCESS process = 0;

		UNICODE_STRING the_real_string;
		RtlInitUnicodeString(&the_real_string, info->module_name);

		if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)info->process_id, &process)) && memory::is_process_running(process))
		{
			void* b = memory::get_module_base(process, &the_real_string, info->is64bit);
			if (b)
			{
				info->module_base = b;
				status = STATUS_SUCCESS;
			}
		}
		if (process) { ObDereferenceObject(process); }
	}
	else if (ioctl == CODE_SECTION_BASE)
	{
		if (!is_authorized) { KeBugCheck(0x404); }
		PEPROCESS process = 0;
		if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)info->process_id, &process)) && memory::is_process_running(process))
		{
			void* b = memory::get_section_base(process);
			if (b)
			{
				info->module_base = b;
				status = STATUS_SUCCESS;
			}
		}
		if (process) { ObDereferenceObject(process); }
	}
	else if (ioctl == CODE_READ_MEMORY)
	{
		status = memory::read_memory(info->pml4, (uintptr_t)info->address, info->buffer, info->size);
	}
	else if (ioctl == CODE_WRITE_MEMORY)
	{
		status = memory::write_memory(info->pml4, (uintptr_t)info->address, info->buffer, info->size);
	}
	else if (ioctl == CODE_MOUSE_INIT)
	{
		if (!is_authorized) { KeBugCheck(0x404); }
		if (mouse_obj.mouse_device) { info->status = 12345; }
		else { if (NT_SUCCESS(mouse::init_mouse(&mouse_obj))) { info->status = 12345; } }
		status = STATUS_SUCCESS;
	}
	else if (ioctl == CODE_MOUSE_EVENT)
	{
		mouse::mouse_event(mouse_obj, info->x, info->y, info->button_flags);
		status = STATUS_SUCCESS;
	}
	else if (ioctl == CODE_FRAME_INIT)
	{
		if (!is_authorized) { KeBugCheck(0x404); }
		if (drawing::init(&gdi_funcs)) { info->status = 12345; }
		status = STATUS_SUCCESS;
	}
	else if (ioctl == CODE_FRAME_RECT)
	{
		HDC hdc = gdi_funcs.f_nt_user_get_dc(0);
		if (hdc)
		{
			HBRUSH brush = gdi_funcs.f_nt_gdi_create_solid_brush(RGB(info->r, info->g, info->b), 0);
			if (brush)
			{
				RECT r = { info->x, info->y, info->x + info->width, info->y + info->height };
				drawing::frame_rect(hdc, &r, brush, info->thickness);
				gdi_funcs.f_nt_gdi_delete_object_app(brush);
			}
			gdi_funcs.f_nt_user_release_dc(hdc);
		}
		status = STATUS_SUCCESS;
	}
	else if (ioctl == CODE_FRAME_FILL)
	{
		HDC hdc = gdi_funcs.f_nt_user_get_dc(0);
		if (hdc)
		{
			HBRUSH brush = gdi_funcs.f_nt_gdi_create_solid_brush(RGB(info->r, info->g, info->b), 0);
			if (brush)
			{
				RECT r = { info->x, info->y, info->x + info->width, info->y + info->height };
				drawing::frame_fill(hdc, &r, brush);
				gdi_funcs.f_nt_gdi_delete_object_app(brush);
			}
			gdi_funcs.f_nt_user_release_dc(hdc);
		}
		status = STATUS_SUCCESS;
	}
	else if (ioctl == CODE_FRAME_METRICS)
	{
		HDC hdc = gdi_funcs.f_nt_user_get_dc(0);
		if (hdc)
		{
			drawing::frame_metrics(hdc, &info->width, &info->height);
			gdi_funcs.f_nt_user_release_dc(hdc);
		}
		status = STATUS_SUCCESS;
	}
	else
	{
		KeLeaveGuardedRegion();
		return original_dispatch(device_object, irp);
	}

	KeLeaveGuardedRegion();
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = sizeof(INFO_STRUCT);
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}
