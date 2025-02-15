#pragma once
#include <m_pd.h>


typedef void(*pd_gui_callback)(void*, void*);
typedef void(*pd_panel_callback)(void*, int, const char*, const char*);
typedef void(*pd_synchronise_callback)(void*, void*);

void register_gui_triggers(t_pdinstance* instance, void* target, pd_gui_callback gui_callback, pd_panel_callback panel_callback, pd_synchronise_callback synchronise_callback);
