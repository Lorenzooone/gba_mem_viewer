#ifndef VBLANK_HANDLER__
#define VBLANK_HANDLER__

void vblank_wait_function(void);
void register_vblank_function(IntFn function);

#endif
