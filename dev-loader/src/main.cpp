// DEV LOADER

#include "../src/mapper/mapper.h"

bool ldr(std::string dll_path)
{
	printf("[DEV-LOADER]: Opening core.dll\n");
	HANDLE core_dll = CreateFileA("core.dll", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (core_dll != INVALID_HANDLE_VALUE)
	{
		DWORD core_dll_size = GetFileSize(core_dll, 0);
		printf("[DEV-LOADER]: Allocating %d bytes for core.dll\n", core_dll_size);
		byte* core_dll_array = (byte*)malloc(core_dll_size);
		ReadFile(core_dll, core_dll_array, core_dll_size, 0, 0);
		printf("[DEV-LOADER]: Allocated core.dll at %p\n", core_dll_array);
		mapper::run_dll(core_dll_array, true, &coreapi);
		free(core_dll_array);
		CloseHandle(core_dll);

		printf("[DEV-LOADER]: Opening %s\n", dll_path.c_str());
		HANDLE cheat_dll = CreateFileA(dll_path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if (cheat_dll != INVALID_HANDLE_VALUE)
		{
			DWORD cheat_dll_size = GetFileSize(cheat_dll, 0);
			printf("[DEV-LOADER]: Allocating %d bytes for %s\n", cheat_dll_size, dll_path.c_str());
			byte* cheat_dll_array = (byte*)malloc(cheat_dll_size);
			ReadFile(cheat_dll, cheat_dll_array, cheat_dll_size, 0, 0);
			printf("[DEV-LOADER]: Allocated %s at %p\n", dll_path.c_str(), cheat_dll_array);
			printf("[DEV-LOADER]: EXECUTING %s\n\n", dll_path.c_str());
			mapper::run_dll(cheat_dll_array, false, &coreapi);
			free(cheat_dll_array);
			CloseHandle(cheat_dll);
			return true;
		}
		else
		{
			printf("[DEV-LOADER]: Unable to open %s\n", dll_path.c_str());
			return false;
		}
	}
	else
	{
		printf("[DEV-LOADER]: Unable to open core.dll\n");
		return false;
	}
}

int main()
{
	timeBeginPeriod(1);
	std::string dll_path;
	printf("[DEV-LOADER]: ENTER THE DLL PATH\n");
	std::cin >> dll_path;
	system("cls");
	if (!dll_path.empty())
	{
		if (ldr(dll_path))
		{
			for (;;) { Sleep(100000); }
		}
		else
		{
			printf("[DEV-LOADER]: FAILED TO INITIALIZE\n");
		}
	}
	else
	{
		printf("[DEV-LOADER]: INVALID DLL PATH\n");
	}

	system("pause");
	timeEndPeriod(1);

	return 0;
}