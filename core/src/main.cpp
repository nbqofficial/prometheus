// CORE

#include "../src/corelib/corelib.h"

BOOL WINAPI DllMain(HMODULE hmodule, DWORD reason_for_call, LPVOID lp_reserved)
{
	switch (reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			ntapi::init(&nt_funcs);
			break;

		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}