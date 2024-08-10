#include "base_include.h"
#include "multiboot_handler.h"
#include "graphics_handler.h"
#include "sprite_handler.h"
#include "input_handler.h"
#include "menu_text_handler.h"
#include "rng.h"
#include "print_system.h"
#include "window_handler.h"
#include "sio.h"
#include <stddef.h>
#include "optimized_swi.h"
#include "timing_basic.h"
#include "config_settings.h"
#include "save.h"

#include "ewram_speed_check_bin.h"

#define REG_MEMORY_CONTROLLER_ADDR 0x4000800
#define HW_SET_REG_MEMORY_CONTROLLER_VALUE 0x0D000020
#define REG_MEMORY_CONTROLLER *((u32*)(REG_MEMORY_CONTROLLER_ADDR))

#define WORST_CASE_EWRAM 1
#define TEST_LAG_EWRAM 0
#define MIN_WAITCYCLE 1

#define WAITING_TIME_MOVE_MESSAGES (2*FPS)
#define WAITING_TIME_BEFORE_RESPONSE (1*FPS)
#define WAITING_TIME_REJECTED (2*FPS)
#define MAX_RANDOM_WAIT_TIME (1*FPS)

#define BASE_SCREEN 0
#define LEARNABLE_MOVES_MENU_SCREEN 2
#define TRADE_CANCEL_SCREEN 1
#define INFO_SCREEN 3
#define NATURE_SCREEN 2
#define IV_FIX_SCREEN 2
#define EVOLUTION_MENU_SCREEN 2

void vblank_update_function(void);
void find_optimal_ewram_settings(void);
void disable_all_irqs(void);
u8 init_cursor_y_pos_main_menu(void);
void cursor_update_main_menu(u8);
void cursor_update_base_settings_menu(u8);
void cursor_update_colours_settings_menu(u8, u8);
void loading_print_screen(void);
void waiting_init(s8);
void address_setting_init(mu8*, uintptr_t);
void main_menu_init(mu8*, uintptr_t);
void base_settings_menu_init(mu8*);
void colours_settings_menu_init(mu8*, mu8*);
void prepare_crash_screen(enum CRASH_REASONS);
void crash_on_cartridge_removed(void);
void wait_frames(int);
int main(void);

enum STATE {MAIN_MENU, MULTIBOOT, WAITING_DATA, BASE_SETTINGS_MENU, COLOURS_SETTINGS_MENU, MULTIBOOT_SETTINGS_MENU, ADDRESS_SETTING_MENU};
mu8 curr_state;
u32 counter = 0;
u32 input_counter = 0;

IWRAM_CODE void vblank_update_function() {
    REG_IF |= IRQ_VBLANK;

    flush_screens();

    move_sprites(counter);
    move_cursor_x(counter);
    advance_rng();
    counter++;

    #ifdef __NDS__
    // Increase FPS on NDS
    //__reset_vcount();
    #endif
}

IWRAM_CODE void find_optimal_ewram_settings() {
/*
    #ifdef __GBA__
    size_t size = ewram_speed_check_bin_size>>2;
    const u32* ewram_speed_check = (const u32*)ewram_speed_check_bin;
    u32 test_data[size];
    
    // Check for unsupported (DS)
    if(REG_MEMORY_CONTROLLER != HW_SET_REG_MEMORY_CONTROLLER_VALUE)
        return;
    
    if(TEST_LAG_EWRAM)
        REG_MEMORY_CONTROLLER &= ~(0xF<<24);
    
    // Check for worst case testing (Not for final release)
    if(WORST_CASE_EWRAM)
        return;
    
    // Prepare data to test against
    for(size_t i = 0; i < size; i++)
        test_data[i] = ewram_speed_check[i];
    
    // Detetmine minimum number of stable waitcycles
    for(int i = 0; i < (16-MIN_WAITCYCLE); i++) {
        REG_MEMORY_CONTROLLER &= ~(0xF<<24);
        REG_MEMORY_CONTROLLER |= (15-i-MIN_WAITCYCLE)<<24;
        u8 failed = 0;
        for(size_t j = 0; (!failed) && (j < size); j++)
            if(test_data[j] != ewram_speed_check[j])
                failed = 1;
        if(!failed)
            return;
    }
    #endif
*/
}

void disable_all_irqs() {
    REG_IME = 0;
    REG_IE = 0;
}

u8 init_cursor_y_pos_main_menu() {
    return 0;
}

void cursor_update_main_menu(u8 cursor_y_pos) {
    //update_cursor_y(BASE_Y_CURSOR_MAIN_MENU + (BASE_Y_CURSOR_INCREMENT_MAIN_MENU * cursor_y_pos));
}

void cursor_update_base_settings_menu(u8 cursor_y_pos) {
    update_cursor_y(BASE_Y_CURSOR_BASE_SETTINGS_MENU + (BASE_Y_CURSOR_INCREMENT_BASE_SETTINGS_MENU * cursor_y_pos));
}

