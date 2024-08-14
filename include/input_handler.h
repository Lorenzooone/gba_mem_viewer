#ifndef INPUT_HANDLER__
#define INPUT_HANDLER__

#define CANCEL_MULTIBOOT 0xFF
#define NO_ADDRESS_CHANGE 0xFF
#define CONFIRM_MULTIBOOT 0x35
#define START_MULTIBOOT 0x49
#define SET_ADDRESS 0x71
#define CONFIRM_ADDRESS_CHANGE 0x97

u8 handle_input_multiboot_settings(u16, mu8*, mu8*);
u8 handle_input_multiboot_menu(u16);
u8 handle_input_address_settings(mu8*, u16, mu8*, uintptr_t*);
u8 handle_input_main_menu(u16, mu8*, uintptr_t*);

#endif
