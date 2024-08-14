#include "base_include.h"
#include "multiboot_handler.h"
#include "sio.h"
#include "menu_text_handler.h"
#include "print_system.h"
#include "timing_basic.h"
#include "delays.h"
#include "useful_qualifiers.h"

#define MULTIBOOT_INIT_VALID_VALUE 0x91

#define MAX_FINAL_HANDSHAKE_ATTEMPTS (FPS * 5)
#define MAX_PALETTE_ATTEMPTS 128
#define MULTIBOOT_CHOSEN_PALETTE_VALUE 0x81

// In the end, it's not needed...?!
#define MULTIBOOT_WAIT_TIME_MUS (36 - 36)

#define CRCC_NORMAL_START 0xC387
#define CRCC_MULTI_START 0xFFF8

#define CRCC_NORMAL_XOR 0xC37B
#define CRCC_MULTI_XOR 0xA517

#define DATA_NORMAL_XOR 0x43202F2F
#define DATA_MULTI_XOR 0x6465646F

struct multiboot_dynamic_data {
    mu8 is_normal;
    u32 crcB;
    u32 seed;
    mu8* token_data;
    mu8 client_mask;
};

static void multiboot_send(u32, int, u16*);
static void calc_crc_data_u32(u32, u32*, mu8);
static mu8 received_data_same_as_value(mu8, u16, u16, u16*);
static mu8 received_data_same_as_value_client_bit(mu8, u16, u16*);
enum MULTIBOOT_RESULTS multiboot_main_transfer(struct multiboot_fixed_data*, struct multiboot_dynamic_data*);
static enum MULTIBOOT_RESULTS multiboot_init(u16*, u16*, struct multiboot_fixed_data*);

IWRAM_CODE static inline void calc_crc_data_u32(u32 read_data, u32* crcC_ptr, mu8 is_normal) {
    u32 tmp_crcC = *crcC_ptr;
    u32 xor_val = CRCC_NORMAL_XOR;
    if(!is_normal)
        xor_val = CRCC_MULTI_XOR;
    for(int j = 0; j < 32; j++) {
        mu8 bit = (tmp_crcC ^ read_data) & 1;
        read_data >>= 1;
        tmp_crcC >>= 1;
        if(bit)
            tmp_crcC ^= xor_val;
    }
    *crcC_ptr = tmp_crcC;
}

IWRAM_CODE static enum MULTIBOOT_RESULTS multiboot_init(u16* data, u16* end, struct multiboot_fixed_data* mb_data) {
    mb_data->init = 0;
    mb_data->data = data;
    mb_data->size = (((((uintptr_t)end) + 15) >> 4) << 4) - ((uintptr_t)data);
    if(mb_data->size > MULTIBOOT_MAX_SIZE)
        return MB_TOO_BIG;

    mb_data->crcC_normal_init = 0;
    mb_data->crcC_multi_init = 0;
    mb_data->init = MULTIBOOT_INIT_VALID_VALUE;
    return MB_SUCCESS;
}

static inline mu8 received_data_same_as_value(mu8 client_mask, u16 wanted_value, u16 mask, u16* response) {
    for(int i = 0; i < MAX_NUM_SLAVES; i++) {
        mu8 client_bit = 1 << (i + 1);

        if((client_mask & client_bit) && ((response[i] & mask) != wanted_value))
            return 0;
    }
    return 1;
}

static inline mu8 received_data_same_as_value_client_bit(mu8 client_mask, u16 wanted_value, u16* response) {
    for(int i = 0; i < MAX_NUM_SLAVES; i++) {
        mu8 client_bit = 1 << (i + 1);

        if((client_mask & client_bit) && (response[i] != (wanted_value | client_bit)))
            return 0;
    }
    return 1;
}

IWRAM_CODE static void multiboot_send(u32 data, int is_normal, u16* out_buffer) {
    // Wait at least 36 us between sends
    for(int i = 0; i < MAX_NUM_SLAVES; i++)
        out_buffer[i] = 0;
    if(is_normal)
        out_buffer[0] = timed_sio_normal_master(data, SIO_32, MULTIBOOT_WAIT_TIME_MUS) >> 0x10;
    else
        timed_sio_multi_master(data, MULTIBOOT_WAIT_TIME_MUS, out_buffer);
}

