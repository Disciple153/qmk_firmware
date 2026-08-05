// Copyright 2018-2022 Nick Brassel (@tzarc)
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "os_detection.h"

//----------------------------------------------------------
// Sync

#pragma pack(push)
#pragma pack(1)
typedef struct theme_runtime_config {
    uint32_t scan_rate;
    os_variant_t os_variant;
    uint8_t cpu_pct;
    uint8_t mem_pct;
    uint8_t hour;
    uint8_t minute;
} theme_runtime_config;
#pragma pack(pop)

enum custom_hid_ids {
    ID_STATS_UPDATE = 0x80,
    ID_RGB_STATE    = 0x81,
    ID_FFT_UPDATE   = 0x82,
};

extern theme_runtime_config theme_state;

void theme_init(void);
void theme_state_update(void);
void theme_state_sync(void);
