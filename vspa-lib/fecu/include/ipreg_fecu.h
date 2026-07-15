// SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
// Copyright 2016 - 2025 NXP Semiconductors

// =============================================================================
//! @file           ipreg_fecu.h
//! @brief          FECU (Forward Error Correction Unit) IP Register Mapping
//! @author         NXP Semiconductors
//!
//! Derived from ua-wifi-phy/wifi_phy/common/ipreg_fecu.h.
//! Stripped of all WiFi-specific protocol dependencies.
// =============================================================================

#ifndef __IPREG_FECU_H__
#define __IPREG_FECU_H__

#include <stdint.h>

// -----------------------------------------------------------------------------
//! @defgroup       GROUP_IPREG_FECU
//!
//! IP Register Address and Bit Mapping for FECU
//!
//! @{
// -----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//! @brief  FECU IP Register Memory Map (byte offsets from VSPA IP base)
//!         ADDR = byte_offset >> 2  (word address used by __ip_read/__ip_write)
// ----------------------------------------------------------------------------
#define FECU_CONFIG_BYTE_ADDR           0x300
#define FECU_CONFIG_ADDR                ((FECU_CONFIG_BYTE_ADDR) >> 2)

#define FECU_SIZES_BYTE_ADDR            0x304
#define FECU_SIZES_ADDR                 ((FECU_SIZES_BYTE_ADDR) >> 2)

#define FECU_NUM_PAD_BYTE_ADDR          0x308
#define FECU_NUM_PAD_ADDR               ((FECU_NUM_PAD_BYTE_ADDR) >> 2)

#define FECU_BCC_PUNC_MASK_BYTE_ADDR    0x30c
#define FECU_BCC_PUNC_MASK_ADDR         ((FECU_BCC_PUNC_MASK_BYTE_ADDR) >> 2)

#define FECU_BCC_CONFIG_BYTE_ADDR       0x310
#define FECU_BCC_CONFIG_ADDR            ((FECU_BCC_CONFIG_BYTE_ADDR) >> 2)

#define FECU_LDPC_CONFIG_BYTE_ADDR      0x314
#define FECU_LDPC_CONFIG_ADDR           ((FECU_LDPC_CONFIG_BYTE_ADDR) >> 2)

#define FECU_LDPC_SIZES_BYTE_ADDR       0x318
#define FECU_LDPC_SIZES_ADDR            ((FECU_LDPC_SIZES_BYTE_ADDR) >> 2)

#define FECU_LDPC_EXTRA_SHORT_BYTE_ADDR 0x31c
#define FECU_LDPC_EXTRA_SHORT_ADDR      ((FECU_LDPC_EXTRA_SHORT_BYTE_ADDR) >> 2)

#define FECU_LDPC_EXTRA_REP_BYTE_ADDR   0x320
#define FECU_LDPC_EXTRA_REP_ADDR        ((FECU_LDPC_EXTRA_REP_BYTE_ADDR) >> 2)

#define FECU_BYPASS_BYTE_ADDR           0x324
#define FECU_BYPASS_ADDR                ((FECU_BYPASS_BYTE_ADDR) >> 2)

#define FECU_SC_CONFIG_BYTE_ADDR        0x328
#define FECU_SC_CONFIG_ADDR             ((FECU_SC_CONFIG_BYTE_ADDR) >> 2)

#define FECU_DMEM_READ_COUNT_BYTE_ADDR  0x32c
#define FECU_DMEM_READ_COUNT_ADDR       ((FECU_DMEM_READ_COUNT_BYTE_ADDR) >> 2)

#define FECU_DMEM_SRC_ADDR_BYTE_ADDR    0x330
#define FECU_DMEM_SRC_ADDR_ADDR         ((FECU_DMEM_SRC_ADDR_BYTE_ADDR) >> 2)

#define FECU_DMEM_DST_ADDR_BYTE_ADDR    0x334
#define FECU_DMEM_DST_ADDR_ADDR         ((FECU_DMEM_DST_ADDR_BYTE_ADDR) >> 2)

#define FECU_DMEM_2ND_ADDR_BYTE_ADDR    0x338
#define FECU_DMEM_2ND_ADDR_ADDR         ((FECU_DMEM_2ND_ADDR_BYTE_ADDR) >> 2)

