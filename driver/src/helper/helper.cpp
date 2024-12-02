// DRIVER

#include "helper.h"

void helper::append_string(char* s, char c)
{
	size_t len = strlen(s);
	s[len] = c;
	s[len + 1] = '\0';
}

void helper::append_stringw(wchar_t* s, wchar_t c)
{
	size_t len = wcslen(s);
	s[len] = c;
	s[len + 1] = L'\0';
}

PDRIVER_OBJECT helper::get_driver_object(UNICODE_STRING driver_path)
{
    PDRIVER_OBJECT driver_object = 0;
    if (NT_SUCCESS(ObReferenceObjectByName(&driver_path, OBJ_CASE_INSENSITIVE, 0, 0, *IoDriverObjectType, KernelMode, 0, (PVOID*)&driver_object)))
    {
        return driver_object;
    }
    return 0;
}

void* helper::get_system_module(const char* module_name)
{
	void* module_base = 0;
	ULONG bytes = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, NULL, bytes, &bytes);

	if (!bytes) { return NULL; }

	PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePool(NonPagedPool, bytes);

	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);

	if (!NT_SUCCESS(status)) { return NULL; }

	PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;

	for (ULONG i = 0; i < modules->NumberOfModules; i++)
	{
		if (strcmp((char*)module[i].FullPathName, module_name) == 0)
		{
			module_base = module[i].ImageBase;
			break;
		}
	}

	if (modules) { ExFreePoolWithTag(modules, 0); }

	return module_base;
}

void* helper::get_system_module_short(const char* module_name)
{
	PVOID addr = 0;
	ULONG size = 0;
	NTSTATUS status = 0;
	PRTL_PROCESS_MODULES modules = 0;

	do {
		size = 0;
		status = ZwQuerySystemInformation(SystemModuleInformation, 0, 0, &size);

		modules = (PRTL_PROCESS_MODULES)ExAllocatePool(NonPagedPool, size);
		if (!modules) { return 0; }

		status = ZwQuerySystemInformation(SystemModuleInformation, modules, size, 0);
		if (!NT_SUCCESS(status)) { ExFreePoolWithTag(modules, 0); }

	} while (status == STATUS_INFO_LENGTH_MISMATCH);

	if (!NT_SUCCESS(status)) { return 0; }

	for (ULONG i = 0; i < modules->NumberOfModules; ++i)
	{
		RTL_PROCESS_MODULE_INFORMATION m = modules->Modules[i];

		size_t len_name = strlen(module_name);
		size_t len_path = strlen((PCHAR)m.FullPathName);

		if (len_path >= len_name && memcmp(&m.FullPathName[len_path - len_name], module_name, len_name) == 0)
		{
			addr = m.ImageBase;
			break;
		}
	}

	ExFreePoolWithTag(modules, 0);
	return addr;
}

void* helper::get_system_module_export(const char* module_name, const char* function_name)
{
	void* module = get_system_module_short(module_name);

	if (!module) { return 0; }

	return RtlFindExportedRoutineByName(module, function_name);
}

bool helper::wpm_safe(void* address, void* buffer, size_t size)
{
	PMDL mdl = IoAllocateMdl(address, size, 0, 0, 0);
	if (!mdl) { return false; }

	MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);
	void* map = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmNonCached, 0, 0, NormalPagePriority);
	MmProtectMdlSystemAddress(mdl, PAGE_READWRITE);

	memcpy(map, buffer, size);

	MmUnmapLockedPages(map, mdl);
	MmUnlockPages(mdl);
	IoFreeMdl(mdl);

	return true;
}

UINT64 helper::scan_pattern(PUINT8 base, SIZE_T size, PCUCHAR pattern, PCUCHAR mask, SIZE_T pattern_size)
{
	for (SIZE_T i(0); i < size - pattern_size; i++)
	{
		for (SIZE_T j(0); j < pattern_size; j++)
		{
			if ((mask == 0 || mask[j] != '?') && *(PUINT8)(base + i + j) != (UINT8)(pattern[j])) { break; }

			if (j == pattern_size - 1) { return (UINT64)(base)+i; }
		}
	}
	return 0;
}

