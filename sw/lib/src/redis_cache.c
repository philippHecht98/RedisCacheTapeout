// Copyright (c) 2026 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "redis_cache.h"
#include "util.h"

static volatile uint32_t *const redis_cache_value_lo_reg = (volatile uint32_t *)(REDIS_CACHE_BASE_ADDR + REDIS_CACHE_VALUE_LO_OFFSET);
static volatile uint32_t *const redis_cache_value_hi_reg = (volatile uint32_t *)(REDIS_CACHE_BASE_ADDR + REDIS_CACHE_VALUE_HI_OFFSET);
static volatile uint32_t *const redis_cache_key_reg = (volatile uint32_t *)(REDIS_CACHE_BASE_ADDR + REDIS_CACHE_KEY_OFFSET);
static volatile uint32_t *const redis_cache_op_reg = (volatile uint32_t *)(REDIS_CACHE_BASE_ADDR + REDIS_CACHE_OP_OFFSET);

static inline void redis_cache_write_value(uint64_t value) {
    *redis_cache_value_lo_reg = (uint32_t)(value & 0xFFFFFFFFu);
    *redis_cache_value_hi_reg = (uint32_t)(value >> 32);
}

static inline void redis_cache_write_key(uint32_t key) {
    *redis_cache_key_reg = key;
}

static inline void redis_cache_issue_op(uint32_t op) {
    *redis_cache_op_reg = REDIS_CACHE_CTRL_OP_ENCODE(op);
}

static inline uint32_t redis_cache_read_ctrl(void) {
    return *redis_cache_op_reg;
}

static int redis_cache_wait_idle(uint32_t timeout, uint32_t *ctrl_out) {
#if REDIS_CACHE_FAST_UNSAFE
    (void)timeout;
    uint32_t ctrl;
    do {
        ctrl = redis_cache_read_ctrl();
    } while ((ctrl & (REDIS_CACHE_CTRL_BUSY_MASK | REDIS_CACHE_CTRL_OP_MASK)) != 0u);

    if (ctrl_out != 0) {
        *ctrl_out = ctrl;
    }
    return 0;
#else
    for (uint32_t attempt = 0; attempt < timeout; ++attempt) {
        uint32_t ctrl = redis_cache_read_ctrl();
        if ((ctrl & (REDIS_CACHE_CTRL_BUSY_MASK | REDIS_CACHE_CTRL_OP_MASK)) == 0u) {
            if (ctrl_out != 0) {
                *ctrl_out = ctrl;
            }
            return 0;
        }
    }

    return -1;
#endif
}

int redis_cache_upsert(uint32_t key, uint64_t value) {
    redis_cache_write_value(value);
    redis_cache_write_key(key);
    redis_cache_issue_op(REDIS_CACHE_OP_UPSERT);
    uint32_t ctrl = 0;
    if (redis_cache_wait_idle(REDIS_CACHE_POLL_TIMEOUT, &ctrl) != 0) {
        return REDIS_CACHE_STATUS_ERR;
    }
    if ((ctrl & REDIS_CACHE_CTRL_HIT_MASK) == 0u) {
        return REDIS_CACHE_STATUS_MISS;
    }

    return REDIS_CACHE_STATUS_OK;
}

int redis_cache_get(uint32_t key, uint64_t *value_out) {
    if (value_out == 0) {
        return REDIS_CACHE_STATUS_ERR;
    }

    redis_cache_write_key(key);
    redis_cache_issue_op(REDIS_CACHE_OP_READ);

    uint32_t ctrl = 0;
    if (redis_cache_wait_idle(REDIS_CACHE_POLL_TIMEOUT, &ctrl) != 0) {
        *value_out = 0;
        return REDIS_CACHE_STATUS_ERR;
    }

    if ((ctrl & REDIS_CACHE_CTRL_HIT_MASK) == 0u) {
        *value_out = 0;
        return REDIS_CACHE_STATUS_MISS;
    }

    uint32_t lo = *redis_cache_value_lo_reg;
    uint32_t hi = *redis_cache_value_hi_reg;
    *value_out = ((uint64_t)hi << 32) | lo;

    return REDIS_CACHE_STATUS_OK;
}

int redis_cache_delete(uint32_t key) {
    redis_cache_write_key(key);
    redis_cache_issue_op(REDIS_CACHE_OP_DELETE);
    uint32_t ctrl = 0;
    if (redis_cache_wait_idle(REDIS_CACHE_POLL_TIMEOUT, &ctrl) != 0) {
        return REDIS_CACHE_STATUS_ERR;
    }
    if ((ctrl & REDIS_CACHE_CTRL_HIT_MASK) == 0u) {
        return REDIS_CACHE_STATUS_MISS;
    }

    return REDIS_CACHE_STATUS_OK;
}