#define FECU_DMEM_3RD_ADDR_BYTE_ADDR    0x33c
#define FECU_DMEM_3RD_ADDR_ADDR         ((FECU_DMEM_3RD_ADDR_BYTE_ADDR) >> 2)

#define FECU_DMEM_4TH_ADDR_BYTE_ADDR    0x340
#define FECU_DMEM_4TH_ADDR_ADDR         ((FECU_DMEM_4TH_ADDR_BYTE_ADDR) >> 2)

#define FECU_DMEM_5TH_ADDR_BYTE_ADDR    0x344
#define FECU_DMEM_5TH_ADDR_ADDR         ((FECU_DMEM_5TH_ADDR_BYTE_ADDR) >> 2)

#define FECU_DMEM_6TH_ADDR_BYTE_ADDR    0x348
#define FECU_DMEM_6TH_ADDR_ADDR         ((FECU_DMEM_6TH_ADDR_BYTE_ADDR) >> 2)

#define FECU_DMEM_7TH_ADDR_BYTE_ADDR    0x34c
#define FECU_DMEM_7TH_ADDR_ADDR         ((FECU_DMEM_7TH_ADDR_BYTE_ADDR) >> 2)

#define FECU_DMEM_8TH_ADDR_BYTE_ADDR    0x350
#define FECU_DMEM_8TH_ADDR_ADDR         ((FECU_DMEM_8TH_ADDR_BYTE_ADDR) >> 2)

#define FECU_SAVE_RESTORE_BYTE_ADDR     0x354
#define FECU_SAVE_RESTORE_ADDR          ((FECU_SAVE_RESTORE_BYTE_ADDR) >> 2)

#define FECU_CTRL_BYTE_ADDR             0x358
#define FECU_CTRL_ADDR                  ((FECU_CTRL_BYTE_ADDR) >> 2)

#define FECU_STATUS_BYTE_ADDR           0x364
#define FECU_STATUS_ADDR                ((FECU_STATUS_BYTE_ADDR) >> 2)

#define FECU_DMEM_WRITE_COUNT_BYTE_ADDR 0x368
#define FECU_DMEM_WRITE_COUNT_ADDR      ((FECU_DMEM_WRITE_COUNT_BYTE_ADDR) >> 2)

#define FECU_LDPC_ENC_BLOCK_BYTE_ADDR   0x36c
#define FECU_LDPC_ENC_BLOCK_ADDR        ((FECU_LDPC_ENC_BLOCK_BYTE_ADDR) >> 2)

#define FECU_LDPC_ENC_STATUS_BYTE_ADDR  0x370
#define FECU_LDPC_ENC_STATUS_ADDR       ((FECU_LDPC_ENC_STATUS_BYTE_ADDR) >> 2)

#define FECU_LDPC_DEC_BLOCK_BYTE_ADDR   0x374
#define FECU_LDPC_DEC_BLOCK_ADDR        ((FECU_LDPC_DEC_BLOCK_BYTE_ADDR) >> 2)

#define FECU_LDPC_DEC_STATUS_BYTE_ADDR  0x378
#define FECU_LDPC_DEC_STATUS_ADDR       ((FECU_LDPC_DEC_STATUS_BYTE_ADDR) >> 2)

#define FECU_HW_PARAMS_BYTE_ADDR        0x380
#define FECU_HW_PARAMS_ADDR             ((FECU_HW_PARAMS_BYTE_ADDR) >> 2)

#define FECU_HW_LDPC_PARAMS_BYTE_ADDR   0x384
#define FECU_HW_LDPC_PARAMS_ADDR        ((FECU_HW_LDPC_PARAMS_BYTE_ADDR) >> 2)

