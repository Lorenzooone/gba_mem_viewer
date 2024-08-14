#include "base_include.h"
#include <stddef.h>
#include "useful_qualifiers.h"
#include "timing_basic.h"
#include "delays.h"

#ifdef __GBA__
#ifdef EXEC_EWRAM
#define NUM_CYCLES_PER_ITER 12
#define NUM_CYCLES_PER_ITER_CHECK 22
#define NUM_CYCLES_MULTIPLIER 1
#else
#ifdef EXEC_OVRAM
#define NUM_CYCLES_PER_ITER 4
#define NUM_CYCLES_PER_ITER_CHECK 8
#define NUM_CYCLES_MULTIPLIER 1
#else
#ifdef EXEC_VRAM
#define NUM_CYCLES_PER_ITER 21
#define NUM_CYCLES_PER_ITER_CHECK 42
#define NUM_CYCLES_MULTIPLIER 5
#else
#define NUM_CYCLES_PER_ITER 4
#define NUM_CYCLES_PER_ITER_CHECK 8
#define NUM_CYCLES_MULTIPLIER 1
#endif
#endif
#endif
#else
#define NUM_CYCLES_PER_ITER 4
#define NUM_CYCLES_PER_ITER_CHECK 14
#define NUM_CYCLES_MULTIPLIER 1
#endif

#define DIVIDE_ROUND_OPERATION(x, y) (((x) + ((y) - 1) + ((y) / 2)) / (y))

NO_INLINE IWRAM_CODE void delay_cycles(u32 num_cycles) {
    num_cycles = DIVIDE_ROUND_OPERATION(num_cycles * NUM_CYCLES_MULTIPLIER, NUM_CYCLES_PER_ITER);
    if(!num_cycles)
        return;
    __asm__ volatile (
        "L1%=: subs %[wait], #1;"
        "bne L1%=;"
        : [wait] "+l" (num_cycles)
    );
}

NO_INLINE IWRAM_CODE void delay_cycles_until(u32 num_cycles, vu8* data, u8 wanted, u8 cycles_per_load) {
    num_cycles = DIVIDE_ROUND_OPERATION(num_cycles * NUM_CYCLES_MULTIPLIER, NUM_CYCLES_PER_ITER_CHECK + (cycles_per_load * NUM_CYCLES_MULTIPLIER));
    if(!num_cycles)
        return;
    __asm__ volatile (
        "L1%=: ldrb r3, [%[data], #0];"
        "cmp r3, %[wanted];"
        "beq L2%=;"
        "subs %[wait], #1;"
        "bne L1%=;"
        "L2%=:;"
        : [wait] "+l" (num_cycles) : [data]"l"(data), [wanted]"l"(wanted) : "r3"
    );
}
