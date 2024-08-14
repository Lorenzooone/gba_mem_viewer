#include "base_include.h"
#include "print_system.h"
#include "window_handler.h"

#define HORIZONTAL_WINDOW_TILE 2
#define VERTICAL_WINDOW_TILE 3
#define ANGLE_WINDOW_TILE 4

void create_window(u8, u8, u8, u8, u8);
void reset_window(u8, u8, u8, u8, u8);

void init_address_window() {
    create_window(ADDRESS_WINDOW_X, ADDRESS_WINDOW_Y, ADDRESS_WINDOW_X_SIZE, ADDRESS_WINDOW_Y_SIZE, ADDRESS_WINDOW_SCREEN);
}

void clear_address_window() {
    reset_window(ADDRESS_WINDOW_X, ADDRESS_WINDOW_Y, ADDRESS_WINDOW_X_SIZE, ADDRESS_WINDOW_Y_SIZE, ADDRESS_WINDOW_SCREEN);
}

void create_window(u8 x, u8 y, u8 x_size, u8 y_size, u8 screen_num) {
    set_updated_screen();
    screen_t* screen = get_screen(screen_num);
    #if defined (__NDS__) && (SAME_ON_BOTH_SCREENS)
    screen_t* screen_sub = get_screen_sub(screen_num);
    #endif
    u8 fill_x = x_size == TOTAL_X_SIZE;
    u8 fill_y = y_size == TOTAL_Y_SIZE;
    
    if(2+x+x_size > X_SIZE)
        x_size = X_SIZE - (x + 1);
    if(2+y+y_size > Y_SIZE)
        y_size = Y_SIZE - (y + 1);
    
    if((!x_size) || (!y_size))
        return;
    
    u8 start_x = x-1;
    u8 start_y = y-1;
    if(!x)
        start_x = X_SIZE-1;
    if(!y)
        start_y = Y_SIZE-1;
    
    u8 end_x = x + x_size;
    u8 end_y = y + y_size;
    if(end_x >= X_SIZE)
        end_x -= X_SIZE;
    if(end_y >= Y_SIZE)
        end_y -= Y_SIZE;
    
    if(!fill_y) {
        for(int i = 0; i < x_size; i++) {
            screen[x+(start_y*X_SIZE)+i] = (PALETTE<<12) | HORIZONTAL_WINDOW_TILE;
            screen[x+(end_y*X_SIZE)+i] = (PALETTE<<12) | HORIZONTAL_WINDOW_TILE | VSWAP_TILE;
            #if defined (__NDS__) && (SAME_ON_BOTH_SCREENS)
            screen_sub[x+(start_y*X_SIZE)+i] = (PALETTE<<12) | HORIZONTAL_WINDOW_TILE;
            screen_sub[x+(end_y*X_SIZE)+i] = (PALETTE<<12) | HORIZONTAL_WINDOW_TILE | VSWAP_TILE;
            #endif
        }
        if(fill_x) {
            screen[x+(start_y*X_SIZE)+x_size] = (PALETTE<<12) | HORIZONTAL_WINDOW_TILE;
            screen[x+(end_y*X_SIZE)+x_size] = (PALETTE<<12) | HORIZONTAL_WINDOW_TILE | VSWAP_TILE;
            #if defined (__NDS__) && (SAME_ON_BOTH_SCREENS)
            screen_sub[x+(start_y*X_SIZE)+x_size] = (PALETTE<<12) | HORIZONTAL_WINDOW_TILE;
            screen_sub[x+(end_y*X_SIZE)+x_size] = (PALETTE<<12) | HORIZONTAL_WINDOW_TILE | VSWAP_TILE;
            #endif
        }
    }
    
    if(!fill_x) {
        for(int i = 0; i < y_size; i++) {
            screen[start_x+((y+i)*X_SIZE)] = (PALETTE<<12) | VERTICAL_WINDOW_TILE;
            screen[end_x+((y+i)*X_SIZE)] = (PALETTE<<12) | VERTICAL_WINDOW_TILE | HSWAP_TILE;
            #if defined (__NDS__) && (SAME_ON_BOTH_SCREENS)
            screen_sub[start_x+((y+i)*X_SIZE)] = (PALETTE<<12) | VERTICAL_WINDOW_TILE;
            screen_sub[end_x+((y+i)*X_SIZE)] = (PALETTE<<12) | VERTICAL_WINDOW_TILE | HSWAP_TILE;
            #endif
        }
        if(fill_y) {
            screen[start_x+((y+y_size)*X_SIZE)] = (PALETTE<<12) | VERTICAL_WINDOW_TILE;
            screen[end_x+((y+y_size)*X_SIZE)] = (PALETTE<<12) | VERTICAL_WINDOW_TILE | HSWAP_TILE;
            #if defined (__NDS__) && (SAME_ON_BOTH_SCREENS)
            screen_sub[start_x+((y+y_size)*X_SIZE)] = (PALETTE<<12) | VERTICAL_WINDOW_TILE;
            screen_sub[end_x+((y+y_size)*X_SIZE)] = (PALETTE<<12) | VERTICAL_WINDOW_TILE | HSWAP_TILE;
            #endif
        }
    }
    
    if((!fill_x) && (!fill_y)) {
        screen[start_x+(start_y*X_SIZE)] = (PALETTE<<12) | ANGLE_WINDOW_TILE;
        screen[end_x+(start_y*X_SIZE)] = (PALETTE<<12) | ANGLE_WINDOW_TILE | HSWAP_TILE;
        screen[start_x+(end_y*X_SIZE)] = (PALETTE<<12) | ANGLE_WINDOW_TILE | VSWAP_TILE;
        screen[end_x+(end_y*X_SIZE)] = (PALETTE<<12) | ANGLE_WINDOW_TILE | HSWAP_TILE | VSWAP_TILE;
        #if defined (__NDS__) && (SAME_ON_BOTH_SCREENS)
        screen_sub[start_x+(start_y*X_SIZE)] = (PALETTE<<12) | ANGLE_WINDOW_TILE;
        screen_sub[end_x+(start_y*X_SIZE)] = (PALETTE<<12) | ANGLE_WINDOW_TILE | HSWAP_TILE;
        screen_sub[start_x+(end_y*X_SIZE)] = (PALETTE<<12) | ANGLE_WINDOW_TILE | VSWAP_TILE;
        screen_sub[end_x+(end_y*X_SIZE)] = (PALETTE<<12) | ANGLE_WINDOW_TILE | HSWAP_TILE | VSWAP_TILE;
        #endif
    }
}

void reset_window(u8 x, u8 y, u8 x_size, u8 y_size, u8 screen_num){
    set_updated_screen();
    screen_t* screen = get_screen(screen_num);
    #if defined (__NDS__) && (SAME_ON_BOTH_SCREENS)
    screen_t* screen_sub = get_screen_sub(screen_num);
    #endif

    for(int i = 0; i < y_size; i++) {
        u8 inside_y = y + i;
        if(inside_y >= Y_SIZE)
            inside_y -= Y_SIZE;
        for(int j = 0; j < x_size; j++) {
            u8 inside_x = x + j;
            if(inside_x >= X_SIZE)
                inside_x -= X_SIZE;
            screen[inside_x+(inside_y*X_SIZE)] = (PALETTE<<12) | 0;
            #if defined (__NDS__) && (SAME_ON_BOTH_SCREENS)
            screen_sub[inside_x+(inside_y*X_SIZE)] = (PALETTE<<12) | 0;
            #endif
        }
    }
}