// ----------------------------------------------------------------------------
//! @brief  FECU CONFIG Register bit fields and masks
// ----------------------------------------------------------------------------
typedef struct {
    uint32_t fecu_encode:       1;  //!< Bit[0]   1=encode, 0=decode
    uint32_t use_ldpc:          1;  //!< Bit[1]   1=LDPC, 0=BCC
    uint32_t clear_pending:     1;  //!< Bit[2]   w1c
    uint32_t sw_reset:          1;  //!< Bit[3]
    uint32_t cbits_per_sc:      3;  //!< Bits[6:4] modulation order
    uint32_t _reserved0:        1;  //!< Bit[7]
    uint32_t bw:                4;  //!< Bits[11:8]  channel bandwidth / type
    uint32_t num_streams:       4;  //!< Bits[15:12]
    uint32_t num_bcc_enc:       4;  //!< Bits[19:16]
    uint32_t _reserved1:        4;  //!< Bits[23:20]
    uint32_t stride:            6;  //!< Bits[29:24]
    uint32_t transpose_en:      1;  //!< Bit[30]
    uint32_t _reserved2:        1;  //!< Bit[31]
} fecuConfigRegT;

typedef union {
    fecuConfigRegT configFields;
    uint32_t       config32;
} fecuConfigUnionT;

#define FECU_CONFIG_BIT_MASK            0x000FFFFF

#define FECU_CONFIG_FECU_ENC_B          0
#define FECU_CONFIG_FECU_ENC_MASK       (0x1U << FECU_CONFIG_FECU_ENC_B)

#define FECU_CONFIG_USE_LDPC_B          1
#define FECU_CONFIG_USE_LDPC_MASK       (0x1U << FECU_CONFIG_USE_LDPC_B)

#define FECU_CONFIG_CLR_PEND_B          2
#define FECU_CONFIG_CLR_PEND_MASK       (0x1U << FECU_CONFIG_CLR_PEND_B)

#define FECU_CONFIG_SW_RESET_B          3
#define FECU_CONFIG_SW_RESET_MASK       (0x1U << FECU_CONFIG_SW_RESET_B)

#define FECU_CONFIG_CBITS_PER_SC_B      4
#define FECU_CONFIG_CBITS_PER_SC_MASK   (0x7U << FECU_CONFIG_CBITS_PER_SC_B)

#define FECU_CONFIG_BW_B                8
#define FECU_CONFIG_BW_MASK             (0xFU << FECU_CONFIG_BW_B)

#define FECU_CONFIG_NUM_STREAM_B        12
#define FECU_CONFIG_NUM_STREAM_MASK     (0xFU << FECU_CONFIG_NUM_STREAM_B)

#define FECU_CONFIG_NUM_BCC_B           16
#define FECU_CONFIG_NUM_BCC_MASK        (0xFU << FECU_CONFIG_NUM_BCC_B)

// ----------------------------------------------------------------------------
//! @brief  FECU SIZES Register
// ----------------------------------------------------------------------------
typedef struct {
    uint32_t dbps:  16; //!< Bits[15:0]  data bits per symbol
    uint32_t cbps:  16; //!< Bits[31:16] coded bits per symbol
} fecuSizesRegT;

typedef union {
    fecuSizesRegT sizesFields;
    uint32_t      sizes32;
} fecuSizesUnionT;

#define FECU_SIZES_BIT_MASK         0xFFFFFFFF
#define FECU_SIZES_DBPS_MASK        0x0000FFFF
#define FECU_SIZES_CBPS_MASK        0xFFFF0000
#define FECU_SIZES_CBPS_BIT_POS     16

// ----------------------------------------------------------------------------
//! @brief  FECU Number of Padding Bits Register
// ----------------------------------------------------------------------------
#define FECU_NUM_PAD_BITS_MASK      0x0000FFFF

// ----------------------------------------------------------------------------
//! @brief  FECU BCC Puncture Mask Register
// ----------------------------------------------------------------------------
#define FECU_PUNC_MASK_BIT_MASK     0xFFFFFFFF

// ----------------------------------------------------------------------------
//! @brief  FECU BCC Configuration Register
// ----------------------------------------------------------------------------
typedef struct {
    uint32_t punc_length:       5;  //!< Bits[4:0]
    uint32_t _reserved0:        11; //!< Bits[15:5]
    uint32_t enc_start_state:   6;  //!< Bits[21:16]
    uint32_t _reserved1:        2;  //!< Bits[23:22]
    uint32_t enc_end_state:     6;  //!< Bits[29:24]
    uint32_t _reserved2:        2;  //!< Bits[31:30]
} fecuBccConfigRegT;

