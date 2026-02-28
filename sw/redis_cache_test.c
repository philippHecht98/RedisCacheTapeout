// Copyright (c) 2026 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include "uart.h"
#include "print.h"
#include "redis_cache.h"
#include "util.h"

static const char hex_symbols[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

static void print_hex32_padded(uint32_t value) {
    for (int idx = 7; idx >= 0; --idx) {
        uint32_t nibble = (value >> (idx * 4)) & 0xFu;
        putchar(hex_symbols[nibble]);
    }
}

static void print_hex64(uint64_t value) {
    putchar('0');
    putchar('x');
    print_hex32_padded((uint32_t)(value >> 32));
    print_hex32_padded((uint32_t)(value & 0xFFFFFFFFu));
}

static void print_get_result(uint32_t key) {
    uint64_t value = 0;
    int get_status = redis_cache_get(key, &value);

    printf("Get key=0x%x ", key);
    if (get_status == REDIS_CACHE_STATUS_OK) {
        printf("hit val=");
        print_hex64(value);
        printf("\r\n");
    } else if (get_status == REDIS_CACHE_STATUS_MISS) {
        printf("miss\r\n");
    } else {
        printf("error\r\n");
    }
}

static uint32_t next_prng_u32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

// static void redis_cache_mmio_sanity(void) {
//     printf("MMIO sanity start\r\n");

//     printf("Write VALUE_LO...\r\n");
//     *reg32(REDIS_CACHE_BASE_ADDR, REDIS_CACHE_VALUE_LO_OFFSET) = 0xA5A5A5A5u;
//     printf("Write VALUE_LO ok\r\n");

//     printf("Write VALUE_HI...\r\n");
//     *reg32(REDIS_CACHE_BASE_ADDR, REDIS_CACHE_VALUE_HI_OFFSET) = 0x5A5A5A5Au;
//     printf("Write VALUE_HI ok\r\n");

//     printf("Write KEY...\r\n");
//     *reg32(REDIS_CACHE_BASE_ADDR, REDIS_CACHE_KEY_OFFSET) = 0xCAFEBABEu;
//     printf("Write KEY ok\r\n");

//     printf("Write OP...\r\n");
//     *reg32(REDIS_CACHE_BASE_ADDR, REDIS_CACHE_OP_OFFSET) = REDIS_CACHE_CTRL_OP_ENCODE(REDIS_CACHE_OP_NOOP);
//     printf("Write OP ok\r\n");

//     printf("Read VALUE_LO...\r\n");
//     uint32_t val_lo = *reg32(REDIS_CACHE_BASE_ADDR, REDIS_CACHE_VALUE_LO_OFFSET);
//     printf("Read VALUE_LO=0x%x\r\n", val_lo);

//     printf("Read VALUE_HI...\r\n");
//     uint32_t val_hi = *reg32(REDIS_CACHE_BASE_ADDR, REDIS_CACHE_VALUE_HI_OFFSET);
//     printf("Read VALUE_HI=0x%x\r\n", val_hi);

//     printf("Read CTRL...\r\n");
//     uint32_t ctrl = *reg32(REDIS_CACHE_BASE_ADDR, REDIS_CACHE_OP_OFFSET);
//     printf("Read CTRL busy=%u op=0x%x\r\n",
//            (unsigned)((ctrl & REDIS_CACHE_CTRL_BUSY_MASK) != 0u),
//            (unsigned)REDIS_CACHE_CTRL_OP_DECODE(ctrl));

//     printf("MMIO sanity done\r\n");
// }

int main() {
    uart_init();

    printf("Redis cache test\r\n");
    // redis_cache_mmio_sanity();

    uint32_t key1 = 0x11111111u;
    uint64_t value1_1 = 0x1122334455667788ULL;
    uint64_t value1_2 = 0xAABBCCDD00112233ULL;

    uint32_t key2 = 0x22222222u;
    uint64_t value2_1 = 0xDEADBEEFCAFEBABEuLL;
    uint64_t value2_2 = 0xFEEDFACE12345678ULL;

    // Upesert first value for both keys
    printf("Upsert key1=0x%x val=", key1);
    print_hex64(value1_1);
    printf("\r\n");
    int status = redis_cache_upsert(key1, value1_1);
    printf("Upsert status=0x%x\r\n", (uint32_t)status);

    printf("Upsert key2=0x%x val=", key2);
    print_hex64(value2_1);
    printf("\r\n");
    status = redis_cache_upsert(key2, value2_1);
    printf("Upsert status=0x%x\r\n", (uint32_t)status);

    // Read back values for both keys
    print_get_result(key1);
    print_get_result(key2);

    // Update values for both keys
    printf("Update key=0x%x val=", key1);
    print_hex64(value1_2);
    printf("\r\n");
    status = redis_cache_upsert(key1, value1_2);
    printf("Update status=0x%x\r\n", (uint32_t)status);

    printf("Update key=0x%x val=", key2);
    print_hex64(value2_2);
    printf("\r\n");
    status = redis_cache_upsert(key2, value2_2);
    printf("Update status=0x%x\r\n", (uint32_t)status);

    // Read back updated values for both keys
    print_get_result(key1);
    print_get_result(key2);

    // Delete both keys
    printf("Delete key=0x%x\r\n", key1);
    status = redis_cache_delete(key1);
    printf("Delete status=0x%x\r\n", (uint32_t)status);

    printf("Delete key=0x%x\r\n", key2);
    status = redis_cache_delete(key2);
    printf("Delete status=0x%x\r\n", (uint32_t)status);

    // Read back after delete to confirm deletion
    print_get_result(key1);
    print_get_result(key2);

    // Insert 8 pseudo-random key/value pairs
    printf("Insert 8 pseudo-random entries\r\n");
    uint32_t prng_state = 0xC0FFEE42u;
    for (int idx = 0; idx < 8; ++idx) {
        uint32_t rand_key = next_prng_u32(&prng_state);
        uint64_t rand_value = ((uint64_t)next_prng_u32(&prng_state) << 32) |
                              (uint64_t)next_prng_u32(&prng_state);

        printf("Random upsert[%d] key=0x%x val=", idx, rand_key);
        print_hex64(rand_value);
        printf("\r\n");

        status = redis_cache_upsert(rand_key, rand_value);
        printf("Random upsert status=0x%x\r\n", (uint32_t)status);
    }

    uart_write_flush();
    return 0;
}
