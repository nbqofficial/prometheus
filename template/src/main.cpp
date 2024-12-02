// TEMPLATE

// ----------------------------------------------------------------------
// WHAT DOES THIS TEMPLATE INCLUDE?							
// ----------------------------------------------------------------------
// CORE LIB: EXPORTED API CALLS (FOR INTERFACING WITH THE KERNEL DRIVER)
// CMDGUI LIB: FOR EASILY DESIGNING BASIC COMMAND LINE MENU INTERFACES
// 
// IN THE MAIN ROUTINE YOU CAN ALL THE API CALLS AND THEIR EXPLANATION
// ----------------------------------------------------------------------

#include "../src/cmdgui/cmdgui.h"

void main_routine()
{
	IsGUIThread(TRUE);																									// only necessary to call if you intend to use frame rendering functions (frame_init, frame_rect etc.)
																														// because rendering functions must be called from win32 context thread

	// CMDGUI EXPLAINED
	toggle example_toggle("EXAMPLE SWITCH", false);																		// defines a cmdgui element of type toggle with the display name "EXAMPLE SWITCH" and default value of false
	slider example_slider("EXAMPLE SLIDER", 5.0, 0.0, 10.0, 1.0);														// defines a cmdgui element of type slider with the display name "EXAMPLE SLIDER", default value of 5.0, 
																														// minimum value of 0.0, maximum value of 10.0 and a step/increment value of 1.0

	tab example_tab("EXAMPLE TAB");																						// defines a cmdgui tab element with the display name "EXAMPLE TAB" which we can add toggle/slider elements
	example_tab.insert(&example_toggle);																				// places the toggle element as the first element in the tab row
	example_tab.insert(&example_slider);																				// places the slider element as the second element in the tab row

	cmdgui gui;																											// initializes the gui
	gui.insert(&example_tab);																							// places the tab element in the menu, if we had more tabs we could insert them in order of their appearance in the menu

	gui.navigate();																										// should be called in a loop because it checks whether the values are changing by monitoring key presses

	bool toggle_value = (bool)gui.get_value(0, 0);																		// retrieves the value of an element at tab index 0 and element index 0 which in this case is a toggle (thus the bool cast)
	float slider_value = (float)gui.get_value(0, 1);																	// retrieves the value of an element at tab index 0 and element index 1 which in this case is a slider (thus the float cast)


	// CORE EXPLAINED
	ULONG status = coreapi->get_status(driver_handle);																	// queries the kernel driver to get a status information (12345 == STATUS_SUCCESS, ANY_OTHER_VALUE == STATUS_FAILED)
																														// we use this to make sure that the kernel driver is loaded correctly and we have an established communication
	
	char process_name[] = "test.exe";
	ULONG process_id = coreapi->get_process_id_by_name(driver_handle, process_name, sizeof(process_name));				// retrieves the process identifier given the process name

	uintptr_t pml4 = coreapi->get_pml4(driver_handle, process_id);														// retrieves the page table map level 4 given the process id (this is used in read/write memory calls)
	
	void* section_base = coreapi->get_section_base(driver_handle, process_id);											// retrieves the section base (main module base) of a process given its process id

	wchar_t module_name[] = L"test.dll";
	void* module_base = coreapi->get_module_base(driver_handle, process_id, module_name, sizeof(module_name), true);	// retrieves the base address of any loaded module by name within a given process specified by process id

	byte buffer[100];
	coreapi->read_memory(driver_handle, pml4, (void*)0xDEADBEEF, buffer, sizeof(buffer));								// reads a certain size of memory at a specified virtual address and stores it inside the buffer
	coreapi->write_memory(driver_handle, pml4, (void*)0xDEADBEEF, buffer, sizeof(buffer));								// writes a certain size buffer at a specified virtual address

	bool mouse_status = coreapi->mouse_init(driver_handle);																// initializes the mouse device used for mouse simulation (must be done before calling coreapi->mouse_evt())
	coreapi->mouse_evt(driver_handle, 10, 10, 0);																		// works similar to mouse_event WINAPI, in this case it simulates mouse movement by 10 pixels in x and y direction

	bool drawing_status = coreapi->frame_init(driver_handle);															// initializes the rendering device used for rendering esp (must be done before calling coreapi->frame_rect(), coreapi->frame_fill() and coreapi->frame_metrics())
	
	int width, height;
	coreapi->frame_metrics(driver_handle, &width, &height);																// retrieves the screen resolution size of your display and stores it in width and height variables

	coreapi->frame_rect(driver_handle, 0, 0, 100, 100, 1, 255, 255, 255);												// draws a white rectangle with the size of 100 and thickness of 1 at the top left corner of the screen
	coreapi->frame_fill(driver_handle, 0, 0, 50, 50, 255, 0, 0);														// draws a red solid rectangle with the size of 50 at the top left corner of the screen

	// KEEP IN MIND THAT RUNNING THIS TEMPLATE LIKE THIS IS NOT RECOMMENDED
	// THIS IS JUST MEANT TO SERVE AS A LEARNING EXAMPLE OF HOW TO USE THE API CORRECTLY
	// SOME API CALLS MUST BE DONE BEFORE OTHERS FOR EVERYTHING TO WORK PROPERLY
	// SINCE THE KERNEL DRIVER IS INVOLVED, ANY ISSUES COULD CAUSE A BSOD
}

BOOL WINAPI DllMain(HMODULE hmod, DWORD reason_for_call, LPVOID lp_reserved)
{
	if (reason_for_call == DLL_PROCESS_ATTACH)
	{
		coreapi = (PCORE_API)lp_reserved;																				// lp_reserved contains pointer to coreapi struct (we cast it so we can use it)
		driver_handle = coreapi->attach();																				// attach to the driver framework (global driver handle to use in every file)

		if (driver_handle)
		{
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)main_routine, 0, 0, 0);											// if the driver handle is valid we run the main routine
		}
	}
	else if (reason_for_call == DLL_PROCESS_DETACH)
	{
		coreapi->detach(driver_handle);																					// detach from the driver framework on detach event
	}
	return TRUE;
}