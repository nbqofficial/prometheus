// DRIVER

#pragma once

#include "../memory/memory.h"

typedef HDC(*_nt_user_get_dc)(HWND hwnd);
typedef int(*_nt_user_release_dc)(HDC hdc);
typedef HBRUSH(*_nt_gdi_create_solid_brush)(COLORREF cr, HBRUSH brush);
typedef HBRUSH(*_nt_gdi_select_brush)(HDC hdc, HBRUSH brush);
typedef BOOL(*_nt_gdi_delete_object_app)(HANDLE hobj);
typedef BOOL(*_nt_gdi_pat_blt)(HDC hdc, INT x, INT y, INT cx, INT cy, DWORD rop);
typedef INT(*_nt_gdi_get_device_caps)(HDC hdc, INT i);

typedef struct _GDI_API
{
	_nt_user_get_dc f_nt_user_get_dc;
	_nt_user_release_dc f_nt_user_release_dc;
	_nt_gdi_create_solid_brush f_nt_gdi_create_solid_brush;
	_nt_gdi_select_brush f_nt_gdi_select_brush;
	_nt_gdi_delete_object_app f_nt_gdi_delete_object_app;
	_nt_gdi_pat_blt f_nt_gdi_pat_blt;
	_nt_gdi_get_device_caps f_nt_gdi_get_device_caps;
}GDI_API, * PGDI_API;

extern GDI_API gdi_funcs;

namespace drawing
{
	bool init(PGDI_API gdi_api);

	bool frame_rect(HDC hdc, const RECT* rect, HBRUSH brush, int thickness);

	bool frame_fill(HDC hdc, const RECT* rect, HBRUSH brush);

	bool frame_metrics(HDC hdc, int* width, int* height);
}