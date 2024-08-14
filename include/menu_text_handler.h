#ifndef MENU_TEXT_HANDLER__
#define MENU_TEXT_HANDLER__

#include "multiboot_handler.h"
#include "config_settings.h"

void print_address_setting(u8, u8, uintptr_t);
void print_main_menu(u8, uintptr_t);
void print_multiboot_settings(u8, u8);
void print_multiboot_mid_process(u8);
void print_multiboot(enum MULTIBOOT_RESULTS);

#endif
