// DRIVER

#pragma once

#include "../defs/definitions.h"

extern "C" POBJECT_TYPE * IoDriverObjectType;

namespace helper
{
	void append_string(char* s, char c);

	void append_stringw(wchar_t* s, wchar_t c);

	PDRIVER_OBJECT get_driver_object(UNICODE_STRING driver_path);

	void* get_system_module(const char* module_name);

	void* get_system_module_short(const char* module_name);

	void* get_system_module_export(const char* module_name, const char* function_name);

	bool wpm_safe(void* address, void* buffer, size_t size);

	UINT64 scan_pattern(PUINT8 base, SIZE_T size, PCUCHAR pattern, PCUCHAR mask, SIZE_T pattern_size);

	PVOID scan_by_section(PVOID base, PCCHAR section_name, PCUCHAR pattern, PCUCHAR mask, SIZE_T len);

	uintptr_t get_ntoskrnl();

	PVOID resolve_relative_address(PVOID instruction, ULONG offset, ULONG instruction_size);

	BOOLEAN data_cmp(const BYTE* data, const BYTE* mask, const char* sz_mask);

	uintptr_t find_pattern(uintptr_t address, uintptr_t len, BYTE* mask, char* sz_mask);

	PVOID find_section(char* section_name, uintptr_t module_ptr, PULONG size);

	uintptr_t find_pattern_in_section(char* section_name, uintptr_t module_ptr, BYTE* mask, char* sz_mask);

	uintptr_t dereference(uintptr_t address, unsigned int offset);
}