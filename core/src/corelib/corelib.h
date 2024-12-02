// CORE

#pragma once

#include "../defs/definitions.h"

namespace core
{
	// NON EXPORTED

	void append_string(char* s, char c);

	// EXPORTED

	extern "C" __declspec(dllexport) HANDLE attach();

	extern "C" __declspec(dllexport) bool detach(HANDLE driver_handle);

	extern "C" __declspec(dllexport) ULONG get_status(HANDLE driver_handle);

	extern "C" __declspec(dllexport) ULONG get_process_id_by_name(HANDLE driver_handle, char* process_name, size_t process_name_size);

	extern "C" __declspec(dllexport) uintptr_t get_pml4(HANDLE driver_handle, ULONG process_id);

	extern "C" __declspec(dllexport) void* get_module_base(HANDLE driver_handle, ULONG process_id, wchar_t* module_name, size_t module_name_size, bool is64bit);

	extern "C" __declspec(dllexport) void* get_section_base(HANDLE driver_handle, ULONG process_id);

	extern "C" __declspec(dllexport) bool read_memory(HANDLE driver_handle, uintptr_t pml4, void* address, void* buffer, size_t size);

	extern "C" __declspec(dllexport) bool write_memory(HANDLE driver_handle, uintptr_t pml4, void* address, void* buffer, size_t size);

	extern "C" __declspec(dllexport) bool mouse_init(HANDLE driver_handle);

	extern "C" __declspec(dllexport) bool mouse_evt(HANDLE driver_handle, long x, long y, unsigned short button_flags);

	extern "C" __declspec(dllexport) bool frame_init(HANDLE driver_handle);

	extern "C" __declspec(dllexport) bool frame_rect(HANDLE driver_handle, long x, long y, int width, int height, int thickness, unsigned char r, unsigned char g, unsigned char b);

	extern "C" __declspec(dllexport) bool frame_fill(HANDLE driver_handle, long x, long y, int width, int height, unsigned char r, unsigned char g, unsigned char b);

	extern "C" __declspec(dllexport) bool frame_metrics(HANDLE driver_handle, int* width, int* height);
}