#define FECU_BCC_CONFIG_BIT_MASK        0x3FFFFFFF
#define FECU_BCC_PUNC_LENGTH_MASK       0x0000001F
#define FECU_BCC_ENC_START_STATE_MASK   0x003F0000
#define FECU_BCC_ENC_END_STATE_MASK     0x3F000000

// ----------------------------------------------------------------------------
//! @brief  FECU LDPC Configuration Register
// ----------------------------------------------------------------------------
typedef struct {
    uint32_t repeat:        1;  //!< Bit[0]
    uint32_t _reserved0:    3;  //!< Bits[3:1]
    uint32_t block_length:  2;  //!< Bits[5:4]  0=648, 1=1296, 2=1944
    uint32_t _reserved1:    2;  //!< Bits[7:6]
    uint32_t coding_rate:   2;  //!< Bits[9:8]  0=1/2, 1=2/3, 2=3/4, 3=5/6
    uint32_t _reserved2:    6;  //!< Bits[15:10]
    uint32_t max_iter:      8;  //!< Bits[23:16]
    uint32_t _reserved3:    8;  //!< Bits[31:24]
} fecuLdpcConfigRegT;

#define FECU_LDPC_REPEAT_B              0
#define FECU_LDPC_REPEAT_MASK           (0x1U << FECU_LDPC_REPEAT_B)

#define FECU_LDPC_BLOCK_LENGTH_B        4
#define FECU_LDPC_BLOCK_LENGTH_MASK     (0x3U << FECU_LDPC_BLOCK_LENGTH_B)

#define FECU_LDPC_CODING_RATE_B         8
#define FECU_LDPC_CODING_RATE_MASK      (0x3U << FECU_LDPC_CODING_RATE_B)

#define FECU_LDPC_MAX_ITER_B            16
#define FECU_LDPC_MAX_ITER_MASK         (0xFFU << FECU_LDPC_MAX_ITER_B)

// ----------------------------------------------------------------------------
//! @brief  FECU LDPC Sizes Register
// ----------------------------------------------------------------------------
#define FECU_LDPC_NUM_SHORT_BITS_MASK   0x000007FF
#define FECU_LDPC_NUM_REP_BITS_MASK     0xFFFF0000

// ----------------------------------------------------------------------------
//! @brief  FECU Bypass Register
// ----------------------------------------------------------------------------
typedef struct {
    uint32_t int_bypass:    1;  //!< Bit[0]
    uint32_t vd_bypass:     1;  //!< Bit[1]
    uint32_t ce_bypass:     1;  //!< Bit[2]
    uint32_t sc_bypass:     1;  //!< Bit[3]
    uint32_t le_bypass:     1;  //!< Bit[4]
    uint32_t ld_bypass:     1;  //!< Bit[5]
    uint32_t di_bypass:     1;  //!< Bit[6]
    uint32_t _reserved:     25; //!< Bits[31:7]
} fecuBypassRegT;

#define FECU_BYPASS_REG_BIT_MASK    0x0000007F
#define FECU_BYPASS_NOTHING         0x00000000

#define FECU_BYPASS_INT_B           0
#define FECU_BYPASS_INT_MASK        (0x1U << FECU_BYPASS_INT_B)

#define FECU_BYPASS_VD_B            1
#define FECU_BYPASS_VD_MASK         (0x1U << FECU_BYPASS_VD_B)

#define FECU_BYPASS_CE_B            2
#define FECU_BYPASS_CE_MASK         (0x1U << FECU_BYPASS_CE_B)

#define FECU_BYPASS_SC_B            3
#define FECU_BYPASS_SC_MASK         (0x1U << FECU_BYPASS_SC_B)

#define FECU_BYPASS_LE_B            4
#define FECU_BYPASS_LE_MASK         (0x1U << FECU_BYPASS_LE_B)

#define FECU_BYPASS_LD_B            5
#define FECU_BYPASS_LD_MASK         (0x1U << FECU_BYPASS_LD_B)

