// DRIVER

#include "memory.h"

bool memory::is_process_running(PEPROCESS process)
{
	if (!process) { return false; }

	PKPROCESS kproc = (PKPROCESS)process;
	PDISPATCHER_HEADER header = (PDISPATCHER_HEADER)kproc;

	if (!header->SignalState) { return true; }

	return false;
}

ULONG memory::get_process_id_by_name(PEPROCESS start_process, const char* process_name)
{
	PLIST_ENTRY active_process_links;
	PEPROCESS current_process = start_process;

	do
	{
		PKPROCESS kproc = (PKPROCESS)current_process;
		PDISPATCHER_HEADER header = (PDISPATCHER_HEADER)kproc;
		LPSTR current_process_name = (LPSTR)((PUCHAR)current_process + win_offsets::wofs.IMAGE_FILE_NAME);

		if (header->SignalState == 0 && strcmp(current_process_name, process_name) == 0)
		{
			return (ULONG)PsGetProcessId(current_process);
		}

		active_process_links = (PLIST_ENTRY)((PUCHAR)current_process + win_offsets::wofs.ACTIVE_PROCESS_LINKS);
		current_process = (PEPROCESS)(active_process_links->Flink);
		current_process = (PEPROCESS)((PUCHAR)current_process - win_offsets::wofs.ACTIVE_PROCESS_LINKS);

	} while (start_process != current_process);

	return 0;
}

uintptr_t memory::get_pml4(PEPROCESS process)
{
	if (!process) { return 0; }

	uintptr_t pml4 = 0;

	uintptr_t addr = (uintptr_t)((PUCHAR)process + 0x28);

	memcpy(&pml4, (void*)addr, sizeof(uintptr_t));

	return pml4;
}

void* memory::get_module_base(PEPROCESS process, PUNICODE_STRING module_name, bool is64bit)
{
	if (!process) { return 0; }

	uintptr_t pml4 = get_pml4(process);
	if (!pml4) { return 0; }

	if (is64bit)
	{
		uintptr_t peb_address = (uintptr_t)PsGetProcessPeb(process);
		if (!peb_address) { return 0; }

		PEB peb = { 0 };
		if (!NT_SUCCESS(read_memory(pml4, peb_address, &peb, sizeof(PEB)))) { return 0; }

		PEB_LDR_DATA peb_ldr_data = { 0 };
		if (!NT_SUCCESS(read_memory(pml4, (uintptr_t)peb.Ldr, &peb_ldr_data, sizeof(PEB_LDR_DATA)))) { return 0; }

		PLIST_ENTRY ldr_list_head = (PLIST_ENTRY)peb_ldr_data.InLoadOrderModuleList.Flink;
		PLIST_ENTRY ldr_current_node = (PLIST_ENTRY)peb_ldr_data.InLoadOrderModuleList.Flink;

		do
		{
			LDR_DATA_TABLE_ENTRY lst_entry = { 0 };
			if (!NT_SUCCESS(read_memory(pml4, (uintptr_t)ldr_current_node, &lst_entry, sizeof(LDR_DATA_TABLE_ENTRY)))) { return 0; }

			ldr_current_node = lst_entry.InLoadOrderLinks.Flink;
			if (lst_entry.BaseDllName.Length > 0)
			{
				WCHAR sz_base_dll_name[MAX_PATH] = { 0 };
				if (!NT_SUCCESS(read_memory(pml4, (uintptr_t)lst_entry.BaseDllName.Buffer, &sz_base_dll_name, lst_entry.BaseDllName.Length))) { return 0; }

				UNICODE_STRING ustr;
				RtlUnicodeStringInit(&ustr, sz_base_dll_name);

				if (RtlCompareUnicodeString(&ustr, module_name, TRUE) == 0) { return (PVOID)lst_entry.DllBase; }
			}

		} while (ldr_list_head != ldr_current_node);
	}
	else // 32 bit
	{
		uintptr_t peb32_address = (uintptr_t)PsGetProcessWow64Process(process);
		if (!peb32_address) { return 0; }

		PEB32 peb32 = { 0 };
		if (!NT_SUCCESS(read_memory(pml4, peb32_address, &peb32, sizeof(PEB32)))) { return 0; }

		PEB_LDR_DATA32 peb_ldr_data32 = { 0 };
		if (!NT_SUCCESS(read_memory(pml4, (uintptr_t)peb32.Ldr, &peb_ldr_data32, sizeof(PEB_LDR_DATA32)))) { return 0; }

		PLIST_ENTRY32 ldr_list_head32 = (PLIST_ENTRY32)peb_ldr_data32.InLoadOrderModuleList.Flink;
		PLIST_ENTRY32 ldr_current_node32 = (PLIST_ENTRY32)peb_ldr_data32.InLoadOrderModuleList.Flink;

		do
		{
			LDR_DATA_TABLE_ENTRY32 lst_entry32 = { 0 };
			if (!NT_SUCCESS(read_memory(pml4, (uintptr_t)ldr_current_node32, &lst_entry32, sizeof(LDR_DATA_TABLE_ENTRY32)))) { return 0; }

			ldr_current_node32 = (PLIST_ENTRY32)lst_entry32.InLoadOrderLinks.Flink;
			if (lst_entry32.BaseDllName.Length > 0)
			{
				WCHAR sz_base_dll_name[MAX_PATH] = { 0 };
				if (!NT_SUCCESS(read_memory(pml4, (uintptr_t)lst_entry32.BaseDllName.Buffer, &sz_base_dll_name, lst_entry32.BaseDllName.Length))) { return 0; }

				UNICODE_STRING ustr;
				RtlUnicodeStringInit(&ustr, sz_base_dll_name);

				if (RtlCompareUnicodeString(&ustr, module_name, TRUE) == 0) { return (PVOID)lst_entry32.DllBase; }
			}

		} while (ldr_list_head32 != ldr_current_node32);
	}
	return 0;
}

