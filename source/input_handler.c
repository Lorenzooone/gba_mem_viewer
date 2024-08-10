#include "base_include.h"
#include "input_handler.h"
#include "sprite_handler.h"
#include "useful_qualifiers.h"
#include "config_settings.h"

u8 handle_input_multiboot_settings(u16 keys, mu8* is_normal, mu8* update) {
    if(keys & KEY_A)
        return CONFIRM_MULTIBOOT;
    if(keys & KEY_B)
        return CANCEL_MULTIBOOT;

    if(*is_normal && (keys & KEY_RIGHT)) {
        *is_normal = 0;
        *update = 1;
    }
    else if((!(*is_normal)) && (keys & KEY_LEFT)) {
        *is_normal = 1;
        *update = 1;
    }

    return 0;
}

u8 handle_input_multiboot_menu(u16 keys) {
    if(keys & KEY_A)
        return 1;
    return 0;
}

static uintptr_t sanitize_address(uintptr_t address, int change) {
	return (address + 0x10000000 + change) & 0x0FFFFFFF;
}

u8 handle_input_address_settings(mu8* cursor_x_pos, u16 keys, mu8* update, uintptr_t* address) {
	if(keys & KEY_A)
		return CONFIRM_ADDRESS_CHANGE;
	if(keys & KEY_B)
		return NO_ADDRESS_CHANGE;
	if(keys & KEY_LEFT) {
		if((*cursor_x_pos) == 0)
			*cursor_x_pos = 5;
		else
			*cursor_x_pos -= 1;
		*update = 1;
		return 0;
	}
	if(keys & KEY_RIGHT) {
		if((*cursor_x_pos) == 5)
			*cursor_x_pos = 0;
		else
			*cursor_x_pos += 1;
		*update = 1;
		return 0;
	}

	int value_to_add = 0x10 << (4 * (5 - (*cursor_x_pos)));
	int change = 0;
	if(keys & KEY_UP)
		change = value_to_add;
	if(keys & KEY_DOWN)
		change = -value_to_add;
	if(change) {
    	*address = sanitize_address(*address, change);
    	*update = 1;
	}

	return 0;
}

u8 handle_input_main_menu(mu8* cursor_y_pos, u16 keys, mu8* update, uintptr_t* address) {
    int change = 0;
    if((keys & KEY_B) || (keys & KEY_SELECT))
    	return START_MULTIBOOT;
    if((keys & KEY_A) || (keys & KEY_START))
    	return SET_ADDRESS;
    if(keys & KEY_UP)
    	change -= 0x10;
    if(keys & KEY_DOWN)
    	change += 0x10;
    if(keys & KEY_LEFT)
    	change -= 0x80;
    if(keys & KEY_RIGHT)
    	change += 0x80;
    if(change) {
    	*address = sanitize_address(*address, change);
    	*update = 1;
	}
    return 0;
}

u8 handle_input_base_settings_menu(u16 keys, mu8* cursor_y_pos, mu8* update) {
    if(keys & KEY_B)
        return EXIT_BASE_SETTINGS;

    if(keys & KEY_A)
        return ENTER_COLOUR_MENU;

    return 0;
}

u8 handle_input_colours_menu(u16 keys, mu8* cursor_y_pos, mu8* cursor_x_pos, mu8* update) {
    
    if((keys & KEY_B) && (!(*cursor_x_pos)))
        return EXIT_COLOURS_SETTINGS;

    if(keys & KEY_B)
        *cursor_x_pos = 0;
    else if(keys & KEY_A) {
        if(!(*cursor_x_pos))
            *cursor_x_pos += 1;
        else {
            set_single_colour(*cursor_y_pos, (*cursor_x_pos)-1, get_single_colour(*cursor_y_pos, (*cursor_x_pos)-1)+1);
            *update = 1;
        }
    }
    else if((keys & KEY_LEFT) && (*cursor_x_pos))
        *cursor_x_pos -= 1;
    else if((keys & KEY_RIGHT) && ((*cursor_x_pos) < NUM_SUB_COLOURS))
        *cursor_x_pos += 1;
    else if((keys & KEY_UP) && (*cursor_x_pos)) {
        set_single_colour(*cursor_y_pos, (*cursor_x_pos)-1, get_single_colour(*cursor_y_pos, (*cursor_x_pos)-1) + 1);
        *update = 1;
    }
    else if((keys & KEY_DOWN) && (*cursor_x_pos)) {
        set_single_colour(*cursor_y_pos, (*cursor_x_pos)-1, get_single_colour(*cursor_y_pos, (*cursor_x_pos)-1) - 1);
        *update = 1;
    }
    else if(keys & KEY_UP) {
        if(*cursor_y_pos)
            *cursor_y_pos -= 1;
        else
            *cursor_y_pos = NUM_COLOURS-1;
    }
    else if(keys & KEY_DOWN) {
        if((*cursor_y_pos) >= (NUM_COLOURS-1))
            *cursor_y_pos = 0;
        else
            *cursor_y_pos += 1;
    }
    
    return 0;
}
