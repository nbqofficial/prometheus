// TEMPLATE

#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <io.h>
#include <fcntl.h>

typedef HANDLE(*_attach)();
typedef bool (*_detach)(HANDLE driver_handle);
typedef ULONG(*_get_status)(HANDLE driver_handle);
typedef ULONG(*_get_process_id_by_name)(HANDLE driver_handle, char* process_name, size_t process_name_size);
typedef uintptr_t(*_get_pml4)(HANDLE driver_handle, ULONG process_id);
typedef void* (*_get_module_base)(HANDLE driver_handle, ULONG process_id, wchar_t* module_name, size_t module_name_size, bool is64bit);
typedef void* (*_get_section_base)(HANDLE driver_handle, ULONG process_id);
typedef bool (*_read_memory)(HANDLE driver_handle, uintptr_t pml4, void* address, void* buffer, size_t size);
typedef bool (*_write_memory)(HANDLE driver_handle, uintptr_t pml4, void* address, void* buffer, size_t size);
typedef bool (*_mouse_init)(HANDLE driver_handle);
typedef bool (*_mouse_evt)(HANDLE driver_handle, long x, long y, unsigned short button_flags);
typedef bool (*_frame_init)(HANDLE driver_handle);
typedef bool (*_frame_rect)(HANDLE driver_handle, long x, long y, int width, int height, int thickness, unsigned char r, unsigned char g, unsigned char b);
typedef bool (*_frame_fill)(HANDLE driver_handle, long x, long y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
typedef bool (*_frame_metrics)(HANDLE driver_handle, int* width, int* height);

typedef struct _CORE_API
{
	_attach attach;
	_detach detach;
	_get_status get_status;
	_get_process_id_by_name get_process_id_by_name;
	_get_pml4 get_pml4;
	_get_module_base get_module_base;
	_get_section_base get_section_base;
	_read_memory read_memory;
	_write_memory write_memory;
	_mouse_init mouse_init;
	_mouse_evt mouse_evt;
	_frame_init frame_init;
	_frame_rect frame_rect;
	_frame_fill frame_fill;
	_frame_metrics frame_metrics;
}CORE_API, * PCORE_API;

extern PCORE_API coreapi;
extern HANDLE driver_handle;