#ifndef MENU_TEXT_HANDLER__
#define MENU_TEXT_HANDLER__

#include "multiboot_handler.h"
#include "config_settings.h"

enum CRASH_REASONS{BAD_SAVE, BAD_TRADE, CARTRIDGE_REMOVED};

void print_crash(enum CRASH_REASONS);
void print_address_setting(u8, u8, uintptr_t);
void print_main_menu(u8, uintptr_t);
void print_multiboot_settings(u8, u8);
void print_multiboot_mid_process(u8);
void print_multiboot(enum MULTIBOOT_RESULTS);
void print_waiting(s8);
void print_loading(void);
void print_base_settings_menu(u8);
void print_colour_settings_menu(u8);

#endif
