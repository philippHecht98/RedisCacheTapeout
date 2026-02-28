// Copyright (c) 2026 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdint.h>
#include "config.h"

#define REDIS_CACHE_VALUE_LO_OFFSET 0x00
#define REDIS_CACHE_VALUE_HI_OFFSET 0x04
#define REDIS_CACHE_KEY_OFFSET 0x08
#define REDIS_CACHE_OP_OFFSET 0x0C

#define REDIS_CACHE_OP_NOOP 0x0u
#define REDIS_CACHE_OP_READ 0x1u
#define REDIS_CACHE_OP_UPSERT 0x2u
#define REDIS_CACHE_OP_DELETE 0x3u

#define REDIS_CACHE_CTRL_BUSY_MASK 0x1u
#define REDIS_CACHE_CTRL_OP_SHIFT 1u
#define REDIS_CACHE_CTRL_OP_MASK (0x7u << REDIS_CACHE_CTRL_OP_SHIFT)
#define REDIS_CACHE_CTRL_OP_ENCODE(op) ((uint32_t)(op) << REDIS_CACHE_CTRL_OP_SHIFT)
#define REDIS_CACHE_CTRL_OP_DECODE(ctrl) (((ctrl) & REDIS_CACHE_CTRL_OP_MASK) >> REDIS_CACHE_CTRL_OP_SHIFT)
#define REDIS_CACHE_CTRL_HIT_MASK (0x1u << 4)

#ifndef REDIS_CACHE_WAIT_CYCLES
#define REDIS_CACHE_WAIT_CYCLES 2u
#endif

#ifndef REDIS_CACHE_POLL_TIMEOUT
#define REDIS_CACHE_POLL_TIMEOUT 64u
#endif

#ifndef REDIS_CACHE_FAST_UNSAFE
#define REDIS_CACHE_FAST_UNSAFE 0u
#endif

#define REDIS_CACHE_STATUS_OK 0
#define REDIS_CACHE_STATUS_MISS 1
#define REDIS_CACHE_STATUS_ERR -1

int redis_cache_upsert(uint32_t key, uint64_t value);
int redis_cache_get(uint32_t key, uint64_t *value_out);
int redis_cache_delete(uint32_t key);
