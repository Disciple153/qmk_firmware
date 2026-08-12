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
#define ALBUM_COLORS_COUNT (MAX_FFT_BANDS / 2)

enum custom_hid_ids {
    ID_RGB_GET          = 0x80,
    ID_RGB_SET          = 0x81,
    ID_PC_STATS_UPDATE  = 0x82,
    ID_FFT_SET          = 0x83,
    ID_ALBUM_COLORS_SET = 0x84,
    ID_HOST_STRING      = 0x85,
    ID_MULTI_EFFECT_GET = 0x86,
    ID_MULTI_EFFECT_SET = 0x87
};

enum ColorSource {
    CS_VOID = 0xFF,
    CS_PRIMARY = 0x00,    ///< Every LED uses primary_color.
    CS_SECONDARY = 0x01,  ///< Every LED uses secondary_color.
    // CS_GRADIENT = 0x02,   ///< LEDs are sampled from gradient_colors_fg/bg via interpolation.
    CS_ALBUM = 0x03,      ///< LEDs are sampled from album_colors (derived from album art), via interpolation.
};

enum ColorEffect {
    CE_VOID = 0xFF,
    CE_NONE = 0x00,           ///< All weights are 1 (fully foreground).
    CE_SPECTRUM = 0x01,       ///< Weights follow live audio spectrum band levels received over MQTT.
    // CE_PULSE = 0x02,          ///< Weights follow a smooth cosine pulse across the whole strip.
};

enum PositionEffect {
    PE_VOID = 0xFF,
    PE_NONE = 0x00,    ///< Colors/weights are applied 1:1 to LED positions.
    PE_SCROLL = 0x01,  ///< Colors/weights are scrolled continuously along the strip over time.
};

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
    HSV secondary_color;
    uint8_t fft_band_count;
    uint8_t fft_bands[MAX_FFT_BANDS];
    bool fft_active;
    HSV album_colors[ALBUM_COLORS_COUNT];
    int album_colors_count;
    enum ColorSource color_source_fg;
    enum ColorSource color_source_bg;
    enum ColorEffect color_effect;
    enum PositionEffect position_effect;
    char host_string[20];
    int scroll_time_ds;
} theme_runtime_config;
#pragma pack(pop)

extern theme_runtime_config theme_state;

void theme_init(void);
void theme_state_update(void);
void theme_state_sync(void);
