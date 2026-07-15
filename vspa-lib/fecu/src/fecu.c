// SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
// Copyright 2016 - 2025 NXP Semiconductors

// =============================================================================
//! @file           fecu.c
//! @brief          FECU Low-Level Driver
//! @author         NXP Semiconductors
// =============================================================================

#include "fecu.h"

void fecuInit(void) {
    fecuClearPending();
    fecuClearError();
}
