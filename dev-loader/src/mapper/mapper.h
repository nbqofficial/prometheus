// DEV LOADER

#pragma once

#include "../core/core.h"

#define RELOC_FLAG64(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)

using _load_library_a = HMODULE(WINAPI*)(LPCSTR lib_name);
using _get_proc_address = FARPROC(WINAPI*)(HMODULE hmodule, LPCSTR proc_name);
using _dll_entrypoint = BOOL(WINAPI*)(void* hdll, DWORD reason_for_call, void* lp_reserved);

typedef struct _MAPPING_DATA
{
	_load_library_a f_load_library_a;
	_get_proc_address f_get_proc_address;
}MAPPING_DATA, * PMAPPING_DATA;

namespace mapper
{
	void append_string(char* s, char c);

	void __stdcall shellcode(PMAPPING_DATA data);

	bool run_dll(byte* payload, bool is_core, PCORE_API pcoreapi);

	void* manual_get_proc_address(void* base, PIMAGE_NT_HEADERS nt_header, LPCSTR proc_name);
}