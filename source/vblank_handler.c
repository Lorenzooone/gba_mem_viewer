#include "base_include.h"
#include "vblank_handler.h"

#ifndef ABSOLUTE_MINIMAL_START
#define NORMAL_VBLANK_WAIT
#endif

static IntFn vblank_function = NULL;

void vblank_wait_function(void) {
    #ifdef NORMAL_VBLANK_WAIT
    VBlankIntrWait();
    #else
    while(REG_VCOUNT == VBLANK_SCANLINES);
    while(REG_VCOUNT != VBLANK_SCANLINES);
    if(vblank_function != NULL)
        vblank_function();
    #endif
}

void register_vblank_function(IntFn function) {
    #ifdef NORMAL_VBLANK_WAIT
    irqSet(IRQ_VBLANK, function);
    #else
    vblank_function = function;
    #endif
}
