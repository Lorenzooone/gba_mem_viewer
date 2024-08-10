#include "base_include.h"
#include "menu_text_handler.h"
#include "input_handler.h"
#include "sprite_handler.h"
#include "print_system.h"
#include "window_handler.h"
#include "config_settings.h"
#include "save.h"
#include <stddef.h>

#define NUM_BYTES_PER_LINE 8

#define NUM_LINES 10
#define MAIN_MENU_DISTANCE_FROM_BORDER 2

#define SUMMARY_LINE_MAX_SIZE 18
#define PRINTABLE_INVALID_STRINGS 3

void print_bottom_info(void);
void print_single_colour_info(u8);

static uint32_t prev_read_data[16][NUM_BYTES_PER_LINE / 4];

const char colours_chars[] = {'R', 'G', 'B'};

void print_crash(enum CRASH_REASONS reason) {
    reset_screen(BLANK_FILL);
    
    init_crash_window();
    clear_crash_window();
    
    set_text_y(CRASH_WINDOW_Y);
    set_text_x(CRASH_WINDOW_X);
    PRINT_FUNCTION("      CRASHED!\n\n");
    set_text_x(CRASH_WINDOW_X);
    switch(reason) {
        case BAD_SAVE:
            PRINT_FUNCTION("ISSUES WHILE SAVING!\n\n");
            break;
        case BAD_TRADE:
            PRINT_FUNCTION("ISSUES WHILE TRADING!\n\n");
            break;
        case CARTRIDGE_REMOVED:
            PRINT_FUNCTION(" CARTRIDGE REMOVED!\n\n");
            break;
        default:
            break;
    }
    set_text_x(CRASH_WINDOW_X);
    PRINT_FUNCTION("TURN OFF THE CONSOLE.");
}

void print_base_settings_menu(u8 update) {
    if(!update)
        return;

    default_reset_screen();
    PRINT_FUNCTION("\n  Color Settings\n\n");

    print_bottom_info();
    const struct version_t* version = get_version();
    set_text_x(X_LIMIT-(4+(3*3)+1));
    PRINT_FUNCTION("V.\x03.\x03.\x03\x02", version->main_version, version->sub_version, version->revision_version, version->revision_letter);
}

void print_single_colour_info(u8 colour) {
    for(int i = 0; i < NUM_SUB_COLOURS; i++) {
        set_text_x((BASE_X_CURSOR_COLOURS_SETTINGS_MENU_IN>>3)+(i*(BASE_X_CURSOR_INCREMENT_COLOURS_SETTINGS_MENU_IN>>3))+2);
        if(colour < NUM_COLOURS)
            PRINT_FUNCTION("\x0B", get_single_colour(colour, i), 2);
        else
            PRINT_FUNCTION(" \x02", colours_chars[i]);
    }
}

void print_colour_settings_menu(u8 update) {
    if(!update)
        return;

    default_reset_screen();
    PRINT_FUNCTION("\n  Colour");
    print_single_colour_info(NUM_COLOURS);
    set_text_y(BASE_Y_CURSOR_COLOURS_SETTINGS_MENU>>3);
    PRINT_FUNCTION("  Background:");
    print_single_colour_info(BACKGROUND_COLOUR_POS);
    set_text_y((BASE_Y_CURSOR_COLOURS_SETTINGS_MENU>>3)+(BASE_Y_CURSOR_INCREMENT_COLOURS_SETTINGS_MENU>>3));
    PRINT_FUNCTION("  Font:");
    print_single_colour_info(FONT_COLOUR_POS);
    set_text_y((BASE_Y_CURSOR_COLOURS_SETTINGS_MENU>>3)+((BASE_Y_CURSOR_INCREMENT_COLOURS_SETTINGS_MENU>>3)*2));
    PRINT_FUNCTION("  Window 1:");
    print_single_colour_info(WINDOW_COLOUR_1_POS);
    set_text_y((BASE_Y_CURSOR_COLOURS_SETTINGS_MENU>>3)+((BASE_Y_CURSOR_INCREMENT_COLOURS_SETTINGS_MENU>>3)*3));
    PRINT_FUNCTION("  Window 2:");
    print_single_colour_info(WINDOW_COLOUR_2_POS);
    set_text_y((BASE_Y_CURSOR_COLOURS_SETTINGS_MENU>>3)+((BASE_Y_CURSOR_INCREMENT_COLOURS_SETTINGS_MENU>>3)*4));
    PRINT_FUNCTION("  Cursor:");
    print_single_colour_info(SPRITE_COLOUR_POS);
    
    init_colour_window();
    clear_colour_window();
    
    set_text_y(COLOURS_WINDOW_Y+(COLOURS_WINDOW_Y_SIZE>>1));
    set_text_x(COLOURS_WINDOW_X+(COLOURS_WINDOW_X_SIZE>>1)-((11+1)>>1));
    PRINT_FUNCTION("Test Window");

    print_bottom_info();
    PRINT_FUNCTION(" - UP/DOWN: Change");
}

