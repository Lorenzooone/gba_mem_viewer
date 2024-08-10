#ifndef INPUT_HANDLER__
#define INPUT_HANDLER__

#define CANCEL_MULTIBOOT 0xFF
#define EXIT_BASE_SETTINGS 0xFF
#define EXIT_COLOURS_SETTINGS 0xFF
#define NO_ADDRESS_CHANGE 0xFF
#define ENTER_COLOUR_MENU 1
#define CONFIRM_MULTIBOOT 0x35
#define START_MULTIBOOT 0x49
#define SET_ADDRESS 0x71
#define START_SETTINGS_MENU 0x84
#define CONFIRM_ADDRESS_CHANGE 0x97

u8 handle_input_multiboot_settings(u16, mu8*, mu8*);
u8 handle_input_multiboot_menu(u16);
u8 handle_input_address_settings(mu8*, u16, mu8*, uintptr_t*);
u8 handle_input_main_menu(mu8*, u16, mu8*, uintptr_t*);
u8 handle_input_base_settings_menu(u16, mu8*, mu8*);
u8 handle_input_colours_menu(u16, mu8*, mu8*, mu8*);
u8 handle_input_gen12_settings_menu(u16, mu8*, mu8*);

#endif
