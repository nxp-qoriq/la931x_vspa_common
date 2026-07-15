// SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
// Copyright 2025 NXP Semiconductors

// =============================================================================
//! @file           fecu_p2p.c
//! @brief          Generic Point-to-Point FEC API -- FECU encode/decode driver
//! @author         NXP Semiconductors
//!
//! Implements the fecu_p2p_* API declared in fecu_p2p.h.
//! No WiFi-specific types, headers, or constants are used here.
// =============================================================================

#include "fecu_p2p.h"

// ----------------------------------------------------------------------------
// Internal BCC puncture look-up table  [rate][0]=mask, [rate][1]=length
// ----------------------------------------------------------------------------
static const uint32_t s_bccPuncMask[4] = {
    FECU_P2P_PUNC_MASK_1_2,   // FEC_RATE_1_2
    FECU_P2P_PUNC_MASK_2_3,   // FEC_RATE_2_3
    FECU_P2P_PUNC_MASK_3_4,   // FEC_RATE_3_4
    FECU_P2P_PUNC_MASK_5_6    // FEC_RATE_5_6
};

static const uint8_t s_bccPuncLen[4] = {
    FECU_P2P_PUNC_LENGTH_1_2,
    FECU_P2P_PUNC_LENGTH_2_3,
    FECU_P2P_PUNC_LENGTH_3_4,
    FECU_P2P_PUNC_LENGTH_5_6
};

// ----------------------------------------------------------------------------
// Internal helpers
// ----------------------------------------------------------------------------

// Build FECU_CONFIG word for an encode or decode operation.
static uint32_t build_config(const fecuP2pCfgT *cfg, bool encode)
{
    fecuConfigUnionT c;
    c.config32 = 0;

    c.configFields.fecu_encode = encode ? 1U : 0U;
    c.configFields.use_ldpc    = cfg->use_ldpc ? 1U : 0U;
    c.configFields.cbits_per_sc = (uint32_t)cfg->mod;
    c.configFields.bw           = (uint32_t)cfg->bw;
    c.configFields.num_streams  = 1U;   // single-stream P2P
    c.configFields.num_bcc_enc  = cfg->use_ldpc ? 0U : 1U;

    return c.config32;
}

// Program the common (encode+decode) register set -- everything except CTRL.
static void program_common_regs(const fecuP2pCfgT *cfg, bool encode,
                                 uint32_t src_dmem_addr, uint32_t dst_dmem_addr)
{
    fecuSizesUnionT sz;
    uint32_t bypass = FECU_BYPASS_NOTHING;

    // FECU_CONFIG
    __ip_write(FECU_CONFIG_ADDR, FECU_CONFIG_BIT_MASK,
               build_config(cfg, encode));

    // FECU_SIZES: data and coded bits per operation
    sz.sizesFields.dbps = cfg->data_bits;
    sz.sizesFields.cbps = cfg->coded_bits;
    __ip_write(FECU_SIZES_ADDR, FECU_SIZES_BIT_MASK, sz.sizes32);

    if (!cfg->use_ldpc) {
        // BCC path
        __ip_write(FECU_NUM_PAD_ADDR,
                   FECU_NUM_PAD_BITS_MASK,
                   cfg->num_pad_bits);
        __ip_write(FECU_BCC_PUNC_MASK_ADDR,
                   FECU_PUNC_MASK_BIT_MASK,
                   cfg->bcc_punc_mask);
        __ip_write(FECU_BCC_CONFIG_ADDR,
                   FECU_BCC_CONFIG_BIT_MASK,
                   (uint32_t)cfg->bcc_punc_len);
    } else {
        // LDPC path
        // FECU_LDPC_REPEAT bit[0]: enable repeat-codeword mode.
        // Only set when ldpc_num_rep > 0; setting it with no repeats
        // corrupts the LDPC engine state.
        uint32_t ldpc_repeat = (cfg->ldpc_num_rep > 0U) ? 1U : 0U;
        uint32_t ldpc_cfg =
            ((uint32_t)cfg->ldpc_max_iter << FECU_LDPC_MAX_ITER_B) |
            ((uint32_t)cfg->rate          << FECU_LDPC_CODING_RATE_B) |
            ((uint32_t)cfg->ldpc_cw_sz    << FECU_LDPC_BLOCK_LENGTH_B)|
            (ldpc_repeat << FECU_LDPC_REPEAT_B);
        __ip_write(FECU_LDPC_CONFIG_ADDR, FECU_LDPC_BLOCK_LENGTH_MASK |
                   FECU_LDPC_CODING_RATE_MASK | FECU_LDPC_MAX_ITER_MASK | FECU_LDPC_REPEAT_MASK,
                   ldpc_cfg);

        uint32_t ldpc_sz =
            ((uint32_t)cfg->ldpc_num_rep   << 16) |
            ((uint32_t)cfg->ldpc_num_short & FECU_LDPC_NUM_SHORT_BITS_MASK);
        __ip_write(FECU_LDPC_SIZES_ADDR, FECU_SIZES_BIT_MASK, ldpc_sz);
    }

    // Bypass register -- optional stage skips
    if (cfg->bypass_scrambler)
        bypass |= FECU_BYPASS_SC_MASK;
    if (cfg->bypass_interleaver)
        bypass |= FECU_BYPASS_INT_MASK;
    __ip_write(FECU_BYPASS_ADDR, FECU_BYPASS_REG_BIT_MASK, bypass);

    // Scrambler LFSR seed (only relevant for encode; self-sync on decode)
    __ip_write(FECU_SC_CONFIG_ADDR, FECU_SC_CONFIG_LFSR_INIT_MASK,
               (uint32_t)cfg->sc_lfsr_init);

    // DMEM addresses and read count
    FECU_SetDmemInAddr(src_dmem_addr);
    FECU_SetDmemOutAddr(dst_dmem_addr);
    FECU_SetDmemReadCnt(cfg->data_bits);   // bits (encode) or LLRs (decode)
}

