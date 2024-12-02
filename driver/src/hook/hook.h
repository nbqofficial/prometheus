// DRIVER

#pragma once

#include "../mouse/mouse.h"

namespace hook
{
	void* get_shellcode(uintptr_t hook_function);

	void* get_code_cave();
}