PVOID helper::scan_by_section(PVOID base, PCCHAR section_name, PCUCHAR pattern, PCUCHAR mask, SIZE_T len)
{
	ANSI_STRING ansiSectionName, ansiCurrentSectionName;
	RtlInitAnsiString(&ansiSectionName, section_name);

	if (base == 0) { return 0; }

	PIMAGE_NT_HEADERS64 header = RtlImageNtHeader(base);
	if (!header) { return 0; }

	PIMAGE_SECTION_HEADER firstSection = (PIMAGE_SECTION_HEADER)((uintptr_t)&header->FileHeader + header->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER));

	for (PIMAGE_SECTION_HEADER section(firstSection); section < firstSection + header->FileHeader.NumberOfSections; section++)
	{
		RtlInitAnsiString(&ansiCurrentSectionName, (PCCHAR)section->Name);
		if (!RtlCompareString(&ansiSectionName, &ansiCurrentSectionName, TRUE))
		{
			UINT64 result = scan_pattern((PUCHAR)base + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask, len);
			if (result != 0) { return (PVOID)result; }
		}
	}
	return 0;
}

uintptr_t helper::get_ntoskrnl()
{
	char a[256] = { 0 };
	for (int i = 0; i < 256; i++) { helper::append_string(a, (char)i - 1); }
	char ntos_string[] = { a[110], a[116], a[111], a[115], a[107], a[114], a[110], a[108], a[46], a[101], a[120], a[101], '\0' }; // ntoskrnl.exe
	uintptr_t address = (uintptr_t)get_system_module_short(ntos_string);
	memset(ntos_string, 0, sizeof(ntos_string));
	memset(a, 0, sizeof(a));
	return address;
}

PVOID helper::resolve_relative_address(PVOID instruction, ULONG offset, ULONG instruction_size)
{
	ULONG_PTR instr = (ULONG_PTR)instruction;
	LONG rip_offset = *(PLONG)(instr + offset);
	return (PVOID)(instr + instruction_size + rip_offset);
}

BOOLEAN helper::data_cmp(const BYTE* data, const BYTE* mask, const char* sz_mask)
{
	for (; *sz_mask; ++sz_mask, ++data, ++mask)
	{
		if (*sz_mask == 'c' && *data != *mask) { return 0; }
	}
	return (*sz_mask) == 0;
}

uintptr_t helper::find_pattern(uintptr_t address, uintptr_t len, BYTE* mask, char* sz_mask)
{
	size_t max_len = len - strlen(sz_mask);
	for (uintptr_t i = 0; i < max_len; i++)
	{
		if (data_cmp((BYTE*)(address + i), mask, sz_mask)) { return (uintptr_t)(address + i); }
	}
	return 0;
}

PVOID helper::find_section(char* section_name, uintptr_t module_ptr, PULONG size)
{
	size_t namelength = strlen(section_name);
	PIMAGE_NT_HEADERS headers = (PIMAGE_NT_HEADERS)(module_ptr + ((PIMAGE_DOS_HEADER)module_ptr)->e_lfanew);
	PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(headers);
	for (DWORD i = 0; i < headers->FileHeader.NumberOfSections; ++i)
	{
		PIMAGE_SECTION_HEADER section = &sections[i];
		if (memcmp(section->Name, section_name, namelength) == 0 && namelength == strlen((char*)section->Name))
		{
			if (size) { *size = section->Misc.VirtualSize; }
			return (PVOID)(module_ptr + section->VirtualAddress);
		}
	}
	return 0;
}

uintptr_t helper::find_pattern_in_section(char* section_name, uintptr_t module_ptr, BYTE* mask, char* sz_mask)
{
	ULONG section_size = 0;
	PVOID section_base = find_section(section_name, module_ptr, &section_size);

	if (!section_base) { return 0; }
	return find_pattern((uintptr_t)section_base, section_size, mask, sz_mask);
}

uintptr_t helper::dereference(uintptr_t address, unsigned int offset)
{
	if (address == 0) { return 0; }

	return address + (int)((*(int*)(address + offset) + offset) + sizeof(int));
}
