#include "base_include.h"
#include "multiboot_handler.h"
#include "graphics_handler.h"
#include "input_handler.h"
#include "menu_text_handler.h"
#include "print_system.h"
#include "window_handler.h"
#include "sio.h"
#include "delays.h"
#include <stddef.h>
#include "optimized_swi.h"
#include "timing_basic.h"
#include "config_settings.h"
#include "save.h"

//#include "ewram_speed_check_bin.h"

#define REG_MEMORY_CONTROLLER_ADDR 0x4000800
#define HW_SET_REG_MEMORY_CONTROLLER_VALUE 0x0D000020
#define REG_MEMORY_CONTROLLER *((u32*)(REG_MEMORY_CONTROLLER_ADDR))

#define WORST_CASE_EWRAM 1
#define TEST_LAG_EWRAM 0
#define MIN_WAITCYCLE 1

#define BASE_SCREEN 2

//#define DO_TIMER_TESTS
#define NUM_TIMER_TESTS ((SCREEN_HEIGHT >> 3) - 1)

extern uint32_t _start;
extern uint32_t _prog_end;

void vblank_update_function(void);
void find_optimal_ewram_settings(void);
void disable_all_irqs(void);
void delay_no_read_test(u16*);
void delay_read_test(u16*);
void timer_test(void);
void address_setting_init(mu8*, uintptr_t);
void main_menu_init(uintptr_t);
int main(void);

enum STATE {MAIN_MENU, MULTIBOOT, MULTIBOOT_SETTINGS_MENU, ADDRESS_SETTING_MENU};
// curr_state should be "enum STATE", but that creates issues if building for VRAM...
mu8 curr_state;
u32 counter = 0;
u32 input_counter = 0;

