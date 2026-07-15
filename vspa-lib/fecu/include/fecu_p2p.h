// SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
// Copyright 2025 NXP Semiconductors

// =============================================================================
//! @file           fecu_p2p.h
//! @brief          Generic Point-to-Point FEC API using the LA9310 FECU block
//! @author         NXP Semiconductors
//!
//! This layer sits on top of the low-level fecu.h driver.  It provides a
//! simple configure + trigger interface that is independent of any specific
//! wireless standard (not tied to 802.11 or any other protocol).
//!
//! Typical usage:
//!   1. Fill a fecuP2pCfgT struct with the desired FEC parameters.
//!   2. Load raw input data into VSPA DMEM.
//!   3. Call fecu_p2p_encode() or fecu_p2p_decode().
//!   4. Call fecu_p2p_wait_done() to poll until FECU finishes.
//!   5. Read the result from the destination DMEM address.
// =============================================================================

#ifndef __FECU_P2P_H__
#define __FECU_P2P_H__

#include <stdbool.h>
#include <stdint.h>
#include "fecu.h"

// -----------------------------------------------------------------------------
//! @defgroup GROUP_FECU_P2P
//!
//! Generic P2P FEC layer -- protocol-agnostic encode/decode API.
//!
//! @{
// -----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//! @brief  BCC puncture table entry
//!
//! Standard 802.11-compatible puncture masks.  The table index is fecuRateT.
//! The same masks can be used for any custom BCC-coded P2P link that uses the
//! LA9310 FECU convolutional encoder / Viterbi decoder.
// ----------------------------------------------------------------------------
#define FECU_P2P_PUNC_LENGTH_1_2    2       //!< punc_mask length for rate 1/2
#define FECU_P2P_PUNC_MASK_1_2      0x3U    //!< keep all 2 bits

#define FECU_P2P_PUNC_LENGTH_2_3    4       //!< punc_mask length for rate 2/3
#define FECU_P2P_PUNC_MASK_2_3      0x7U    //!< keep 3 out of 4

#define FECU_P2P_PUNC_LENGTH_3_4    6       //!< punc_mask length for rate 3/4
#define FECU_P2P_PUNC_MASK_3_4      0x27U   //!< keep 4 out of 6

#define FECU_P2P_PUNC_LENGTH_5_6    10      //!< punc_mask length for rate 5/6
#define FECU_P2P_PUNC_MASK_5_6      0x267U  //!< keep 6 out of 10

// ----------------------------------------------------------------------------
//! @brief  FECU P2P configuration structure
//!
//! One instance describes a single FEC operation (one FECU command).
//! For a P2P link, pre-compute this once per modulation/coding scheme (MCS)
//! change and reuse it for every subsequent block.
// ----------------------------------------------------------------------------
typedef struct {
    // --- FEC scheme selection ------------------------------------------------
    bool           use_ldpc;        //!< false = BCC/Viterbi,  true = LDPC

    // --- Shared parameters ---------------------------------------------------
    fecuBwT        bw;              //!< Interleaver bandwidth setting
    fecuModT       mod;             //!< Modulation order (BPSK .. 256QAM)
    fecuRateT      rate;            //!< Coding rate (used to look up BCC punc
                                    //!< table or set LDPC coding_rate field)

    // --- Symbol sizing -------------------------------------------------------
    uint16_t       data_bits;       //!< Input data bits per FECU operation
    uint16_t       coded_bits;      //!< Output coded bits per FECU operation
                                    //!<   (used for LDPC encoder sizing only)

    // --- BCC-specific parameters (ignored when use_ldpc = true) -------------
    uint16_t       num_pad_bits;    //!< Tail/padding bits after last data bit
    uint32_t       bcc_punc_mask;   //!< BCC puncture bit mask
    uint8_t        bcc_punc_len;    //!< Number of valid bits in bcc_punc_mask

    // --- LDPC-specific parameters (ignored when use_ldpc = false) -----------
    fecuLdpcCwSzT  ldpc_cw_sz;     //!< Codeword size: 648 / 1296 / 1944 bits
    uint8_t        ldpc_max_iter;   //!< Max decoder iterations (0 = 256)
    uint16_t       ldpc_num_short;  //!< Shortening bits per codeword
    uint16_t       ldpc_num_rep;    //!< Repetition / puncture bits per codeword

    // --- Scrambler -----------------------------------------------------------
    uint8_t        sc_lfsr_init;    //!< 7-bit LFSR seed (Tx); ignored on Rx
                                    //!<   for the self-sync scrambler mode
    // --- Bypass flags (set to true to skip individual pipeline stages) -------
    bool           bypass_scrambler;    //!< Skip scrambler / de-scrambler
    bool           bypass_interleaver;  //!< Skip interleaver / tone mapper
} fecuP2pCfgT;

