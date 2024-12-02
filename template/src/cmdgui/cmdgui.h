// TEMPLATE

#pragma once

#include "../core/core.h"

namespace globals
{
	extern HANDLE console_handle;
	extern WORD select_color;
	extern WORD deselect_color;
}


namespace helper
{
	void print_unicode(const wchar_t* text);

	int map(int x, int in_min, int in_max, int out_min, int out_max);
}


class element
{
	public:

		virtual void display(int size, bool highlight) = 0;

		virtual float get_value() = 0;

		virtual int get_name_size() = 0;

		virtual void change_value(bool increment) = 0;
};


class toggle : public element
{
	private:

		std::string name;
		bool default_value;
		bool value;

	public:

		toggle();

		toggle(std::string name, bool default_value);

		~toggle();

		void display(int size, bool highlight) override;

		float get_value() override;

		int get_name_size() override;

		void change_value(bool increment) override;
};


class slider : public element
{
	private:

		std::string name;
		float default_value;
		float value;
		float min_value;
		float max_value;
		float step_value;

	public:

		slider();

		slider(std::string name, float default_value, float min_value, float max_value, float step_value);

		~slider();

		void display(int size, bool highlight) override;

		float get_value() override;

		int get_name_size() override;

		void change_value(bool increment) override;
};


class tab
{
	private:

		std::string name;
		std::vector<element*> elements;
		int size;

	public:

		tab();

		tab(std::string name);

		~tab();

		void insert(element* e);

		void display(int active_pointer);

		std::string get_tab_name();

		int get_elements_size();

		float get_element_value(int index);

		void inc_element_value(int index);

		void dec_element_value(int index);
};


class cmdgui
{
	private:

		std::vector<tab*> tabs;
		int active_tab;
		int active_pointer;
		bool unlocked;

	public:

		cmdgui();

		~cmdgui();

		void insert(tab* t);

		void display();

		void change_active_pointer(bool increment);

		void navigate();

		float get_value(int tab, int element);
};