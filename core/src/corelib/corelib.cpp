// CORE

#include "corelib.h"

// NON EXPORTED

void core::append_string(char* s, char c)
{
    size_t len = strlen(s);
    s[len] = c;
    s[len + 1] = '\0';
}

// EXPORTED

HANDLE core::attach()
{
    char a[256] = { 0 };
    for (int i = 0; i < 256; i++) { append_string(a, (char)i - 1); }

    char path[] = { a[92], a[92], a[46], a[92], a[83], a[112], a[97], a[99], a[101], a[112], a[111], a[114], a[116], '\0' };
    HANDLE driver_handle = nt_funcs.f_create_file_a(path, GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    memset(path, 0, sizeof(path));

    return driver_handle;
}

bool core::detach(HANDLE driver_handle)
{
    return nt_funcs.f_close_handle(driver_handle);
}

ULONG core::get_status(HANDLE driver_handle)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };

        if (nt_funcs.f_device_io_control(driver_handle, CODE_STATUS, &info, sizeof(info), &info, sizeof(info), &bytes, 0))
        {
            return info.status;
        }
        return 0;
    }
    return 0;
}

ULONG core::get_process_id_by_name(HANDLE driver_handle, char* process_name, size_t process_name_size)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };
        memcpy(info.process_name, process_name, process_name_size);

        if (nt_funcs.f_device_io_control(driver_handle, CODE_PROCESS_ID, &info, sizeof(info), &info, sizeof(info), &bytes, 0))
        {
            return info.process_id;
        }
        return 0;
    }
    return 0;
}

uintptr_t core::get_pml4(HANDLE driver_handle, ULONG process_id)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };
        info.process_id = process_id;

        if (nt_funcs.f_device_io_control(driver_handle, CODE_PML4, &info, sizeof(info), &info, sizeof(info), &bytes, 0))
        {
            return info.pml4;
        }
        return 0;
    }
    return 0;
}

void* core::get_module_base(HANDLE driver_handle, ULONG process_id, wchar_t* module_name, size_t module_name_size, bool is64bit)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };
        info.process_id = process_id;
        memcpy(info.module_name, module_name, module_name_size);
        info.is64bit = is64bit;

        if (nt_funcs.f_device_io_control(driver_handle, CODE_MODULE_BASE, &info, sizeof(info), &info, sizeof(info), &bytes, 0))
        {
            return info.module_base;
        }
        return 0;
    }
    return 0;
}

void* core::get_section_base(HANDLE driver_handle, ULONG process_id)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };
        info.process_id = process_id;

        if (nt_funcs.f_device_io_control(driver_handle, CODE_SECTION_BASE, &info, sizeof(info), &info, sizeof(info), &bytes, 0))
        {
            return info.module_base;
        }
        return 0;
    }
    return 0;
}

bool core::read_memory(HANDLE driver_handle, uintptr_t pml4, void* address, void* buffer, size_t size)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };
        info.pml4 = pml4;
        info.address = address;
        info.buffer = buffer;
        info.size = size;

        return nt_funcs.f_device_io_control(driver_handle, CODE_READ_MEMORY, &info, sizeof(info), &info, sizeof(info), &bytes, 0);
    }
    return false;
}

bool core::write_memory(HANDLE driver_handle, uintptr_t pml4, void* address, void* buffer, size_t size)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };
        info.pml4 = pml4;
        info.address = address;
        info.buffer = buffer;
        info.size = size;

        return nt_funcs.f_device_io_control(driver_handle, CODE_WRITE_MEMORY, &info, sizeof(info), &info, sizeof(info), &bytes, 0);
    }
    return false;
}

bool core::mouse_init(HANDLE driver_handle)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };

        if (nt_funcs.f_device_io_control(driver_handle, CODE_MOUSE_INIT, &info, sizeof(info), &info, sizeof(info), &bytes, 0))
        {
            return (info.status == 12345);
        }
        return false;
    }
    return false;
}

bool core::mouse_evt(HANDLE driver_handle, long x, long y, unsigned short button_flags)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };
        info.x = x;
        info.y = y;
        info.button_flags = button_flags;

        return nt_funcs.f_device_io_control(driver_handle, CODE_MOUSE_EVENT, &info, sizeof(info), &info, sizeof(info), &bytes, 0);
    }
    return false;
}

bool core::frame_init(HANDLE driver_handle)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };

        if (nt_funcs.f_device_io_control(driver_handle, CODE_FRAME_INIT, &info, sizeof(info), &info, sizeof(info), &bytes, 0))
        {
            return (info.status == 12345);
        }
        return false;
    }
    return false;
}

bool core::frame_rect(HANDLE driver_handle, long x, long y, int width, int height, int thickness, unsigned char r, unsigned char g, unsigned char b)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };
        info.x = x;
        info.y = y;
        info.width = width;
        info.height = height;
        info.thickness = thickness;
        info.r = r;
        info.g = g;
        info.b = b;

        return nt_funcs.f_device_io_control(driver_handle, CODE_FRAME_RECT, &info, sizeof(info), &info, sizeof(info), &bytes, 0);
    }
    return false;
}

bool core::frame_fill(HANDLE driver_handle, long x, long y, int width, int height, unsigned char r, unsigned char g, unsigned char b)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };
        info.x = x;
        info.y = y;
        info.width = width;
        info.height = height;
        info.r = r;
        info.g = g;
        info.b = b;

        return nt_funcs.f_device_io_control(driver_handle, CODE_FRAME_FILL, &info, sizeof(info), &info, sizeof(info), &bytes, 0);
    }
    return false;
}

bool core::frame_metrics(HANDLE driver_handle, int* width, int* height)
{
    if (driver_handle)
    {
        ULONG bytes = 0;
        INFO_STRUCT info = { 0 };

        if (nt_funcs.f_device_io_control(driver_handle, CODE_FRAME_METRICS, &info, sizeof(info), &info, sizeof(info), &bytes, 0))
        {
            *width = info.width;
            *height = info.height;
            return true;
        }
        return false;
    }
    return false;
}