#define FECU_BYPASS_DI_B            6
#define FECU_BYPASS_DI_MASK         (0x1U << FECU_BYPASS_DI_B)

// ----------------------------------------------------------------------------
//! @brief  FECU Scrambler Config Register
// ----------------------------------------------------------------------------
typedef struct {
    uint32_t lfsr_init:     7;  //!< Bits[6:0]
    uint32_t _reserved0:    1;  //!< Bit[7]
    uint32_t mode_802b:     1;  //!< Bit[8]  set for 802.11b mode
    uint32_t _reserved1:    23; //!< Bits[31:9]
} fecuScrConfigRegT;

#define FECU_SC_CONFIG_LFSR_INIT_MASK   0x7F

#define FECU_SC_CONFIG_MODE_802B_B      8
#define FECU_SC_CONFIG_MODE_802B_MASK   (0x1U << FECU_SC_CONFIG_MODE_802B_B)

// ----------------------------------------------------------------------------
//! @brief  FECU DMEM Read Count Register
// ----------------------------------------------------------------------------
#define FECU_DMEM_RD_CNT_BIT_MASK   0x0000FFFF

// ----------------------------------------------------------------------------
//! @brief  FECU DMEM Source Address Register
// ----------------------------------------------------------------------------
#define FECU_DMEM_SRC_BIT_MASK          0xFFFFFFFF
#define FECU_DMEM_SRC_ADDR_MASK         0x00FFFFFF
#define FECU_DMEM_SRC_KEEP_BITS_MASK    0xFF000000

// ----------------------------------------------------------------------------
//! @brief  FECU DMEM Destination Address Register
// ----------------------------------------------------------------------------
#define FECU_DMEM_DEST_BIT_MASK         0xFFFFFFFF
#define FECU_DMEM_DEST_ADDR_MASK        0x00FFFFFF
#define FECU_DMEM_DEST_REP_BITS_MASK    0xFF000000

// ----------------------------------------------------------------------------
//! @brief  FECU Context Save/Restore Register
// ----------------------------------------------------------------------------
#define FECU_SAVE_RESTORE_ADDR_MASK     0x00FFFFFF
#define FECU_SAVE_RESTORE_REST_B        30
#define FECU_SAVE_RESTORE_SAVE_B        31
#define FECU_SAVE_RESTORE_REST_MASK     (0x1U << FECU_SAVE_RESTORE_REST_B)
#define FECU_SAVE_RESTORE_SAVE_MASK     (0x1U << FECU_SAVE_RESTORE_SAVE_B)

// ----------------------------------------------------------------------------
//! @brief  FECU Control Register
//!
//! NOTE: writing this register latches a new operation into the FECU command
//! FIFO. All other FECU registers must be set up BEFORE writing FECU_CTRL.
// ----------------------------------------------------------------------------
typedef struct {
    uint32_t start_type:        2;  //!< Bits[1:0] 0=imm, 1=DMA, 2=IPPU
    uint32_t queue_output:      1;  //!< Bit[2]
    uint32_t _reserved0:        1;  //!< Bit[3]
    uint32_t first_sym:         1;  //!< Bit[4]
    uint32_t one_b4_last_sym:   1;  //!< Bit[5]
    uint32_t last_sym:          1;  //!< Bit[6]
    uint32_t _reserved1:        1;  //!< Bit[7]
    uint32_t dma_go_en:         1;  //!< Bit[8]
    uint32_t ippu_go_en:        1;  //!< Bit[9]
    uint32_t vcpu_go_en:        1;  //!< Bit[10]
    uint32_t dec_err_go_en:     1;  //!< Bit[11]
    uint32_t irqen_done:        1;  //!< Bit[12]
    uint32_t _reserved2:        19; //!< Bits[31:13]
} fecuControlRegT;

typedef union {
    fecuControlRegT controlFields;
    uint32_t        control32;
} fecuControlUnionT;

#define FECU_CTRL_BIT_MASK              0x00001FFF

#define FECU_CTRL_START_TYPE_B          0
#define FECU_CTRL_START_TYPE_MASK       (0x3U << FECU_CTRL_START_TYPE_B)
#define FECU_CTRL_START_IMM             0x0
#define FECU_CTRL_START_DMA             0x1
#define FECU_CTRL_START_IPPU            0x2

