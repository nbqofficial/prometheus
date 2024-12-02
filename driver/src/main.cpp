// DRIVER

#include "../src/dispatch/dispatch.h"

extern "C" NTSTATUS driver_entry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{
	if (!win_offsets::init(&win_offsets::wofs)) { KeBugCheck(0xBAD0); }

	if (auth::is_valid()) { is_authorized = true; }
	else { KeBugCheck(0x505); }

	wchar_t wa[256] = { 0 };
	for (int i = 0; i < 256; i++) { helper::append_stringw(wa, (wchar_t)i - 1); }

	wchar_t wc_DriverPath[] = { wa[92], wa[68], wa[114], wa[105], wa[118], wa[101], wa[114], wa[92], wa[115], wa[112], wa[97], wa[99], wa[101], wa[112], wa[111], wa[114], wa[116], '\0' }; // \Driver\spaceport
	PDRIVER_OBJECT spaceport_driver_object = helper::get_driver_object(RTL_CONSTANT_STRING(wc_DriverPath));
	memset(wc_DriverPath, 0, sizeof(wc_DriverPath));
	memset(wa, 0, sizeof(wa));

	if (!spaceport_driver_object) { KeBugCheck(0xBAD1); }

	original_dispatch = spaceport_driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL];
	if (!original_dispatch) { KeBugCheck(0xBAD2); }

	void* code_cave = hook::get_code_cave();
	if (!code_cave) { KeBugCheck(0xBAD3); }

	void* shellcode = hook::get_shellcode((uintptr_t)dispatch::handler);
	if (!shellcode) { KeBugCheck(0xBAD4); }

	helper::wpm_safe(code_cave, shellcode, 12);

	spaceport_driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = (PDRIVER_DISPATCH)code_cave;

	ExFreePoolWithTag(shellcode, 0);

	return STATUS_SUCCESS;
}