IWRAM_CODE enum MULTIBOOT_RESULTS multiboot_main_transfer(struct multiboot_fixed_data* mb_data, struct multiboot_dynamic_data* mb_dyn_data) {
    u16 recv_data[MAX_NUM_SLAVES];
    u32* u32_data = (u32*)mb_data->data;
    delay_cycles(CLOCK_CYCLES_PER_MS((1000+15)/16));

    multiboot_send((mb_data->size - 0x190) >> 2, mb_dyn_data->is_normal, recv_data);
    for(int i = 0; i < MAX_NUM_SLAVES; i++) {
        mu8 contribute = 0xFF;
        mu8 client_bit = 1 << (i + 1);

        if(mb_dyn_data->client_mask & client_bit)
            contribute = recv_data[i] & 0xFF;
        mb_dyn_data->crcB |= contribute << (8 * (i + 1));
    }

    if(mb_dyn_data->is_normal) {
        if(!mb_data->crcC_normal_init)
            mb_data->crcC_normal = CRCC_NORMAL_START;
        for(size_t i = 0xC0 >> 2; i < (mb_data->size >> 2); i++) {
            mb_dyn_data->seed = (mb_dyn_data->seed * 0x6F646573) + 1;
            multiboot_send(u32_data[i] ^ (0xFE000000 - (i << 2)) ^ mb_dyn_data->seed ^ DATA_NORMAL_XOR, mb_dyn_data->is_normal, recv_data);
            if(!received_data_same_as_value(mb_dyn_data->client_mask, i << 2, 0xFFFF, recv_data))
                return MB_SEND_FAILURE;
            if(!mb_data->crcC_normal_init)
                calc_crc_data_u32(u32_data[i], &mb_data->crcC_normal, mb_dyn_data->is_normal);
        }
        mb_data->crcC_normal_init = 1;
    }
    else {
        if(!mb_data->crcC_multi_init)
            mb_data->crcC_multi = CRCC_MULTI_START;
        for(size_t i = 0xC0 >> 2; i < (mb_data->size >> 2); i++) {
            mb_dyn_data->seed = (mb_dyn_data->seed * 0x6F646573) + 1;
            multiboot_send((u32_data[i] ^ (0xFE000000 - (i << 2)) ^ mb_dyn_data->seed ^ DATA_MULTI_XOR) & 0xFFFF, mb_dyn_data->is_normal, recv_data);
            if(!received_data_same_as_value(mb_dyn_data->client_mask, i << 2, 0xFFFF, recv_data))
                return MB_SEND_FAILURE;
            multiboot_send((u32_data[i] ^ (0xFE000000 - (i << 2)) ^ mb_dyn_data->seed ^ DATA_MULTI_XOR) >> 16, mb_dyn_data->is_normal, recv_data);
            if(!received_data_same_as_value(mb_dyn_data->client_mask, (i << 2) + 2, 0xFFFF, recv_data))
                return MB_SEND_FAILURE;
            if(!mb_data->crcC_multi_init)
                calc_crc_data_u32(u32_data[i], &mb_data->crcC_multi, mb_dyn_data->is_normal);
        }
        mb_data->crcC_multi_init = 1;
    }

    multiboot_send(0x0065, mb_dyn_data->is_normal, recv_data);

    return MB_SUCCESS;
}

#ifdef HAS_SIO
enum MULTIBOOT_RESULTS multiboot_normal (u16* data, u16* end, struct multiboot_fixed_data* mb_data, int is_normal) {
#else
enum MULTIBOOT_RESULTS multiboot_normal (u16* UNUSED(data), u16* UNUSED(end), struct multiboot_fixed_data* UNUSED(mb_data), int UNUSED(is_normal)) {
#endif
    #ifdef HAS_SIO
    u16 response[MAX_NUM_SLAVES];
    int attempts, sends, halves;
    mu8 answers[MAX_NUM_SLAVES] = {0xFF, 0xFF, 0xFF};
    mu8 handshake;
    mu8 sendMask;
    u32 attempt_counter;
    const mu8 palette = MULTIBOOT_CHOSEN_PALETTE_VALUE;
    struct multiboot_dynamic_data mb_dyn_data;
    mb_dyn_data.is_normal = is_normal;
    mb_dyn_data.client_mask = 0;

    if(mb_data->init != MULTIBOOT_INIT_VALID_VALUE)
        multiboot_init(data, end, mb_data);

    if(mb_dyn_data.is_normal)
        init_sio_normal(SIO_MASTER, SIO_32);
    else
        init_sio_multi(SIO_MASTER);

    print_multiboot_mid_process(0);
    prepare_flush();

    for(attempts = 0; attempts < 128; attempts++) {
        for(sends = 0; sends < 16; sends++) {
            multiboot_send(0x6200, mb_dyn_data.is_normal, response);

            for(int i = 0; i < MAX_NUM_SLAVES; i++)
                if((response[i] & 0xFFF0) == 0x7200) {
                    mb_dyn_data.client_mask |= response[i] & 0xF;
                }
        }

        if(mb_dyn_data.client_mask)
            break;
        else
            VBlankIntrWait();
    }

