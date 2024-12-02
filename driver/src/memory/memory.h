// DRIVER

#pragma once

#include "../hook/hook.h"

namespace memory
{
	bool is_process_running(PEPROCESS process);

	ULONG get_process_id_by_name(PEPROCESS start_process, const char* process_name);

	uintptr_t get_pml4(PEPROCESS process);

	void* get_module_base(PEPROCESS process, PUNICODE_STRING module_name, bool is64bit);

	void* get_section_base(PEPROCESS process);

	NTSTATUS physical_read_memory(uintptr_t pa, void* buffer, size_t size, size_t* bytes_read);

	NTSTATUS physical_write_memory(uintptr_t pa, void* buffer, size_t size);

	uintptr_t virt_to_phys(uintptr_t pml4, uintptr_t va, ULONG offset);

	NTSTATUS read_memory(uintptr_t pml4, uintptr_t va, void* buffer, size_t size);

	NTSTATUS write_memory(uintptr_t pml4, uintptr_t va, void* buffer, size_t size);
}
