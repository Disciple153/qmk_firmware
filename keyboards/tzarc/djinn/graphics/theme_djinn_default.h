// Copyright 2018-2022 Nick Brassel (@tzarc)
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "os_detection.h"
#include "color.h"

// LCD dimensions
#define LCD_WIDTH 240
#define LCD_HEIGHT 320
#define LCD_CENTER_X (LCD_WIDTH / 2)
#define LCD_CENTER_Y (LCD_HEIGHT / 2)
#define LCD_WIDTH_I (LCD_WIDTH - 1)
#define LCD_HEIGHT_I (LCD_HEIGHT - 1)
#define BAR_WIDTH 8
#define MARGIN 9
#define MARGIN_R (LCD_WIDTH_I - MARGIN)
#define TEXT_MARGIN 16

#define MAX_FFT_BANDS 14

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
    uint8_t fft_band_count;
    uint8_t fft_bands[14];
    HSV album_colors[7];
    char host_string[20];
} theme_runtime_config;
#pragma pack(pop)

enum custom_hid_ids {
    ID_RGB_GET          = 0x80,
    ID_RGB_SET          = 0x81,
    ID_PC_STATS_UPDATE  = 0x82,
    ID_FFT_SET          = 0x83,
    ID_ALBUM_COLORS_SET = 0x84,
    ID_HOST_STRING      = 0x85
};

extern theme_runtime_config theme_state;

void theme_init(void);
void theme_state_update(void);
void theme_state_sync(void);
