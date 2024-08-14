#include "base_include.h"
#include "menu_text_handler.h"
#include "input_handler.h"
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

static uint32_t prev_read_data[16][NUM_BYTES_PER_LINE / 4];

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
}
