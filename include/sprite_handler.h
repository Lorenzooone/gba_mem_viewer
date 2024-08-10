#ifndef SPRITE_HANDLER__
#define SPRITE_HANDLER__

#include "print_system.h"
#include "config_settings.h"
#include "graphics_handler.h"
#include "window_handler.h"

#define TOP_SCREEN_SPRITE_POS SCREEN_REAL_HEIGHT
#define LEFT_SCREEN_SPRITE_POS SCREEN_REAL_WIDTH

#define BASE_Y_SPRITE_INFO_PAGE 0
#define BASE_X_SPRITE_INFO_PAGE 0

#define JAPANESE_Y_AUTO_INCREASE_SPRITES (JAPANESE_Y_AUTO_INCREASE<<3)

#define CURSOR_Y_POS_CANCEL (SCREEN_HEIGHT-8)
#define CURSOR_X_POS_CANCEL 2

#define BASE_X_CURSOR_MAIN_MENU 2
#define BASE_Y_CURSOR_MAIN_MENU 8
#define BASE_Y_CURSOR_INCREMENT_MAIN_MENU 16
#define BOTTOM_Y_CURSOR_MAIN_MENU_VALUE ((((Y_LIMIT-1)<<3)-BASE_Y_CURSOR_MAIN_MENU)/BASE_Y_CURSOR_INCREMENT_MAIN_MENU)

#define BASE_X_CURSOR_BASE_SETTINGS_MENU 2
#define BASE_Y_CURSOR_BASE_SETTINGS_MENU 8
#define BASE_Y_CURSOR_INCREMENT_BASE_SETTINGS_MENU 16
#define BOTTOM_Y_CURSOR_BASE_SETTINGS_MENU_VALUE ((((Y_LIMIT-3)<<3)-BASE_Y_CURSOR_BASE_SETTINGS_MENU)/BASE_Y_CURSOR_INCREMENT_BASE_SETTINGS_MENU)

#define BASE_X_CURSOR_COLOURS_SETTINGS_MENU 2
#define BASE_X_CURSOR_COLOURS_SETTINGS_MENU_IN (2+112)
#define BASE_X_CURSOR_INCREMENT_COLOURS_SETTINGS_MENU_IN 40
#define BASE_Y_CURSOR_COLOURS_SETTINGS_MENU 24
#define BASE_Y_CURSOR_INCREMENT_COLOURS_SETTINGS_MENU 16

void init_sprites(void);
void enable_sprites_rendering(void);
void disable_sprites_rendering(void);
void init_sprite_counter(void);
void init_oam_palette(void);
void init_cursor(void);
void update_cursor_y(u16);
void update_cursor_base_x(u16);
void raw_update_sprite_y(u8, u8);
void move_sprites(u8 counter);
void move_cursor_x(u8 counter);
void disable_cursor(void);
void disable_all_cursors(void);
u8 get_next_sprite_index(void);
void reset_sprites(u8);
void disable_all_sprites(void);
void enable_all_sprites(void);
void update_normal_oam(void);
void reset_sprites_to_cursor(u8);
void fade_all_sprites_to_white(u16);
void remove_fade_all_sprites(void);
void set_cursor_palette(void);

#endif
