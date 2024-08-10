#include "base_include.h"
#include "graphics_handler.h"
#include "useful_qualifiers.h"
#include "print_system.h"
#include <stddef.h>

#define CPUFASTSET_FILL (0x1000000)

void convert_xbpp(u8* src, u32* dst, size_t src_size, u8* colors, u8 is_forward, u8 num_bpp) {
    if(num_bpp == 0 || num_bpp > 4)
        return;
    size_t num_rows = Div(src_size, num_bpp);
    if(DivMod(src_size, num_bpp))
        num_rows += 1;
    for(size_t i = 0; i < num_rows; i++) {
        u32 src_data = 0;
        for(int j = 0; j < num_bpp; j++)
            src_data |= src[(i*num_bpp)+j]<<(8*j);
        u32 row = 0;
        for(int j = 0; j < 8; j++)
            if(!is_forward)
                row |= (colors[(src_data>>(num_bpp*j))&((1<<num_bpp)-1)]&0xF) << (4*j);
            else
                row |= (colors[(src_data>>(num_bpp*j))&((1<<num_bpp)-1)]&0xF) << (4*(7-j));
        dst[i] = row;
    }
}

void convert_1bpp(u8* src, u32* dst, size_t src_size, u8* colors, u8 is_forward) {
    convert_xbpp(src, dst, src_size, colors, is_forward, 1);
}

void convert_2bpp(u8* src, u32* dst, size_t src_size, u8* colors, u8 is_forward) {
    convert_xbpp(src, dst, src_size, colors, is_forward, 2);
}

void convert_3bpp(u8* src, u32* dst, size_t src_size, u8* colors, u8 is_forward) {
    convert_xbpp(src, dst, src_size, colors, is_forward, 3);
}
