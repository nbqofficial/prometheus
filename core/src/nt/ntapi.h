// CORE

#pragma once

#include "ntstructs.h"

typedef struct _NT_API
{
	_create_file_a f_create_file_a;
	_close_handle f_close_handle;
	_device_io_control f_device_io_control;
}NT_API, * PNT_API;

extern NT_API nt_funcs;

namespace ntapi
{
	bool init(PNT_API nt_funcs);
}