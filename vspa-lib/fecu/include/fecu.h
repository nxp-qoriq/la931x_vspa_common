// SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
// Copyright 2016 - 2025 NXP Semiconductors

// =============================================================================
//! @file           fecu.h
//! @brief          FECU (Forward Error Correction Unit) Low-Level Driver
//! @author         NXP Semiconductors
//!
//! Protocol-agnostic FECU primitives: enumerations, inline register accessors,
//! and the fecuInit() function. No WiFi-specific types or headers included.
// =============================================================================

#ifndef __FECU_H__
#define __FECU_H__

#include <vspa/intrinsics.h>
#include <stdbool.h>
#include <stdint.h>
#include "ipreg_fecu.h"

// -----------------------------------------------------------------------------
//! @defgroup GROUP_FECU
//!
//! FECU Low-Level Driver -- protocol-agnostic register accessors and types.
//!
//! @{
// -----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//! @brief  LLR packing constants (soft-decision input to Viterbi / LDPC decoders)
// ----------------------------------------------------------------------------
#define BITS_PER_LLR        8
#define BYTES_PER_LLR       1
#define LLRS_PER_WORD32     (32 / BITS_PER_LLR)    // 4

// ----------------------------------------------------------------------------
//! @brief  FECU channel bandwidth / interleaver type
// ----------------------------------------------------------------------------
typedef enum {
    FECU_BW_20MHZ_802a   = 0b0000,
    FECU_BW_20MHZ_802ac  = 0b0001,
    FECU_BW_40MHZ_802ac  = 0b0010,
    FECU_BW_80MHZ_802ac  = 0b0011,
    FECU_BW_RU26_802ax   = 0b1000,
    FECU_BW_RU52_802ax   = 0b1001,
    FECU_BW_RU106_802ax  = 0b1010,
    FECU_BW_RU242_802ax  = 0b1011,
    FECU_BW_RU484_802ax  = 0b1100,
    FECU_BW_RU996_802ax  = 0b1101
} fecuBwT;

// ----------------------------------------------------------------------------
//! @brief  FECU modulation order (coded bits per sub-carrier)
// ----------------------------------------------------------------------------
typedef enum {
    FECU_MT_BPSK    = 0b000,  // 1 bit/sc
    FECU_MT_QPSK    = 0b001,  // 2 bits/sc
    FECU_MT_16QAM   = 0b010,  // 4 bits/sc
    FECU_MT_64QAM   = 0b011,  // 6 bits/sc
    FECU_MT_256QAM  = 0b100,  // 8 bits/sc
    FECU_MT_1024QAM = 0b101   // 10 bits/sc
} fecuModT;

// ----------------------------------------------------------------------------
//! @brief  FECU LDPC codeword size
// ----------------------------------------------------------------------------
typedef enum {
    FECU_LDPC_CW_648  = 0,
    FECU_LDPC_CW_1296 = 1,
    FECU_LDPC_CW_1944 = 2
} fecuLdpcCwSzT;

// ----------------------------------------------------------------------------
//! @brief  FECU start trigger type
// ----------------------------------------------------------------------------
typedef enum {
    FECU_START_IMM  = 0,
    FECU_START_DMA  = 1,
    FECU_START_IPPU = 2
} fecuStartT;

// ----------------------------------------------------------------------------
//! @brief  FEC coding rate
// ----------------------------------------------------------------------------
typedef enum {
    FEC_RATE_1_2 = 0,
    FEC_RATE_2_3 = 1,
    FEC_RATE_3_4 = 2,
    FEC_RATE_5_6 = 3
} fecuRateT;

// ----------------------------------------------------------------------------
//! @brief  Clear all pending FECU operations (w1c)
// ----------------------------------------------------------------------------
static inline void fecuClearPending(void) {
    __ip_write(FECU_CONFIG_ADDR, FECU_CONFIG_CLR_PEND_MASK, FECU_CONFIG_CLR_PEND_MASK);
}

// ----------------------------------------------------------------------------
//! @brief  Clear FECU start-error status flag (w1c)
// ----------------------------------------------------------------------------
static inline void fecuClearError(void) {
    __ip_write(FECU_STATUS_ADDR, FECU_STATUS_START_ERR_MASK, FECU_STATUS_START_ERR_MASK);
}

// ----------------------------------------------------------------------------
//! @brief  Set FECU DMEM input (source) address
//!
//! @note   Bits[31:24] carry the num_keep_read_bits field. Pass the combined
//!         value when keep-bits support is needed; otherwise pass a plain
//!         half-word address.
// ----------------------------------------------------------------------------
static inline void FECU_SetDmemInAddr(uint32_t dmemAddr) {
    __ip_write(FECU_DMEM_SRC_ADDR_ADDR, FECU_DMEM_SRC_BIT_MASK, dmemAddr);
}

// ----------------------------------------------------------------------------
//! @brief  Set FECU DMEM output (destination) address
//!
//! @note   Bits[31:24] carry the num_repeat_write_bits field.
// ----------------------------------------------------------------------------
static inline void FECU_SetDmemOutAddr(uint32_t dmemAddr) {
    __ip_write(FECU_DMEM_DST_ADDR_ADDR, FECU_DMEM_DEST_BIT_MASK, dmemAddr);
}

// ----------------------------------------------------------------------------
//! @brief  Set number of items (LLRs for decode, bits for encode) to read
// ----------------------------------------------------------------------------
static inline void FECU_SetDmemReadCnt(uint32_t readItems) {
    __ip_write(FECU_DMEM_READ_COUNT_ADDR, FECU_DMEM_RD_CNT_BIT_MASK, readItems);
}

// ----------------------------------------------------------------------------
//! @brief  Write FECU_CONTROL register -- this latches the command into the FIFO
// ----------------------------------------------------------------------------
static inline void FECU_SetControl(uint32_t control) {
    __ip_write(FECU_CTRL_ADDR, FECU_CTRL_BIT_MASK, control);
}

// ----------------------------------------------------------------------------
//! @brief  Return true if the FECU command FIFO is full
// ----------------------------------------------------------------------------
static inline bool FECU_IsCmdFifoFull(void) {
    return (bool)__ip_read(FECU_STATUS_ADDR, FECU_STATUS_CMD_FIFO_FULL_MASK);
}

// ----------------------------------------------------------------------------
//! @brief  Return true if FECU is busy or has a pending command
// ----------------------------------------------------------------------------
static inline bool FECU_IsBusyOrPending(void) {
    return (bool)__ip_read(FECU_STATUS_ADDR, FECU_STATUS_BUSY_OR_PEND_MASK);
}

// ----------------------------------------------------------------------------
//! @brief  Return the FECU DMEM source address (read-back)
// ----------------------------------------------------------------------------
static inline uint32_t FECU_GetDmemInAddr(void) {
    return __ip_read(FECU_DMEM_SRC_ADDR_ADDR, FECU_DMEM_SRC_ADDR_MASK);
}

// ----------------------------------------------------------------------------
//! @brief  Initialize FECU to a clean starting state.
//!
//! Clears any pending commands and any latched start-error flag.
//! There is no way to abort a FECU operation that is already in progress.
// ----------------------------------------------------------------------------
void fecuInit(void);

// -----------------------------------------------------------------------------
//! @} GROUP_FECU
// -----------------------------------------------------------------------------
#endif // __FECU_H__
