// CORE

#include "ntapi.h"

NT_API nt_funcs = { 0 };

bool ntapi::init(PNT_API nt_fs)
{
    char a[256] = { 0 };
    for (int i = 0; i < 256; i++)
    {
        size_t len = strlen(a);
        a[len] = (char)i - 1;
        a[len + 1] = '\0';
    }

    char wc_kernel32dll[] = { a[107], a[101], a[114], a[110], a[101], a[108], a[51], a[50], a[46], a[100], a[108], a[108], '\0' }; // kernel32.dll
    HMODULE kernel_module = GetModuleHandleA(wc_kernel32dll);
    memset(wc_kernel32dll, 0, sizeof(wc_kernel32dll));

    if (!kernel_module) { return false; }

    if (!nt_fs->f_create_file_a || !nt_fs->f_close_handle || !nt_fs->f_device_io_control)
    {
        char wc_CreateFileA[] = { a[67], a[114], a[101], a[97], a[116], a[101], a[70], a[105], a[108], a[101], a[65], '\0' }; // CreateFileA
        nt_fs->f_create_file_a = (_create_file_a)GetProcAddress(kernel_module, wc_CreateFileA);
        memset(wc_CreateFileA, 0, sizeof(wc_CreateFileA));

        char wc_CloseHandle[] = { a[67], a[108], a[111], a[115], a[101], a[72], a[97],a[110], a[100], a[108], a[101], '\0' }; // CloseHandle
        nt_fs->f_close_handle = (_close_handle)GetProcAddress(kernel_module, wc_CloseHandle);
        memset(wc_CloseHandle, 0, sizeof(wc_CloseHandle));

        char wc_DeviceIoControl[] = { a[68], a[101], a[118], a[105], a[99], a[101], a[73], a[111], a[67], a[111], a[110], a[116], a[114], a[111], a[108], '\0' }; // DeviceIoControl
        nt_fs->f_device_io_control = (_device_io_control)GetProcAddress(kernel_module, wc_DeviceIoControl);
        memset(wc_DeviceIoControl, 0, sizeof(wc_DeviceIoControl));
    }
    return (nt_fs->f_create_file_a && nt_fs->f_close_handle && nt_fs->f_device_io_control);
}