    if(!mb_dyn_data.client_mask) {
        return MB_NO_INIT_SYNC;
    }

    multiboot_send(0x6100 | mb_dyn_data.client_mask, mb_dyn_data.is_normal, response);
    if(!received_data_same_as_value_client_bit(mb_dyn_data.client_mask, 0x7200, response))
        return MB_WRONG_ANSWER;

    for(halves = 0; halves < 0x60; ++halves) {
        multiboot_send(mb_data->data[halves], mb_dyn_data.is_normal, response);
        if(!received_data_same_as_value_client_bit(mb_dyn_data.client_mask, (0x60 - halves) << 8, response))
            return MB_HEADER_ISSUE;
    }

    multiboot_send(0x6200, mb_dyn_data.is_normal, response);
    if(!received_data_same_as_value_client_bit(mb_dyn_data.client_mask, 0, response))
        return MB_WRONG_ANSWER;

    multiboot_send(0x6200 | mb_dyn_data.client_mask, mb_dyn_data.is_normal, response);
    if(!received_data_same_as_value_client_bit(mb_dyn_data.client_mask, 0x7200, response))
        return MB_WRONG_ANSWER;

    sendMask = mb_dyn_data.client_mask;
    attempt_counter = 0;

    while(sendMask) {
        multiboot_send(0x6300 | palette, mb_dyn_data.is_normal, response);

        for(int i = 0; i < MAX_NUM_SLAVES; i++) {
            mu8 client_bit = 1 << (i + 1);

            if ((mb_dyn_data.client_mask & client_bit) && ((response[i] & 0xFF00) == 0x7300)) {
                answers[i] = response[i] & 0xFF;
                sendMask &= ~client_bit;
            }
        }
        attempt_counter++;

        if((attempt_counter == MAX_PALETTE_ATTEMPTS) && sendMask)
            return MB_PALETTE_FAILURE;
    }

    mb_dyn_data.seed = palette;
    handshake = 0x11;
    for(int i = 0; i < MAX_NUM_SLAVES; i++) {
        handshake += answers[i];
        mb_dyn_data.seed |= answers[i] << (8 * (i + 1));
    }

    handshake &= 0xFF;
    multiboot_send(0x6400 | handshake, mb_dyn_data.is_normal, response);
    if(!received_data_same_as_value(mb_dyn_data.client_mask, 0x7300, 0xFF00, response))
        return MB_WRONG_ANSWER;
    mb_dyn_data.crcB = handshake;
    
    print_multiboot_mid_process(1);
    prepare_flush();
    VBlankIntrWait();

    #ifndef EXEC_U16_MU8
    enum MULTIBOOT_MODES mb_mode = MODE32_NORMAL;
    MultiBootParam mp;
    
    if(!mb_dyn_data.is_normal)
        mb_mode = MODE16_MULTI;
    
    mp.handshake_data = handshake;
    for(int i = 0; i < MAX_NUM_SLAVES; i++)
        mp.client_data[i] = answers[i];
    mp.palette_data = palette;
    mp.client_bit = mb_dyn_data.client_mask;
    mp.boot_srcp = &((u8*)data)[0xC0];
    mp.boot_endp = (u8*)end;
    
    if(MultiBoot(&mp, mb_mode))
        return MB_SWI_FAILURE;
    #else
    // This is slower before caching the fixed crcC, but it works even in vram/ovram...
    enum MULTIBOOT_RESULTS result = multiboot_main_transfer(mb_data, &mb_dyn_data);
    if(result != MB_SUCCESS)
        return result;

    u32 crcC_final = mb_data->crcC_normal & 0xFFFF;
    if(!mb_dyn_data.is_normal)
        crcC_final = mb_data->crcC_multi & 0xFFFF;
    calc_crc_data_u32(mb_dyn_data.crcB, &crcC_final, mb_dyn_data.is_normal);

    attempt_counter = 0;
    mu8 done = 0;

    while(!done) {
        multiboot_send(0x0065, mb_dyn_data.is_normal, response);
        done = 1;
        if(!received_data_same_as_value(mb_dyn_data.client_mask, 0x0075, 0xFFFF, response))
            done = 0;
        VBlankIntrWait();
        attempt_counter += 1;

        if(attempt_counter == MAX_FINAL_HANDSHAKE_ATTEMPTS)
            return MB_FINAL_HANDSHAKE_FAILURE;
    }
    multiboot_send(0x0066, mb_dyn_data.is_normal, response);
    multiboot_send(crcC_final & 0xFFFF, mb_dyn_data.is_normal, response);

    if(!received_data_same_as_value(mb_dyn_data.client_mask, crcC_final, 0xFFFF, response))
        return MB_CRC_FAILURE;
    #endif

    #endif
    return MB_SUCCESS;
}