// ----------------------------------------------------------------------------
//! @brief  Initialize FECU to a clean state (must be called once at startup).
// ----------------------------------------------------------------------------
void fecu_p2p_init(void);

// ----------------------------------------------------------------------------
//! @brief  Configure FECU and trigger an encode operation.
//!
//! Programs all FECU registers for the given configuration and immediately
//! starts the encode (FECU_CTRL_SINGLE_BLOCK -- first+last symbol, immediate
//! start).  The caller must call fecu_p2p_wait_done() before reading the
//! result.
//!
//! @param[in]  cfg           FEC configuration.
//! @param[in]  src_dmem_addr Half-word (16-bit) DMEM address of input bits.
//! @param[in]  dst_dmem_addr Half-word (16-bit) DMEM address for output bits.
// ----------------------------------------------------------------------------
void fecu_p2p_encode(const fecuP2pCfgT *cfg,
                     uint32_t           src_dmem_addr,
                     uint32_t           dst_dmem_addr);

// ----------------------------------------------------------------------------
//! @brief  Configure FECU and trigger a decode operation.
//!
//! Programs all FECU registers for the given configuration and immediately
//! starts the decode (FECU_CTRL_SINGLE_BLOCK -- first+last symbol, immediate
//! start).  Input data is soft-decision LLRs, one signed 8-bit value per coded
//! bit (di_bypass is not set, so the DMEM interface always uses 8-bit LLRs).
//! The caller must call fecu_p2p_wait_done() before reading the result.
//!
//! @param[in]  cfg           FEC configuration.
//! @param[in]  src_dmem_addr Half-word (16-bit) DMEM address of input LLRs.
//!                           On VSPA a data pointer is already a half-word
//!                           address; pass it directly (no shifting).
//! @param[in]  dst_dmem_addr Half-word (16-bit) DMEM address for decoded bits.
//!
//! @return     true  if the LDPC decoder reported a decode failure (any bad
//!             codeword); always false for BCC (Viterbi has no failure flag).
// ----------------------------------------------------------------------------
bool fecu_p2p_decode(const fecuP2pCfgT *cfg,
                     uint32_t           src_dmem_addr,
                     uint32_t           dst_dmem_addr);

// ----------------------------------------------------------------------------
//! @brief  Busy-wait (VCPU spin-poll) until FECU is not busy and has no
//!         pending commands.
// ----------------------------------------------------------------------------
void fecu_p2p_wait_done(void);

// ----------------------------------------------------------------------------
//! @brief  Populate BCC puncture mask and length from a coding rate enum.
//!
//! Convenience helper for callers that only know the rate and need to fill
//! the bcc_punc_mask / bcc_punc_len fields of fecuP2pCfgT.
//!
//! @param[in]  rate      Desired coding rate.
//! @param[out] punc_mask Puncture bit mask for FECU_BCC_PUNC_MASK register.
//! @param[out] punc_len  Puncture mask length for FECU_BCC_CONFIG register.
// ----------------------------------------------------------------------------
void fecu_p2p_bcc_punc_from_rate(fecuRateT  rate,
                                  uint32_t  *punc_mask,
                                  uint8_t   *punc_len);

// -----------------------------------------------------------------------------
//! @} GROUP_FECU_P2P
// -----------------------------------------------------------------------------
#endif // __FECU_P2P_H__