IWRAM_CODE void vblank_update_function() {
    REG_IF |= IRQ_VBLANK;

    flush_screens();

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

void address_setting_init(mu8* cursor_x_pos, uintptr_t address) {
    curr_state = ADDRESS_SETTING_MENU;
    set_screen(ADDRESS_WINDOW_SCREEN);
    *cursor_x_pos = 0;
    print_address_setting(1, *cursor_x_pos, address);
    enable_screen(ADDRESS_WINDOW_SCREEN);
    prepare_flush();
}

void main_menu_init(uintptr_t address) {
    curr_state = MAIN_MENU;
    set_screen(BASE_SCREEN);
    set_bg_pos(BASE_SCREEN, 0, 0);
    print_main_menu(1, address);
    disable_all_screens_but_current();
    enable_screen(BASE_SCREEN);
    prepare_flush();
}

void delay_no_read_test(u16* results) {
    for(int i = 0; i < NUM_TIMER_TESTS; i++) {
        REG_TM3CNT_L = 0;
        REG_TM3CNT_H = 0x83;
        delay_cycles(CLOCK_CYCLES_PER_MS(500));
        REG_TM3CNT_H = 0x03;
        results[i] = REG_TM3CNT_L;
    }
}

void delay_read_test(u16* results) {
    for(int i = 0; i < NUM_TIMER_TESTS; i++) {
        REG_TM3CNT_L = 0;
        REG_TM3CNT_H = 0x83;
        delay_cycles_until(CLOCK_CYCLES_PER_MS(500), (vu8*)ROM, 0xFD, 4);
        REG_TM3CNT_H = 0x03;
        results[i] = REG_TM3CNT_L;
    }
}

void timer_test() {
    default_reset_screen();
    u32 keys = 0;
    u16 results[2][NUM_TIMER_TESTS];
    PRINT_FUNCTION("PRESS ANY KEY TO TEST");
    prepare_flush();
    while(!keys) {
        VBlankIntrWait();
        scanKeys();
        keys = keysHeld();
    }
    default_reset_screen();
    PRINT_FUNCTION("TEST UNDER TEST...");
    prepare_flush();

    delay_read_test(&results[1][0]);
    delay_no_read_test(&results[0][0]);

    default_reset_screen();
    PRINT_FUNCTION("RESULTS:\n");
    for(int i = 0; i < NUM_TIMER_TESTS; i++) {
        PRINT_FUNCTION("\x0D - \x0D\n", results[0][i], 4, results[1][i], 4);
    }
    prepare_flush();
    keys = 0;
    while(!keys) {
        VBlankIntrWait();
        scanKeys();
        keys = keysHeld();
    }
    
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
    u32 keys;
    struct multiboot_fixed_data mb_data;

    init_numbers();

    #ifdef __GBA__
    irqInit();
    #endif
    irqSet(IRQ_VBLANK, vblank_update_function);
    irqEnable(IRQ_VBLANK);

    #ifdef EXEC_BASE
    u16* mb_start = (u16*)EWRAM;
    u16* mb_end = (u16*)(EWRAM + MULTIBOOT_MAX_SIZE);
    #else
    u16* mb_start = (u16*)&_start;
    u16* mb_end = (u16*)&_prog_end;
    #endif

    mu8 is_normal = 0;
    mu8 returned_val;
    mu8 update = 0;
    mu8 cursor_x_pos = 0;
    uintptr_t address = ROM;
    uintptr_t tmp_address = address;

    main_menu_init(address);
    //PRINT_FUNCTION("\n\n0x\x0D: 0x\x0D\n", REG_MEMORY_CONTROLLER_ADDR, 8, REG_MEMORY_CONTROLLER, 8);
    scanKeys();
    keys = keysDown();
    #ifdef DO_TIMER_TESTS
    while(1) {
        timer_test();
    }
    return 0;
    #endif
    
    while(1) {
        
        do {
            prepare_flush();
            VBlankIntrWait();
            scanKeys();
            keys = keysDown();
            switch(curr_state) {
                case MAIN_MENU:
                    print_main_menu(update, address);
                    break;
                default:
                    break;
            }
        } while ((!(keys & KEY_LEFT)) && (!(keys & KEY_RIGHT)) && (!(keys & KEY_A)) && (!(keys & KEY_B)) && (!(keys & KEY_UP)) && (!(keys & KEY_DOWN)) && (!(keys & KEY_START)) && (!(keys & KEY_SELECT)));
        
        input_counter++;
        switch(curr_state) {
            case MAIN_MENU:
                returned_val = handle_input_main_menu(keys, &update, &address);
                print_main_menu(update, address);
                if(returned_val == START_MULTIBOOT) {
                    curr_state = MULTIBOOT_SETTINGS_MENU;
                    is_normal = 1;
                    print_multiboot_settings(is_normal, 1);
                }
                if(returned_val == SET_ADDRESS) {
                    address_setting_init(&cursor_x_pos, address);
                    tmp_address = address;
                }
                break;
            case MULTIBOOT_SETTINGS_MENU:
                returned_val = handle_input_multiboot_settings(keys, &is_normal, &update);
                if(returned_val == CONFIRM_MULTIBOOT) {
                    curr_state = MULTIBOOT;
                    #ifdef HAS_SIO
                    sio_stop_irq_slave();
                    irqDisable(IRQ_SERIAL);
                    #endif
                    print_multiboot(multiboot_normal(mb_start, mb_end, &mb_data, is_normal));
                }
                else if(returned_val == CANCEL_MULTIBOOT)
                    main_menu_init(address);
                else
                    print_multiboot_settings(is_normal, update);
                break;
            case MULTIBOOT:
                if(handle_input_multiboot_menu(keys)) {
                    main_menu_init(address);
                }
                break;
            case ADDRESS_SETTING_MENU:
                returned_val = handle_input_address_settings(&cursor_x_pos, keys, &update, &tmp_address);
                print_address_setting(update, cursor_x_pos, tmp_address);
                if(returned_val == CONFIRM_ADDRESS_CHANGE)
                    address = tmp_address;
                if(returned_val)
                    main_menu_init(address);
                break;
            default:
                main_menu_init(address);
                break;
        }
        update = 0;
    }

    return 0;
}
