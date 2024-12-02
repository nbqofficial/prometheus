// DEV LOADER

#include "mapper.h"

void mapper::append_string(char* s, char c)
{
	size_t len = strlen(s);
	s[len] = c;
	s[len + 1] = '\0';
}

void __stdcall mapper::shellcode(PMAPPING_DATA data)
{
	if (!data) { return; }

	byte* base = reinterpret_cast<byte*>(data);
	auto* opt = &reinterpret_cast<IMAGE_NT_HEADERS*>(base + reinterpret_cast<IMAGE_DOS_HEADER*>(data)->e_lfanew)->OptionalHeader;

	auto __load_library_a = data->f_load_library_a;
	auto __get_proc_address = data->f_get_proc_address;
	auto __dll_main = reinterpret_cast<_dll_entrypoint>(base + opt->AddressOfEntryPoint);

	byte* location_delta = base - opt->ImageBase;

	if (location_delta)
	{
		if (!opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) { return; }

		auto* reloc_data = reinterpret_cast<IMAGE_BASE_RELOCATION*>(base + opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

		while (reloc_data->VirtualAddress)
		{
			UINT amount_of_entries = (reloc_data->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			WORD* relative_info = reinterpret_cast<WORD*>(reloc_data + 1);

			for (UINT i = 0; i != amount_of_entries; ++i, ++relative_info)
			{
				if (RELOC_FLAG64(*relative_info))
				{
					UINT_PTR* patch = reinterpret_cast<UINT_PTR*>(base + reloc_data->VirtualAddress + ((*relative_info) & 0xFFF));
					*patch += reinterpret_cast<UINT_PTR>(location_delta);
				}
			}
			reloc_data = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<BYTE*>(reloc_data) + reloc_data->SizeOfBlock);
		}
	}

	if (opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
	{
		auto* import_descr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		while (import_descr->Name)
		{
			char* sz_mod = reinterpret_cast<char*>(base + import_descr->Name);
			HINSTANCE hdll = __load_library_a(sz_mod);

			ULONG_PTR* thunk_ref = reinterpret_cast<ULONG_PTR*>(base + import_descr->OriginalFirstThunk);
			ULONG_PTR* func_ref = reinterpret_cast<ULONG_PTR*>(base + import_descr->FirstThunk);

			if (!thunk_ref) { thunk_ref = func_ref; }

			for (; *thunk_ref; ++thunk_ref, ++func_ref)
			{
				if (IMAGE_SNAP_BY_ORDINAL(*thunk_ref))
				{
					*func_ref = (ULONG_PTR)__get_proc_address(hdll, reinterpret_cast<char*>(*thunk_ref & 0xFFFF));
				}
				else
				{
					auto* pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base + (*thunk_ref));
					*func_ref = (ULONG_PTR)__get_proc_address(hdll, pImport->Name);
				}
			}
			++import_descr;
		}
	}

	if (opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
	{
		auto* tls = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(base + opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
		auto* callback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(tls->AddressOfCallBacks);
		for (; callback && *callback; ++callback)
		{
			(*callback)(base, DLL_PROCESS_ATTACH, nullptr);
		}
	}

	__dll_main(base, DLL_PROCESS_ATTACH, &coreapi);
}

bool mapper::run_dll(byte* payload, bool is_core, PCORE_API pcoreapi)
{
	if (!payload) { return false; }

	PIMAGE_NT_HEADERS nt_headers = reinterpret_cast<IMAGE_NT_HEADERS*>(payload + reinterpret_cast<IMAGE_DOS_HEADER*>(payload)->e_lfanew);
	PIMAGE_OPTIONAL_HEADER opt_header = &nt_headers->OptionalHeader;
	PIMAGE_FILE_HEADER file_header = &nt_headers->FileHeader;

	byte* target_base = reinterpret_cast<byte*>(VirtualAlloc(0, opt_header->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

	if (!target_base) { return false; }

	MAPPING_DATA data = { 0 };
	data.f_load_library_a = LoadLibraryA;
	data.f_get_proc_address = GetProcAddress;

	auto* section_header = IMAGE_FIRST_SECTION(nt_headers);

	for (UINT i = 0; i != file_header->NumberOfSections; ++i, ++section_header)
	{
		if (section_header->SizeOfRawData)
		{
			memcpy((void*)(target_base + section_header->VirtualAddress), (void*)(payload + section_header->PointerToRawData), section_header->SizeOfRawData);
		}
	}

	memcpy(payload, &data, sizeof(data));

	memcpy(target_base, payload, 0x1000);

	if (is_core)	// resolve core.dll exports to pcoreapi
	{
		if (!pcoreapi->attach || !pcoreapi->detach || !pcoreapi->get_status ||
			!pcoreapi->get_process_id_by_name || !pcoreapi->get_pml4 || !pcoreapi->get_module_base ||
			!pcoreapi->get_section_base || !pcoreapi->read_memory || !pcoreapi->write_memory || 
			!pcoreapi->mouse_init || !pcoreapi->mouse_evt ||
			!pcoreapi->frame_init || !pcoreapi->frame_rect || !pcoreapi->frame_fill || !pcoreapi->frame_metrics)
		{
			char a[256] = { 0 };
			for (int i = 0; i < 256; i++) { append_string(a, (char)i - 1); }

			char wc_attach[] = { a[97], a[116], a[116], a[97], a[99], a[104], '\0' }; // attach
			pcoreapi->attach = (_attach)manual_get_proc_address(target_base, nt_headers, wc_attach);
			memset(wc_attach, 0, sizeof(wc_attach));

			char wc_detach[] = { a[100], a[101], a[116], a[97], a[99], a[104], '\0' }; // detach
			pcoreapi->detach = (_detach)manual_get_proc_address(target_base, nt_headers, wc_detach);
			memset(wc_detach, 0, sizeof(wc_detach));

			char wc_get_status[] = { a[103], a[101], a[116], a[95], a[115], a[116], a[97], a[116], a[117], a[115], '\0' }; // get_status
			pcoreapi->get_status = (_get_status)manual_get_proc_address(target_base, nt_headers, wc_get_status);
			memset(wc_get_status, 0, sizeof(wc_get_status));

			char wc_get_process_id_by_name[] = { a[103], a[101], a[116], a[95], a[112], a[114], a[111], a[99], a[101], a[115], a[115], a[95], a[105], a[100], a[95], a[98], a[121], a[95], a[110], a[97], a[109], a[101], '\0' }; // get_process_id_by_name
			pcoreapi->get_process_id_by_name = (_get_process_id_by_name)manual_get_proc_address(target_base, nt_headers, wc_get_process_id_by_name);
			memset(wc_get_process_id_by_name, 0, sizeof(wc_get_process_id_by_name));

			char wc_get_pml4[] = { a[103], a[101], a[116], a[95], a[112], a[109], a[108], a[52], '\0' }; // get_pml4
			pcoreapi->get_pml4 = (_get_pml4)manual_get_proc_address(target_base, nt_headers, wc_get_pml4);
			memset(wc_get_pml4, 0, sizeof(wc_get_pml4));

			char wc_get_module_base[] = { a[103], a[101], a[116], a[95], a[109], a[111], a[100], a[117], a[108], a[101], a[95], a[98], a[97], a[115], a[101], '\0' }; // get_module_base
			pcoreapi->get_module_base = (_get_module_base)manual_get_proc_address(target_base, nt_headers, wc_get_module_base);
			memset(wc_get_module_base, 0, sizeof(wc_get_module_base));

			char wc_get_section_base[] = { a[103], a[101], a[116], a[95], a[115], a[101], a[99], a[116], a[105], a[111], a[110], a[95], a[98], a[97], a[115], a[101], '\0' }; // get_section_base
			pcoreapi->get_section_base = (_get_section_base)manual_get_proc_address(target_base, nt_headers, wc_get_section_base);
			memset(wc_get_section_base, 0, sizeof(wc_get_section_base));

			char wc_read_memory[] = { a[114],  a[101],  a[97],  a[100],  a[95],  a[109],  a[101],  a[109],  a[111],  a[114],  a[121], '\0' }; // read_memory
			pcoreapi->read_memory = (_read_memory)manual_get_proc_address(target_base, nt_headers, wc_read_memory);
			memset(wc_read_memory, 0, sizeof(wc_read_memory));

			char wc_write_memory[] = { a[119], a[114], a[105], a[116], a[101], a[95], a[109], a[101], a[109], a[111], a[114], a[121], '\0' }; // write_memory
			pcoreapi->write_memory = (_write_memory)manual_get_proc_address(target_base, nt_headers, wc_write_memory);
			memset(wc_write_memory, 0, sizeof(wc_write_memory));

			char wc_mouse_init[] = { a[109], a[111], a[117], a[115], a[101], a[95], a[105], a[110], a[105], a[116],  '\0' }; // mouse_init
			pcoreapi->mouse_init = (_mouse_init)manual_get_proc_address(target_base, nt_headers, wc_mouse_init);
			memset(wc_mouse_init, 0, sizeof(wc_mouse_init));

			char wc_mouse_evt[] = { a[109], a[111], a[117], a[115], a[101], a[95], a[101], a[118], a[116], '\0' }; // mouse_evt
			pcoreapi->mouse_evt = (_mouse_evt)manual_get_proc_address(target_base, nt_headers, wc_mouse_evt);
			memset(wc_mouse_evt, 0, sizeof(wc_mouse_evt));

			char wc_frame_init[] = { a[102], a[114], a[97], a[109], a[101], a[95], a[105], a[110], a[105], a[116], '\0' }; // frame_init
			pcoreapi->frame_init = (_frame_init)manual_get_proc_address(target_base, nt_headers, wc_frame_init);
			memset(wc_frame_init, 0, sizeof(wc_frame_init));

			char wc_frame_rect[] = { a[102], a[114], a[97], a[109], a[101], a[95], a[114], a[101], a[99], a[116], '\0' }; // frame_rect
			pcoreapi->frame_rect = (_frame_rect)manual_get_proc_address(target_base, nt_headers, wc_frame_rect);
			memset(wc_frame_rect, 0, sizeof(wc_frame_rect));

			char wc_frame_fill[] = { a[102], a[114], a[97], a[109], a[101], a[95], a[102], a[105], a[108], a[108], '\0' }; // frame_fill
			pcoreapi->frame_fill = (_frame_fill)manual_get_proc_address(target_base, nt_headers, wc_frame_fill);
			memset(wc_frame_fill, 0, sizeof(wc_frame_fill));

			char wc_frame_metrics[] = { a[102],  a[114],  a[97],  a[109],  a[101],  a[95],  a[109],  a[101],  a[116],  a[114],  a[105],  a[99],  a[115], '\0' }; // frame_metrics
			pcoreapi->frame_metrics = (_frame_metrics)manual_get_proc_address(target_base, nt_headers, wc_frame_metrics);
			memset(wc_frame_metrics, 0, sizeof(wc_frame_metrics));

			memset(a, 0, sizeof(a));
		}
	}

	shellcode((PMAPPING_DATA)target_base);

	return true;
}

void* mapper::manual_get_proc_address(void* base, PIMAGE_NT_HEADERS nt_header, LPCSTR proc_name)
{
	const auto export_directory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>((uintptr_t)base + nt_header->OptionalHeader.DataDirectory[0].VirtualAddress);

	const auto export_functions = reinterpret_cast<std::uint32_t*>((uintptr_t)base + export_directory->AddressOfFunctions);
	const auto export_names = reinterpret_cast<std::uint32_t*>((uintptr_t)base + export_directory->AddressOfNames);
	const auto export_ordinals = reinterpret_cast<std::uint16_t*>((uintptr_t)base + export_directory->AddressOfNameOrdinals);

	for (auto i = 0u; i < export_directory->NumberOfNames; i++)
	{
		const auto export_name = reinterpret_cast<const char*>((uintptr_t)base + export_names[i]);

		if (std::strcmp(export_name, proc_name)) { continue; }

		return reinterpret_cast<void*>((uintptr_t)base + export_functions[export_ordinals[i]]);
	}
}
