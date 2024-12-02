// DRIVER

#include "drawing.h"

GDI_API gdi_funcs = { 0 };

bool drawing::init(PGDI_API gdi_api)
{
    if (!gdi_api->f_nt_user_get_dc || !gdi_api->f_nt_user_release_dc ||
        !gdi_api->f_nt_gdi_create_solid_brush || !gdi_api->f_nt_gdi_select_brush ||
        !gdi_api->f_nt_gdi_delete_object_app || !gdi_api->f_nt_gdi_pat_blt || !gdi_api->f_nt_gdi_get_device_caps)
    {
        char a[256] = { 0 };
        for (int i = 0; i < 256; i++) { helper::append_string(a, (char)i - 1); }

        char wc_win32kbase[] = { a[119], a[105], a[110], a[51], a[50], a[107], a[98], a[97], a[115], a[101], a[46], a[115], a[121], a[115], '\0' }; // win32kbase.sys
        char wc_win32kfull[] = { a[119], a[105], a[110], a[51], a[50], a[107], a[102], a[117], a[108], a[108], a[46], a[115], a[121], a[115], '\0' }; // win32kfull.sys

        char wc_NtUserGetDC[] = { a[78], a[116], a[85], a[115], a[101], a[114], a[71], a[101], a[116], a[68], a[67], '\0' }; // NtUserGetDC
        gdi_api->f_nt_user_get_dc = (_nt_user_get_dc)helper::get_system_module_export(wc_win32kbase, wc_NtUserGetDC);
        memset(wc_NtUserGetDC, 0, sizeof(wc_NtUserGetDC));

        char wc_NtUserReleaseDC[] = { a[78], a[116], a[85], a[115], a[101], a[114], a[82], a[101], a[108], a[101], a[97], a[115], a[101], a[68], a[67], '\0' }; // NtUserReleaseDC
        gdi_api->f_nt_user_release_dc = (_nt_user_release_dc)helper::get_system_module_export(wc_win32kbase, wc_NtUserReleaseDC);
        memset(wc_NtUserReleaseDC, 0, sizeof(wc_NtUserReleaseDC));

        char wc_NtGdiCreateSolidBrush[] = { a[78], a[116], a[71], a[100], a[105], a[67], a[114], a[101], a[97], a[116], a[101], a[83], a[111], a[108], a[105], a[100], a[66], a[114], a[117], a[115], a[104], '\0' }; // NtGdiCreateSolidBrush
        gdi_api->f_nt_gdi_create_solid_brush = (_nt_gdi_create_solid_brush)helper::get_system_module_export(wc_win32kfull, wc_NtGdiCreateSolidBrush);
        memset(wc_NtGdiCreateSolidBrush, 0, sizeof(wc_NtGdiCreateSolidBrush));

        char wc_NtGdiSelectBrush[] = { a[78], a[116], a[71], a[100], a[105], a[83], a[101], a[108], a[101], a[99], a[116], a[66], a[114], a[117], a[115], a[104], '\0' }; // NtGdiSelectBrush
        gdi_api->f_nt_gdi_select_brush = (_nt_gdi_select_brush)helper::get_system_module_export(wc_win32kfull, wc_NtGdiSelectBrush);
        memset(wc_NtGdiSelectBrush, 0, sizeof(wc_NtGdiSelectBrush));

        char wc_NtGdiDeleteObjectApp[] = { a[78], a[116], a[71], a[100], a[105], a[68], a[101], a[108], a[101], a[116], a[101], a[79], a[98], a[106], a[101], a[99], a[116], a[65], a[112], a[112], '\0' }; // NtGdiDeleteObjectApp
        gdi_api->f_nt_gdi_delete_object_app = (_nt_gdi_delete_object_app)helper::get_system_module_export(wc_win32kbase, wc_NtGdiDeleteObjectApp);
        memset(wc_NtGdiDeleteObjectApp, 0, sizeof(wc_NtGdiDeleteObjectApp));

        char wc_NtGdiPatBlt[] = { a[78], a[116], a[71], a[100], a[105], a[80], a[97], a[116], a[66], a[108], a[116], '\0' }; // NtGdiPatBlt
        gdi_api->f_nt_gdi_pat_blt = (_nt_gdi_pat_blt)helper::get_system_module_export(wc_win32kfull, wc_NtGdiPatBlt);
        memset(wc_NtGdiPatBlt, 0, sizeof(wc_NtGdiPatBlt));

        char wc_NtGdiGetDeviceCaps[] = { a[78], a[116], a[71], a[100], a[105], a[71], a[101], a[116], a[68], a[101], a[118], a[105], a[99], a[101], a[67], a[97], a[112], a[115], '\0' }; // NtGdiGetDeviceCaps
        gdi_api->f_nt_gdi_get_device_caps = (_nt_gdi_get_device_caps)helper::get_system_module_export(wc_win32kbase, wc_NtGdiGetDeviceCaps);
        memset(wc_NtGdiGetDeviceCaps, 0, sizeof(wc_NtGdiGetDeviceCaps));

        memset(wc_win32kbase, 0, sizeof(wc_win32kbase));
        memset(wc_win32kfull, 0, sizeof(wc_win32kfull));
        memset(a, 0, sizeof(a));
    }
    return (gdi_api->f_nt_user_get_dc && gdi_api->f_nt_user_release_dc &&
        gdi_api->f_nt_gdi_create_solid_brush && gdi_api->f_nt_gdi_select_brush &&
        gdi_api->f_nt_gdi_delete_object_app && gdi_api->f_nt_gdi_pat_blt && gdi_api->f_nt_gdi_get_device_caps);
}

bool drawing::frame_rect(HDC hdc, const RECT* rect, HBRUSH brush, int thickness)
{
    HBRUSH oldbrush;
    RECT r = *rect;

    if (!(oldbrush = gdi_funcs.f_nt_gdi_select_brush(hdc, brush))) { return false; }

    gdi_funcs.f_nt_gdi_pat_blt(hdc, r.left, r.top, thickness, r.bottom - r.top, PATCOPY);
    gdi_funcs.f_nt_gdi_pat_blt(hdc, r.right - thickness, r.top, thickness, r.bottom - r.top, PATCOPY);
    gdi_funcs.f_nt_gdi_pat_blt(hdc, r.left, r.top, r.right - r.left, thickness, PATCOPY);
    gdi_funcs.f_nt_gdi_pat_blt(hdc, r.left, r.bottom - thickness, r.right - r.left, thickness, PATCOPY);

    gdi_funcs.f_nt_gdi_select_brush(hdc, oldbrush);
    return true;
}

bool drawing::frame_fill(HDC hdc, const RECT* rect, HBRUSH brush)
{
    HBRUSH oldbrush;
    RECT r = *rect;

    if (!(oldbrush = gdi_funcs.f_nt_gdi_select_brush(hdc, brush))) { return false; }

    gdi_funcs.f_nt_gdi_pat_blt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, PATCOPY);

    gdi_funcs.f_nt_gdi_select_brush(hdc, oldbrush);
    return true;
}

bool drawing::frame_metrics(HDC hdc, int* width, int* height)
{
    *width = gdi_funcs.f_nt_gdi_get_device_caps(hdc, HORZRES);
    *height = gdi_funcs.f_nt_gdi_get_device_caps(hdc, VERTRES);
    return true;
}