#define FECU_CTRL_1ST_SYM_B             4
#define FECU_CTRL_1ST_SYM_MASK          (0x1U << FECU_CTRL_1ST_SYM_B)

#define FECU_CTRL_2ND_2_LAST_SYM_B      5
#define FECU_CTRL_2ND_2_LAST_SYM_MASK   (0x1U << FECU_CTRL_2ND_2_LAST_SYM_B)

#define FECU_CTRL_LAST_SYM_B            6
#define FECU_CTRL_LAST_SYM_MASK         (0x1U << FECU_CTRL_LAST_SYM_B)

#define FECU_CTRL_VCPU_GO_EN_B          10
#define FECU_CTRL_VCPU_GO_EN_MASK       (0x1U << FECU_CTRL_VCPU_GO_EN_B)

#define FECU_CTRL_IRQ_EN_DONE_B         12
#define FECU_CTRL_IRQ_EN_DONE_MASK      (0x1U << FECU_CTRL_IRQ_EN_DONE_B)

// Convenience: single-block operation -- first symbol AND last symbol both set
#define FECU_CTRL_SINGLE_BLOCK \
    (FECU_CTRL_1ST_SYM_MASK | FECU_CTRL_LAST_SYM_MASK | FECU_CTRL_START_IMM)

// ----------------------------------------------------------------------------
//! @brief  FECU Status Register
// ----------------------------------------------------------------------------
typedef struct {
    uint32_t busy:              1;  //!< Bit[0]
    uint32_t busy_or_pend:      1;  //!< Bit[1]
    uint32_t cmd_fifo_full:     1;  //!< Bit[2]
    uint32_t suspend:           1;  //!< Bit[3]
    uint32_t cmd_depth:         2;  //!< Bits[5:4]
    uint32_t _reserved0:        2;  //!< Bits[7:6]
    uint32_t start_src:         2;  //!< Bits[9:8]
    uint32_t start_err:         1;  //!< Bit[10] w1c
    uint32_t _reserved1:        21; //!< Bits[31:11]
} fecuStatusRegT;

#define FECU_STATUS_BUSY_B              0
#define FECU_STATUS_BUSY_MASK           (0x1U << FECU_STATUS_BUSY_B)

#define FECU_STATUS_BUSY_OR_PEND_B      1
#define FECU_STATUS_BUSY_OR_PEND_MASK   (0x1U << FECU_STATUS_BUSY_OR_PEND_B)

#define FECU_STATUS_CMD_FIFO_FULL_B     2
#define FECU_STATUS_CMD_FIFO_FULL_MASK  (0x1U << FECU_STATUS_CMD_FIFO_FULL_B)

#define FECU_STATUS_START_ERR_B         10
#define FECU_STATUS_START_ERR_MASK      (0x1U << FECU_STATUS_START_ERR_B)

// ----------------------------------------------------------------------------
//! @brief  FECU LDPC Decode Block Status Register
// ----------------------------------------------------------------------------
#define FECU_LDPC_DEC_ERR_B             24
#define FECU_LDPC_DEC_ERR_MASK          (0x1U << FECU_LDPC_DEC_ERR_B)

#define FECU_LDPC_DEC_NUM_CW_BAD_MASK   0x00FF0000
#define FECU_LDPC_DEC_NUM_CW_GOOD_MASK  0xFF000000

// ----------------------------------------------------------------------------
//! @brief  FECU Hardware Parameters Register
// ----------------------------------------------------------------------------
typedef struct {
    uint32_t max_streams:   4;  //!< Bits[3:0]
    uint32_t max_bcc_paths: 4;  //!< Bits[7:4]
    uint32_t ldpc_support:  1;  //!< Bit[8]
    uint32_t context_sr:    1;  //!< Bit[9]
    uint32_t xpose_present: 1;  //!< Bit[10]
    uint32_t _reserved:     21; //!< Bits[31:11]
} fecuHwParamRegT;

// -----------------------------------------------------------------------------
//! @} GROUP_IPREG_FECU
// -----------------------------------------------------------------------------
#endif // __IPREG_FECU_H__
