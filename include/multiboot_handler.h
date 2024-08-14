#ifndef MULTIBOOT_HANDLER__
#define MULTIBOOT_HANDLER__

#include "base_include.h"
#include <stddef.h>

#define MULTIBOOT_MAX_SIZE 0x3FF40

struct multiboot_fixed_data {
	u16* data;
	size_t size;
	u32 crcC_normal;
	u32 crcC_multi;
	mu8 crcC_normal_init;
	mu8 crcC_multi_init;
	mu8 init;
};

enum MULTIBOOT_RESULTS {MB_SUCCESS, MB_NO_INIT_SYNC, MB_WRONG_ANSWER, MB_HEADER_ISSUE, MB_PALETTE_FAILURE, MB_SWI_FAILURE, MB_NOT_INIT, MB_TOO_BIG, MB_FINAL_HANDSHAKE_FAILURE, MB_CRC_FAILURE, MB_SEND_FAILURE};

enum MULTIBOOT_RESULTS multiboot_normal (u16*, u16*, struct multiboot_fixed_data*, int);

#endif