void print_waiting(s8 y_increase){
    reset_screen(BLANK_FILL);
    init_waiting_window(y_increase);
    clear_waiting_window(y_increase);
    set_text_y(WAITING_WINDOW_Y + y_increase);
    set_text_x(WAITING_WINDOW_X);
    PRINT_FUNCTION("Waiting...");
}

void print_loading(){
    reset_screen(BLANK_FILL);
    init_loading_window();
    clear_loading_window();
    set_text_y(LOADING_WINDOW_Y);
    set_text_x(LOADING_WINDOW_X);
    PRINT_FUNCTION("Loading...");
}

void print_bottom_info(){
    set_text_y(Y_LIMIT-1);
    PRINT_FUNCTION("B: Go Back");
}

void print_address_setting(u8 update, u8 cursor_x_pos, uintptr_t address) {
	if(!update)
		return;

    reset_screen(BLANK_FILL);
    init_address_window();
    clear_address_window();
    set_text_y(ADDRESS_WINDOW_Y);
    set_text_x(ADDRESS_WINDOW_X + 3 + cursor_x_pos);
    PRINT_FUNCTION("+");
    set_text_y(ADDRESS_WINDOW_Y + 1);
    set_text_x(ADDRESS_WINDOW_X);
    PRINT_FUNCTION("0x\x0D", address, 8);
    set_text_y(ADDRESS_WINDOW_Y + 2);
    set_text_x(ADDRESS_WINDOW_X + 3 + cursor_x_pos);
    PRINT_FUNCTION("-");
}

void print_main_menu(u8 update, uintptr_t address) {
	uint32_t read_data[16][NUM_BYTES_PER_LINE / 4];

	for(int i = 0; i < 16; i++) {
		uintptr_t internal_address = (address + (i * NUM_BYTES_PER_LINE)) & 0x0FFFFFFF;
		uint32_t* data = (uint32_t*)internal_address;
		if((internal_address >= SRAM) && (internal_address < (SRAM + 0x20000)))
			copy_save_to_ram_u16(internal_address, (u16*)read_data[i], NUM_BYTES_PER_LINE);
		else
			for(int j = 0; j < (NUM_BYTES_PER_LINE / 4); j++) {
				uint32_t curr_read_data = data[j];
				read_data[i][j] = curr_read_data;
			}
	}

	if(!update) {
		for(int i = 0; i < 16; i++) {
			for(int j = 0; j < (NUM_BYTES_PER_LINE / 4); j++)
				if(prev_read_data[i][j] != read_data[i][j]) {
					update = 1;
					break;
				}
			if(update)
				break;
		}
	}

    if(!update)
        return;

	for(int i = 0; i < 16; i++)
		for(int j = 0; j < (NUM_BYTES_PER_LINE / 4); j++)
			prev_read_data[i][j] = read_data[i][j];

    default_reset_screen();

	for(int i = 0; i < 16; i++) {
		PRINT_FUNCTION("\x0D: \x0D \x0D \x0D \x0D \x0D \x0D \x0D \x0D\n", i * NUM_BYTES_PER_LINE, 2, read_data[i][0] & 0xFF, 2, (read_data[i][0] >> 8) & 0xFF, 2, (read_data[i][0] >> 16) & 0xFF, 2, (read_data[i][0] >> 24) & 0xFF, 2, read_data[i][1] & 0xFF, 2, (read_data[i][1] >> 8) & 0xFF, 2, (read_data[i][1] >> 16) & 0xFF, 2, (read_data[i][1] >> 24) & 0xFF, 2);
	}
	PRINT_FUNCTION("Start Addr.: 0x\x0D", address, 8);
	PRINT_FUNCTION("\nUp:   -0x10 - Down:  +0x10");
	PRINT_FUNCTION("\nLeft: -0x80 - Right: +0x80");
	PRINT_FUNCTION("\nA/Start: Addr. - B/Select: MB");
}

void print_multiboot_settings(u8 is_normal, u8 update) {
    if(!update)
        return;

    default_reset_screen();
    PRINT_FUNCTION("\nCable type: ");
    if(is_normal)
        PRINT_FUNCTION(" GBC> (Fast)");
    else
        PRINT_FUNCTION("<GBA  (Slow)");
    set_text_y(Y_LIMIT-1);
    PRINT_FUNCTION("A: Start - B: Go Back");
}

void print_multiboot_mid_process(u8 initial_handshake) {
    default_reset_screen();
    PRINT_FUNCTION("\nInitiating handshake!\n");
    
    if(initial_handshake)
        PRINT_FUNCTION("\nHandshake successful!\n\nGiving control to SWI 0x25!");
}

void print_multiboot(enum MULTIBOOT_RESULTS result) {

    default_reset_screen();
    
    if(result == MB_SUCCESS)
        PRINT_FUNCTION("\nMultiboot successful!");
    else if(result == MB_NO_INIT_SYNC)
        PRINT_FUNCTION("\nCouldn't sync.\n\nTry again!");
    else
        PRINT_FUNCTION("\nThere was an error.\n\nTry again!");
    
    set_text_y(7);
    PRINT_FUNCTION("A: To the previous menu\n\n");
    PRINT_FUNCTION("The cartridge will be\n");
    PRINT_FUNCTION("read once more.");
}
