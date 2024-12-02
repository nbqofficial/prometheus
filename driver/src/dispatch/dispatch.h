// DRIVER

#pragma once

#include "../auth/auth.h"

extern PDRIVER_DISPATCH original_dispatch;
extern MOUSE_OBJECT mouse_obj;
extern bool is_authorized;

namespace dispatch
{
	NTSTATUS handler(PDEVICE_OBJECT device_object, PIRP irp);
}