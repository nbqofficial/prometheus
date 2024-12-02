// TEMPLATE

#include "cmdgui.h"


HANDLE globals::console_handle = 0;
WORD globals::select_color = 0x8;
WORD globals::deselect_color = 0x8;


void helper::print_unicode(const wchar_t* text)
{
	_setmode(_fileno(stdout), _O_WTEXT);
	wprintf(text);
	_setmode(_fileno(stdout), _O_TEXT);
}

int helper::map(int x, int in_min, int in_max, int out_min, int out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


toggle::toggle()
{
	this->name.clear();
	this->default_value = false;
	this->value = false;
}

toggle::toggle(std::string name, bool default_value)
{
	this->name = name;
	this->default_value = default_value;
	this->value = default_value;
}

toggle::~toggle()
{
}

void toggle::display(int size, bool highlight)
{
	helper::print_unicode(L"▌ ");
	if (highlight) { SetConsoleTextAttribute(globals::console_handle, globals::select_color); }

	printf(" %s", this->name.c_str());

	for (int i = 0; i < (size - this->name.size()); i++) { printf(" "); }

	printf(" |");

	for (int i = 0; i < (50 - size); i++)
	{ 
		if (this->value) { helper::print_unicode(L"■"); }
		else { printf(" "); }	
	}

	printf("| [%s", this->value ? "ON" : "OFF");

	int value_size = 2;
	if (this->value) { value_size = 3; }

	for (int i = 0; i < value_size; i++) { printf(" "); }
	printf("] ");
	SetConsoleTextAttribute(globals::console_handle, globals::deselect_color);
	helper::print_unicode(L" ▐");
	printf("\n");
}

float toggle::get_value()
{
	return (float)this->value;
}

int toggle::get_name_size()
{
	return this->name.size();
}

void toggle::change_value(bool increment)
{
	if (increment)
	{
		if (!this->value) { this->value = true; }
	}
	else
	{
		if (this->value) { this->value = false; }
	}
}


slider::slider()
{
	this->name.clear();
	this->default_value = 0.0f;
	this->value = 0.0f;
	this->min_value = 0.0f;
	this->max_value = 0.0f;
	this->step_value = 0.0f;
}

slider::slider(std::string name, float default_value, float min_value, float max_value, float step_value)
{
	this->name = name;
	this->default_value = default_value;
	this->value = default_value;
	this->min_value = min_value;
	this->max_value = max_value;
	this->step_value = step_value;
}

slider::~slider()
{
}

void slider::display(int size, bool highlight)
{
	helper::print_unicode(L"▌ ");
	if (highlight) { SetConsoleTextAttribute(globals::console_handle, globals::select_color); }

	printf(" %s", this->name.c_str());

	for (int i = 0; i < (size - this->name.size()); i++) { printf(" "); }

	printf(" |");

	int v = helper::map(this->value, this->min_value, this->max_value, 0, (50 - size));
	int l = 50 - v;

	for (int i = 0; i < v; i++) { helper::print_unicode(L"■"); }

	for (int i = 0; i < (l - size); i++) { printf(" "); }

	int number = (int)this->value;
	int temp = 0;
	if (number <= 0) { temp = 1; }
	while (number)
	{
		number /= 10;
		temp++;
	}

	printf("| [%.1f", this->value);

	for (int i = 0; i < (3 - temp); i++) { printf(" "); }
	printf("] ");
	SetConsoleTextAttribute(globals::console_handle, globals::deselect_color);
	helper::print_unicode(L" ▐");
	printf("\n");
}

float slider::get_value()
{
	return (float)this->value;
}

int slider::get_name_size()
{
	return this->name.size();
}

void slider::change_value(bool increment)
{
	if (increment)
	{
		if ((this->value + this->step_value) <= this->max_value)
		{
			this->value += this->step_value;
		}
	}
	else
	{
		if ((this->value - this->step_value) >= 0.0f)
		{
			this->value -= this->step_value;
		}
	}
}


tab::tab()
{
	this->name.clear();
	this->elements.clear();
	this->size = 0;
}

tab::tab(std::string name)
{
	this->name = name;
}

tab::~tab()
{
}

void tab::insert(element* e)
{
	if (this->size < e->get_name_size()) { this->size = e->get_name_size(); }

	this->elements.push_back(e);
}

void tab::display(int active_pointer)
{
	for (int i = 0; i < 67; i++) { helper::print_unicode(L"▄"); }
	printf("\n");
	helper::print_unicode(L"▌");
	for (int i = 0; i < 65; i++) { printf(" "); }
	helper::print_unicode(L"▐");
	printf("\n");

	for (int i = 0; i < this->elements.size(); i++)
	{
		if (i == active_pointer)
		{
			this->elements[i]->display(this->size, true);
		}
		else
		{
			this->elements[i]->display(this->size, false);
		}
	}
	helper::print_unicode(L"▌");
	for (int i = 0; i < 65; i++) { printf(" "); }
	helper::print_unicode(L"▐");
	printf("\n");
	for (int i = 0; i < 67; i++) { helper::print_unicode(L"▀"); }
	printf("\n");
}

std::string tab::get_tab_name()
{
	return this->name;
}

int tab::get_elements_size()
{
	return this->elements.size();
}

float tab::get_element_value(int index)
{
	return this->elements[index]->get_value();
}

void tab::inc_element_value(int index)
{
	this->elements[index]->change_value(true);
}

void tab::dec_element_value(int index)
{
	this->elements[index]->change_value(false);
}


cmdgui::cmdgui()
{
	globals::console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	this->tabs.clear();
	this->active_tab = 0;
	this->active_pointer = 0;
	this->unlocked = false;
}

cmdgui::~cmdgui()
{
	CloseHandle(globals::console_handle);
}

void cmdgui::insert(tab* t)
{
	this->tabs.push_back(t);
}

void cmdgui::display()
{
	printf("\n   ");
	SetConsoleTextAttribute(globals::console_handle, 0x8);

	for (int i = 0; i < this->tabs.size(); i++)
	{
		if (i == this->active_tab) { SetConsoleTextAttribute(globals::console_handle, globals::select_color); }
		printf("%s [F%d]", this->tabs[i]->get_tab_name().c_str(), (i + 1));
		SetConsoleTextAttribute(globals::console_handle, globals::deselect_color);
		printf("   ");
	}
	printf("\n");
	this->tabs[this->active_tab]->display(this->active_pointer);
}

void cmdgui::change_active_pointer(bool increment)
{
	if (increment)
	{
		if (this->active_pointer == (this->tabs[this->active_tab]->get_elements_size() - 1))
		{
			this->active_pointer = 0;
		}
		else
		{
			this->active_pointer++;
		}
	}
	else
	{
		if (this->active_pointer == 0)
		{
			this->active_pointer = (this->tabs[this->active_tab]->get_elements_size() - 1);
		}
		else
		{
			this->active_pointer--;
		}
	}
}

void cmdgui::navigate()
{
	if (this->unlocked)
	{
		if (GetAsyncKeyState(VK_DOWN))
		{
			change_active_pointer(true);
			system("cls");
			display();
			Sleep(100);
		}
		else if (GetAsyncKeyState(VK_UP))
		{
			change_active_pointer(false);
			system("cls");
			display();
			Sleep(100);
		}
		else if (GetAsyncKeyState(VK_LEFT))
		{
			this->tabs[active_tab]->dec_element_value(this->active_pointer);
			system("cls");
			display();
			Sleep(100);
		}
		else if (GetAsyncKeyState(VK_RIGHT))
		{
			this->tabs[active_tab]->inc_element_value(this->active_pointer);
			system("cls");
			display();
			Sleep(100);
		}
		else if (GetAsyncKeyState(VK_F1))
		{
			if (this->tabs.size() > 0)
			{
				this->active_tab = 0;
			}
			system("cls");
			display();
			Sleep(100);
		}
		else if (GetAsyncKeyState(VK_F2))
		{
			if (this->tabs.size() > 1)
			{
				this->active_tab = 1;
			}
			system("cls");
			display();
			Sleep(100);
		}
		else if (GetAsyncKeyState(VK_F3))
		{
			if (this->tabs.size() > 2)
			{
				this->active_tab = 2;
			}
			system("cls");
			display();
			Sleep(100);
		}
		else if (GetAsyncKeyState(VK_F4))
		{
			if (this->tabs.size() > 3)
			{
				this->active_tab = 3;
			}
			system("cls");
			display();
			Sleep(100);
		}
		else if (GetAsyncKeyState(VK_F5))
		{
			if (this->tabs.size() > 4)
			{
				this->active_tab = 4;
			}
			system("cls");
			display();
			Sleep(100);
		}
		else if (GetAsyncKeyState(VK_F6))
		{
			if (this->tabs.size() > 5)
			{
				this->active_tab = 5;
			}
			system("cls");
			display();
			Sleep(100);
		}
		else if (GetAsyncKeyState(VK_F7))
		{
			if (this->tabs.size() > 6)
			{
				this->active_tab = 6;
			}
			system("cls");
			display();
			Sleep(100);
		}
		else if (GetAsyncKeyState(VK_F8))
		{
			if (this->tabs.size() > 7)
			{
				this->active_tab = 7;
			}
			system("cls");
			display();
			Sleep(100);
		}
	}
	if (GetAsyncKeyState(VK_INSERT))
	{
		this->unlocked = !this->unlocked;
		if (this->unlocked)
		{
			globals::select_color = 0xC;
			globals::deselect_color = 0x7;
		}
		else
		{
			globals::select_color = 0x8;
			globals::deselect_color = 0x8;
		}
		system("cls");
		display();
		Sleep(100);
	}
}

float cmdgui::get_value(int tab, int element)
{
	return this->tabs[tab]->get_element_value(element);
}