void cursor_update_colours_settings_menu(u8 cursor_y_pos, u8 cursor_x_pos) {
    if(!cursor_x_pos)
        update_cursor_base_x(BASE_X_CURSOR_COLOURS_SETTINGS_MENU);
    else
        update_cursor_base_x(BASE_X_CURSOR_COLOURS_SETTINGS_MENU_IN + ((cursor_x_pos-1) * BASE_X_CURSOR_INCREMENT_COLOURS_SETTINGS_MENU_IN));
    update_cursor_y(BASE_Y_CURSOR_COLOURS_SETTINGS_MENU + (BASE_Y_CURSOR_INCREMENT_COLOURS_SETTINGS_MENU * cursor_y_pos));
}

void wait_frames(int num_frames) {
    int wait_counter = 0;
    while(wait_counter < num_frames) {
        VBlankIntrWait();
        wait_counter++;
    }
}

void loading_print_screen() {
    u8 prev_screen = get_screen_num();
    set_screen(LOADING_WINDOW_SCREEN);
    print_loading();
    enable_screen(LOADING_WINDOW_SCREEN);
    set_screen(prev_screen);
    prepare_flush();
}

void waiting_init(s8 y_increase) {
    curr_state = WAITING_DATA;
    set_screen(WAITING_WINDOW_SCREEN);
    print_waiting(y_increase);
    enable_screen(WAITING_WINDOW_SCREEN);
    prepare_flush();
}

void address_setting_init(mu8* cursor_x_pos, uintptr_t address) {
    curr_state = ADDRESS_SETTING_MENU;
    set_screen(ADDRESS_WINDOW_SCREEN);
    *cursor_x_pos = 0;
    print_address_setting(1, *cursor_x_pos, address);
    enable_screen(ADDRESS_WINDOW_SCREEN);
    prepare_flush();
}

void main_menu_init(mu8* cursor_y_pos, uintptr_t address) {
    curr_state = MAIN_MENU;
    set_screen(BASE_SCREEN);
    set_bg_pos(BASE_SCREEN, 0, 0);
    print_main_menu(1, address);
    disable_all_screens_but_current();
    reset_sprites_to_cursor(1);
    disable_all_cursors();
    *cursor_y_pos = init_cursor_y_pos_main_menu();
    update_cursor_base_x(BASE_X_CURSOR_MAIN_MENU);
    cursor_update_main_menu(*cursor_y_pos);
    enable_screen(BASE_SCREEN);
    prepare_flush();
}

void base_settings_menu_init(mu8* cursor_y_pos) {
    curr_state = BASE_SETTINGS_MENU;
    set_screen(BASE_SCREEN);
    disable_all_screens_but_current();
    disable_all_cursors();
    print_base_settings_menu(1);
    enable_screen(BASE_SCREEN);
    *cursor_y_pos = 0;
    update_cursor_base_x(BASE_X_CURSOR_BASE_SETTINGS_MENU);
    cursor_update_base_settings_menu(*cursor_y_pos);
    prepare_flush();
}

void colours_settings_menu_init(mu8* cursor_y_pos, mu8* cursor_x_pos) {
    curr_state = COLOURS_SETTINGS_MENU;
    set_screen(BASE_SCREEN);
    disable_all_screens_but_current();
    disable_all_cursors();
    print_colour_settings_menu(1);
    enable_screen(BASE_SCREEN);
    *cursor_y_pos = 0;
    *cursor_x_pos = 0;
    cursor_update_colours_settings_menu(*cursor_y_pos, *cursor_x_pos);
    prepare_flush();
}

void prepare_crash_screen(enum CRASH_REASONS reason) {
    set_screen(BASE_SCREEN);
    default_reset_screen();
    enable_screen(BASE_SCREEN);
    reset_sprites_to_cursor(1);
    disable_all_screens_but_current();
    disable_all_cursors();
    set_screen(CRASH_WINDOW_SCREEN);
    print_crash(reason);
    enable_screen(CRASH_WINDOW_SCREEN);
    prepare_flush();
}

void crash_on_cartridge_removed() {
    prepare_crash_screen(CARTRIDGE_REMOVED);
    base_stop_transfer(0);
    base_stop_transfer(1);
    int curr_vcount = REG_VCOUNT + 1 + 1;
    if(curr_vcount >= SCANLINES)
        curr_vcount -= SCANLINES;
    while(REG_VCOUNT != curr_vcount);
    while(REG_VCOUNT != VBLANK_SCANLINES);
    disable_sprites_rendering();
    flush_screens();
    disable_all_irqs();
    while(1)
        Halt();
}

