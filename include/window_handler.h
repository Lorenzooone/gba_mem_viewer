#ifndef WINDOW_HANDLER__
#define WINDOW_HANDLER__

#include "config_settings.h"

#define TOTAL_Y_SIZE (SCREEN_HEIGHT>>3)
#define TOTAL_X_SIZE (SCREEN_WIDTH>>3)

#define JAPANESE_Y_AUTO_INCREASE (IS_SYS_LANGUAGE_JAPANESE ? 1 : 0)

#define ADDRESS_WINDOW_SCREEN 3
#define ADDRESS_WINDOW_X_SIZE 10
#define ADDRESS_WINDOW_Y_SIZE 3
#define ADDRESS_WINDOW_X ((TOTAL_X_SIZE>>1)-((ADDRESS_WINDOW_X_SIZE+1)>>1))
#define ADDRESS_WINDOW_Y ((TOTAL_Y_SIZE>>1)-((ADDRESS_WINDOW_Y_SIZE+1)>>1))

void init_message_window(void);
void clear_message_window(void);

void init_waiting_window(s8);
void clear_waiting_window(s8);

void init_crash_window(void);
void clear_crash_window(void);

void init_colour_window(void);
void clear_colour_window(void);

void init_loading_window(void);
void clear_loading_window(void);

void init_address_window(void);
void clear_address_window(void);

#endif