// ----------------------------------------------------------------------------
// Public API
// ----------------------------------------------------------------------------

void fecu_p2p_init(void)
{
    fecuInit();
}

void fecu_p2p_encode(const fecuP2pCfgT *cfg,
                     uint32_t           src_dmem_addr,
                     uint32_t           dst_dmem_addr)
{
    program_common_regs(cfg, true, src_dmem_addr, dst_dmem_addr);
    // Trigger: single-block (first symbol + last symbol), immediate start
    FECU_SetControl(FECU_CTRL_SINGLE_BLOCK);
}

bool fecu_p2p_decode(const fecuP2pCfgT *cfg,
                     uint32_t           src_dmem_addr,
                     uint32_t           dst_dmem_addr)
{
    program_common_regs(cfg, false, src_dmem_addr, dst_dmem_addr);
    // FECU_DMEM_READ_COUNT for decode:
    // di_bypass (BYPASS register bit[6]) is never set, so scale=8 is always active.
    // The register unit is LLRs (8-bit each); hardware multiplies by 8 internally.
    // Pass coded_bits regardless of bypass_interleaver.
    FECU_SetDmemReadCnt((uint32_t)cfg->coded_bits);
    // Trigger: single-block, immediate start
    FECU_SetControl(FECU_CTRL_SINGLE_BLOCK);

    // Wait for completion before checking status
    fecu_p2p_wait_done();

    // Return LDPC decode error flag (always 0 for BCC)
    if (cfg->use_ldpc) {
        uint32_t dec_sts = __ip_read(FECU_LDPC_DEC_STATUS_ADDR,
                                     FECU_LDPC_DEC_ERR_MASK);
        if (dec_sts) {
            // Clear the w1c error bit so it does not persist
            __ip_write(FECU_LDPC_DEC_STATUS_ADDR,
                       FECU_LDPC_DEC_ERR_MASK, FECU_LDPC_DEC_ERR_MASK);
            return true;
        }
    }
    return false;
}

void fecu_p2p_wait_done(void)
{
    while (FECU_IsBusyOrPending()) {
        /* spin */
    }
}

void fecu_p2p_bcc_punc_from_rate(fecuRateT  rate,
                                  uint32_t  *punc_mask,
                                  uint8_t   *punc_len)
{
    uint32_t idx = (uint32_t)rate;
    if (idx > 3U) idx = 0U;   // clamp to valid range
    *punc_mask = s_bccPuncMask[idx];
    *punc_len  = s_bccPuncLen[idx];
}