void* memory::get_section_base(PEPROCESS process)
{
	if (!process) { return 0; }
	return PsGetProcessSectionBaseAddress(process);
}

NTSTATUS memory::physical_read_memory(uintptr_t pa, void* buffer, size_t size, size_t* bytes_read)
{
	if (!pa || !buffer || !size) { return STATUS_INVALID_PARAMETER; }

	MM_COPY_ADDRESS address = { 0 };
	address.PhysicalAddress.QuadPart = pa;

	return MmCopyMemory(buffer, address, size, MM_COPY_MEMORY_PHYSICAL, bytes_read);
}

NTSTATUS memory::physical_write_memory(uintptr_t pa, void* buffer, size_t size)
{
	if (!pa || !buffer || !size) { return STATUS_INVALID_PARAMETER; }

	PHYSICAL_ADDRESS address = { 0 };
	address.QuadPart = pa;

	void* mapped_address = MmMapIoSpace(address, size, MmNonCached);
	memcpy(mapped_address, buffer, size);
	MmUnmapIoSpace(mapped_address, size);

	return STATUS_SUCCESS;
}

uintptr_t memory::virt_to_phys(uintptr_t pml4, uintptr_t va, ULONG offset)
{
	if (!pml4 || !va) { return 0; }

	size_t bytes_read = 0;

	const uintptr_t PMASK = (~0xfull << 8) & 0xfffffffffull;
	uintptr_t dirbase = pml4 &= ~0xf;
	uintptr_t address = va + offset;

	uintptr_t page_offset = address & ~(~0ul << 12);
	uintptr_t pte = ((address >> 12) & (0x1ffll));
	uintptr_t pt = ((address >> 21) & (0x1ffll));
	uintptr_t pd = ((address >> 30) & (0x1ffll));
	uintptr_t pdp = ((address >> 39) & (0x1ffll));

	uintptr_t pdpe;
	physical_read_memory((dirbase + 8 * pdp), &pdpe, sizeof(uintptr_t), &bytes_read);
	if (~pdpe & 1) { return 0; }

	uintptr_t pde;
	physical_read_memory(((pdpe & PMASK) + 8 * pd), &pde, sizeof(uintptr_t), &bytes_read);
	if (~pde & 1) { return 0; }

	if (pde & 0x80) { return (pde & (~0ull << 42 >> 12)) + (address & ~(~0ull << 30)); }

	uintptr_t pte_addr;
	physical_read_memory(((pde & PMASK) + 8 * pt), &pte_addr, sizeof(uintptr_t), &bytes_read);
	if (~pte_addr & 1) { return 0; }

	if (pte_addr & 0x80) { return (pte_addr & PMASK) + (address & ~(~0ull << 21)); }

	physical_read_memory(((pte_addr & PMASK) + 8 * pte), &address, sizeof(uintptr_t), &bytes_read);
	address &= PMASK;
	if (!address) { return 0; }

	return address + page_offset;
}

NTSTATUS memory::read_memory(uintptr_t pml4, uintptr_t va, void* buffer, size_t size)
{
	if (!pml4 || !va || !buffer || !size) { return STATUS_INVALID_PARAMETER; }

	NTSTATUS status = STATUS_SUCCESS;

	uintptr_t current_offset = 0;
	uintptr_t total_size = size;

	while (total_size)
	{
		uintptr_t pa = virt_to_phys(pml4, va, current_offset);
		if (!pa) { return STATUS_INVALID_ADDRESS; }

		uintptr_t read_size = min(PAGE_SIZE - (pa & 0xfff), total_size);

		size_t bytes_read = 0;
		status = physical_read_memory(pa, ((PUCHAR)buffer + current_offset), read_size, &bytes_read);

		total_size -= bytes_read;
		current_offset += bytes_read;

		if (!NT_SUCCESS(status)) { break; }
		if (!bytes_read) { break; }
	}

	return status;
}

NTSTATUS memory::write_memory(uintptr_t pml4, uintptr_t va, void* buffer, size_t size)
{
	if (!pml4 || !va || !buffer || !size) { return STATUS_INVALID_PARAMETER; }

	uintptr_t pa = virt_to_phys(pml4, va, 0);
	if (!pa) { return STATUS_INVALID_ADDRESS; }

	return physical_write_memory(pa, buffer, size);
}