int main(void)
{
    #ifdef __GBA__
    RegisterRamReset(RESET_SIO|RESET_SOUND|RESET_OTHER);
    disable_all_irqs();
    #endif
    curr_state = MAIN_MENU;
    counter = 0;
    input_counter = 0;
    find_optimal_ewram_settings();
    set_default_settings();
    REG_WAITCNT = BASE_WAITCNT_VAL;
    init_bank();
    init_text_system();
    init_rng(0,0);
    u32 keys;
    
    init_sprites();
    init_oam_palette();
    init_sprite_counter();
    enable_sprites_rendering();
    init_numbers();

    #ifdef __GBA__
    irqInit();
    #endif
    irqSet(IRQ_VBLANK, vblank_update_function);
    irqEnable(IRQ_VBLANK);

    init_cursor();
    
    int result = 0;
    mu8 is_normal = 0;
    mu8 returned_val;
    mu8 update = 0;
    mu8 own_menu = 0;
    mu8 cursor_y_pos = 0;
    mu8 cursor_x_pos = 0;
    mu8 submenu_cursor_y_pos = 0;
    mu8 submenu_cursor_x_pos = 0;
    mu8 prev_val = 0;
    mu8 success = 0;
    uintptr_t address = ROM;
    uintptr_t tmp_address = address;

    main_menu_init(&cursor_y_pos, address);
    //PRINT_FUNCTION("\n\n0x\x0D: 0x\x0D\n", REG_MEMORY_CONTROLLER_ADDR, 8, REG_MEMORY_CONTROLLER, 8);
    scanKeys();
    keys = keysDown();
    
    while(1) {
        
        do {
            prepare_flush();
            VBlankIntrWait();
            scanKeys();
		    switch(curr_state) {
		        case MAIN_MENU:
                	print_main_menu(update, address);
			        break;
		        default:
			        break;
            }
            keys = keysDown();
        } while ((!(keys & KEY_LEFT)) && (!(keys & KEY_RIGHT)) && (!(keys & KEY_A)) && (!(keys & KEY_B)) && (!(keys & KEY_UP)) && (!(keys & KEY_DOWN)) && (!(keys & KEY_START)) && (!(keys & KEY_SELECT)));
        
        input_counter++;
        switch(curr_state) {
            case MAIN_MENU:
                returned_val = handle_input_main_menu(&cursor_y_pos, keys, &update, &address);
                print_main_menu(update, address);
                cursor_update_main_menu(cursor_y_pos);
                if(returned_val == START_MULTIBOOT) {
                    curr_state = MULTIBOOT_SETTINGS_MENU;
                    is_normal = 1;
                    disable_cursor();
                    print_multiboot_settings(is_normal, 1);
                }
                if(returned_val == SET_ADDRESS) {
                	address_setting_init(&cursor_x_pos, address);
                	tmp_address = address;
                }
                if(returned_val == START_SETTINGS_MENU)
                    base_settings_menu_init(&cursor_y_pos);
                break;
            case MULTIBOOT_SETTINGS_MENU:
                returned_val = handle_input_multiboot_settings(keys, &is_normal, &update);
                if(returned_val == CONFIRM_MULTIBOOT) {
                    curr_state = MULTIBOOT;
                    #ifdef HAS_SIO
                    sio_stop_irq_slave();
                    irqDisable(IRQ_SERIAL);
                    #endif
                    print_multiboot(multiboot_normal((u16*)EWRAM, (u16*)(EWRAM + MULTIBOOT_MAX_SIZE), is_normal));
                }
                else if(returned_val == CANCEL_MULTIBOOT)
                    main_menu_init(&cursor_y_pos, address);
                else
                    print_multiboot_settings(is_normal, update);
                break;
            case MULTIBOOT:
                if(handle_input_multiboot_menu(keys)) {
                	return 0;
                	main_menu_init(&cursor_y_pos, address);
                }
                break;
            case WAITING_DATA:
                break;
            case BASE_SETTINGS_MENU:
                returned_val = handle_input_base_settings_menu(keys, &cursor_y_pos, &update);
                if(returned_val) {
                    if(returned_val == ENTER_COLOUR_MENU)
                        colours_settings_menu_init(&cursor_y_pos, &cursor_x_pos);
                    else if(returned_val == EXIT_BASE_SETTINGS) {
                        main_menu_init(&cursor_y_pos, address);
                    }
                }
                else {
                    print_base_settings_menu(update);
                    cursor_update_base_settings_menu(cursor_y_pos);
                }
                break;
            case COLOURS_SETTINGS_MENU:
                returned_val = handle_input_colours_menu(keys, &cursor_y_pos, &cursor_x_pos, &update);
                if(returned_val)
                    base_settings_menu_init(&cursor_y_pos);
                else {
                    if(update) {
                        VBlankIntrWait();
                        set_text_palettes();
                        set_cursor_palette();
                    }
                    print_colour_settings_menu(update);
                    cursor_update_colours_settings_menu(cursor_y_pos, cursor_x_pos);
                }
                break;
            case ADDRESS_SETTING_MENU:
                returned_val = handle_input_address_settings(&cursor_x_pos, keys, &update, &tmp_address);
                print_address_setting(update, cursor_x_pos, tmp_address);
                if(returned_val == CONFIRM_ADDRESS_CHANGE)
                	address = tmp_address;
            	if(returned_val)
	                main_menu_init(&cursor_y_pos, address);
                break;
            default:
                main_menu_init(&cursor_y_pos, address);
                break;
        }
        update = 0;
    }

    return 0;
}
