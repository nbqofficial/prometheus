// DRIVER

#include "win_offsets.h"

WIN_OFFSETS win_offsets::wofs = { 0 };

bool win_offsets::init(PWIN_OFFSETS ofs)
{
	RTL_OSVERSIONINFOW os_info;
	if (!NT_SUCCESS(RtlGetVersion(&os_info))) { return false; }

	switch (os_info.dwBuildNumber)
	{
		case WIN10_2009: 
			ofs->ACTIVE_PROCESS_LINKS = 0x448;
			ofs->IMAGE_FILE_NAME = 0x5a8;
			break;
		case WIN10_2104: 
			ofs->ACTIVE_PROCESS_LINKS = 0x448;
			ofs->IMAGE_FILE_NAME = 0x5a8;
			break;
		default:
			ofs->ACTIVE_PROCESS_LINKS = 0x448;
			ofs->IMAGE_FILE_NAME = 0x5a8;
			break;
	}
	return true;
}