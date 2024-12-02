// DRIVER

#include "hook.h"

void* hook::get_shellcode(uintptr_t hook_function)
{
    BYTE shellcode[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        // movabs rax, [address]
                         0x50,                                                              // push rax
                         0xC3 };                                                            // ret

    void* code_buffer = ExAllocatePool(NonPagedPool, sizeof(shellcode));

    if (!code_buffer) { return 0; }

    memcpy(code_buffer, shellcode, sizeof(shellcode));

    *(uintptr_t*)((uintptr_t)code_buffer + 0x2) = hook_function;

    return code_buffer;
}

void* hook::get_code_cave()
{
    char a[256] = { 0 };
    for (int i = 0; i < 256; i++) { helper::append_string(a, (char)i - 1); }
    char wc_DriverPath[] = { a[92], a[83], a[121], a[115], a[116], a[101], a[109], a[82], a[111], a[111], a[116], a[92], a[83], a[121], a[115], a[116], a[101], a[109], a[51], a[50], a[92], a[100], a[114], a[105], a[118], a[101], a[114], a[115], a[92], a[115], a[112], a[97], a[99], a[101], a[112], a[111], a[114], a[116], a[46], a[115], a[121], a[115], '\0' }; // \SystemRoot\System32\drivers\spaceport.sys
    void* spaceport_base = helper::get_system_module(wc_DriverPath);
    memset(wc_DriverPath, 0, sizeof(wc_DriverPath));
    memset(a, 0, sizeof(a));

    if (!spaceport_base) { return 0; }

    UCHAR code_cave_pattern[] = { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };
    void* code_cave = helper::scan_by_section(spaceport_base, ".text", code_cave_pattern, 0, sizeof(code_cave_pattern));

    return code_cave;
}