// DRIVER

#pragma once

#include "../nt/ntapi.h"

#define WIN10_2009 19042
#define WIN10_2104 19043

typedef struct _WIN_OFFSETS
{
	ULONG ACTIVE_PROCESS_LINKS;
	ULONG IMAGE_FILE_NAME;
}WIN_OFFSETS, *PWIN_OFFSETS;

namespace win_offsets
{
	extern WIN_OFFSETS wofs;

	bool init(PWIN_OFFSETS ofs);
}