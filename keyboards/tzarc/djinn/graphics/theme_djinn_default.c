// Copyright 2018-2022 Nick Brassel (@tzarc)
// SPDX-License-Identifier: GPL-2.0-or-later
#include <hal.h>
#include <string.h>
#include <ctype.h>
#include <printf.h>
#include "qp.h"
#include "backlight.h"
#include "transactions.h"
#include "split_util.h"

#include "djinn.h"
#include "theme_djinn_default.h"

//#include "nier-automata-yorha.qgf.h"
#include "monado-iii.qgf.h"
//#include "djinn.qgf.h"
#include "lock-caps-ON.qgf.h"
#include "lock-scrl-ON.qgf.h"
#include "lock-num-ON.qgf.h"
#include "lock-caps-OFF.qgf.h"
// #include "lock-scrl-OFF.qgf.h"
// #include "lock-num-OFF.qgf.h"
#include "icons/icons8-play-50.qgf.h"
#include "icons/icons8-end-50.qgf.h"
#include "icons/icons8-skip-to-start-50.qgf.h"
#include "icons/icons8-low-volume-50.qgf.h"
#include "icons/icons8-mute-50.qgf.h"
#include "icons/icons8-voice-50.qgf.h"
#include "thintel15.qff.h"

// LCD dimensions
#define LCD_WIDTH 240
#define LCD_HEIGHT 320
#define LCD_CENTER_X (LCD_WIDTH / 2)
#define LCD_CENTER_Y (LCD_HEIGHT / 2)

static painter_image_handle_t djinn_logo;
static painter_image_handle_t lock_caps_on;
static painter_image_handle_t lock_caps_off;
// static painter_image_handle_t lock_num_on;
// static painter_image_handle_t lock_num_off;
// static painter_image_handle_t lock_scrl_on;
// static painter_image_handle_t lock_scrl_off;
static painter_image_handle_t media_play;
static painter_image_handle_t media_next;
static painter_image_handle_t media_prev;
static painter_image_handle_t volume_down;
static painter_image_handle_t volume_mute;
static painter_image_handle_t volume_up;
static painter_font_handle_t  thintel;

//----------------------------------------------------------
// RGB Matrix naming
#if defined(RGB_MATRIX_ENABLE)
#    include <rgb_matrix.h>

#    if defined(RGB_MATRIX_EFFECT)
#        undef RGB_MATRIX_EFFECT
#    endif // defined(RGB_MATRIX_EFFECT)

#    define RGB_MATRIX_EFFECT(x) RGB_MATRIX_EFFECT_##x,
enum {
    RGB_MATRIX_EFFECT_NONE,
#    include "rgb_matrix_effects.inc"
#    undef RGB_MATRIX_EFFECT
#    ifdef RGB_MATRIX_CUSTOM_KB
#        include "rgb_matrix_kb.inc"
#    endif
#    ifdef RGB_MATRIX_CUSTOM_USER
#        include "rgb_matrix_user.inc"
#    endif
};

#    define RGB_MATRIX_EFFECT(x)    \
        case RGB_MATRIX_EFFECT_##x: \
            return #x;
const char *rgb_matrix_name(uint8_t effect) {
    switch (effect) {
        case RGB_MATRIX_EFFECT_NONE:
            return "NONE";
#    include "rgb_matrix_effects.inc"
#    undef RGB_MATRIX_EFFECT
#    ifdef RGB_MATRIX_CUSTOM_KB
#        include "rgb_matrix_kb.inc"
#    endif
#    ifdef RGB_MATRIX_CUSTOM_USER
#        include "rgb_matrix_user.inc"
#    endif
        default:
            return "UNKNOWN";
    }
}
#endif // defined(RGB_MATRIX_ENABLE)

//----------------------------------------------------------
// UI Initialisation
void keyboard_post_init_display(void) {
    djinn_logo    = qp_load_image_mem(gfx_monado_iii);
    thintel       = qp_load_font_mem(font_thintel15);

    if (is_keyboard_left()) {
        media_play = qp_load_image_mem(gfx_icons8_play_50);
        media_next = qp_load_image_mem(gfx_icons8_end_50);
        media_prev = qp_load_image_mem(gfx_icons8_skip_to_start_50);
    } else {
        lock_caps_on  = qp_load_image_mem(gfx_lock_caps_ON);
        lock_caps_off = qp_load_image_mem(gfx_lock_caps_OFF);
        volume_down   = qp_load_image_mem(gfx_icons8_low_volume_50);
        volume_mute   = qp_load_image_mem(gfx_icons8_mute_50);
        volume_up     = qp_load_image_mem(gfx_icons8_voice_50);
    }
}

//----------------------------------------------------------
// UI Helpers
int centered_x_offset(painter_image_handle_t image, int offset, int spacing) {
    return LCD_CENTER_X - (image->width / 2) + ((image->width + spacing) * offset);
}

//----------------------------------------------------------
// UI Drawing
void draw_ui_user(bool force_redraw) {
    bool            hue_redraw = force_redraw;
    bool            sat_redraw = force_redraw;
    static uint16_t last_hue   = 0xFFFF;
    static uint16_t last_sat   = 0xFFFF;
#if defined(RGB_MATRIX_ENABLE)
    uint16_t curr_hue = rgb_matrix_get_hue();
    uint16_t curr_sat = rgb_matrix_get_sat();
#else
    uint16_t curr_hue = 0;
    uint16_t curr_sat = 255;
#endif
    if (last_hue != curr_hue) {
        last_hue   = curr_hue;
        hue_redraw = true;
    }
    if (last_sat != curr_sat) {
        last_sat   = curr_sat;
        sat_redraw = true;
    }

    bool            layer_state_redraw = false;
    static uint32_t last_layer_state   = 0;
    if (last_layer_state != layer_state) {
        last_layer_state   = layer_state;
        layer_state_redraw = true;
    }

    bool            wpm_redraw      = false;
    static uint32_t last_wpm_update = 0;
    if (timer_elapsed32(last_wpm_update) > 125) {
        last_wpm_update = timer_read32();
        wpm_redraw      = true;
    }

#if defined(RGB_MATRIX_ENABLE)
    bool            rgb_effect_redraw = false;
    static uint16_t last_effect       = 0xFFFF;
    uint8_t         curr_effect       = rgb_matrix_config.mode;
    if (last_effect != curr_effect) {
        last_effect       = curr_effect;
        rgb_effect_redraw = true;
    }
#endif

    // Get current layer
    extern const char *current_layer_name(void);
    const char *layer_name = current_layer_name();
    uint8_t curr_layer = get_highest_layer(layer_state);

    // Show the Djinn logo and two vertical bars on both sides
    if (layer_state_redraw ||hue_redraw || sat_redraw) {
        qp_rect(lcd, 9, 32, 230, 319, curr_hue, curr_sat, 0, true);
        if (curr_layer == _QWERTY) {
            qp_drawimage(lcd, 120 - djinn_logo->width / 2, 32, djinn_logo);
        }
        qp_rect(lcd, 0, 0, 8, 319, curr_hue, curr_sat, 255, true);
        qp_rect(lcd, 231, 0, 239, 319, curr_hue, curr_sat, 255, true);
    }

    // LEFT DISPLAY
    if (is_keyboard_left()) {
        int icon_y = LCD_HEIGHT - media_play->height - 5;
        char buf[64] = {0};
        int  xpos    = 16;
        int  ypos    = 4;

        // Always show layer
        if (hue_redraw || sat_redraw || layer_state_redraw) {
            static int max_layer_xpos = 0;
            xpos = 16;
            snprintf(buf, sizeof(buf), "layer: %s", layer_name);
            xpos += qp_drawtext_recolor(lcd, xpos, ypos, thintel, buf, curr_hue, curr_sat, 255, curr_hue, curr_sat, 0);
            if (max_layer_xpos < xpos) max_layer_xpos = xpos;
            qp_rect(lcd, xpos, ypos, max_layer_xpos, ypos + thintel->line_height, 0, 0, 0, true);
        }
        ypos += thintel->line_height + 4;

        // Layer-specific content
        if (curr_layer == _QWERTY) {
            if (hue_redraw || sat_redraw || wpm_redraw) {
                static int max_wpm_xpos = 0;
                xpos = 16;
                snprintf(buf, sizeof(buf), "wpm: %d", (int)get_current_wpm());
                xpos += qp_drawtext_recolor(lcd, xpos, ypos, thintel, buf, curr_hue, curr_sat, 255, curr_hue, curr_sat, 0);
                if (max_wpm_xpos < xpos) max_wpm_xpos = xpos;
                qp_rect(lcd, xpos, ypos, max_wpm_xpos, ypos + thintel->line_height, 0, 0, 0, true);
            }
        } else if (curr_layer == _RGB) {
#if defined(RGB_MATRIX_ENABLE)
            if (hue_redraw || sat_redraw || rgb_effect_redraw) {
                static int max_rgb_xpos = 0;
                xpos = 16;
                snprintf(buf, sizeof(buf), "rgb: %s", rgb_matrix_name(curr_effect));
                for (int i = 5; i < sizeof(buf); ++i) {
                    if (buf[i] == 0) break;
                    else if (buf[i] == '_') buf[i] = ' ';
                    else if (buf[i - 1] == ' ') buf[i] = toupper(buf[i]);
                    else if (buf[i - 1] != ' ') buf[i] = tolower(buf[i]);
                }
                xpos += qp_drawtext_recolor(lcd, xpos, ypos, thintel, buf, curr_hue, curr_sat, 255, curr_hue, curr_sat, 0);
                if (max_rgb_xpos < xpos) max_rgb_xpos = xpos;
                qp_rect(lcd, xpos, ypos, max_rgb_xpos, ypos + thintel->line_height, 0, 0, 0, true);
            }
#endif
        }

        // Media control icons at bottom for _MEDIA and _RGB layers
        if (curr_layer == _MEDIA || curr_layer == _RGB) {
            if (hue_redraw || sat_redraw || layer_state_redraw) {
                qp_drawimage_recolor(lcd, centered_x_offset(media_prev, -1, 5), icon_y, media_prev, curr_hue, curr_sat, 0, curr_hue, curr_sat, 255);
                qp_drawimage_recolor(lcd, centered_x_offset(media_play, 0, 5), icon_y, media_play, curr_hue, curr_sat, 0, curr_hue, curr_sat, 255);
                qp_drawimage_recolor(lcd, centered_x_offset(media_next, 1, 5), icon_y, media_next, curr_hue, curr_sat, 0, curr_hue, curr_sat, 255);
            }
        }
    }

    // RIGHT DISPLAY
    if (!is_keyboard_left()) {
        int icon_y = LCD_HEIGHT - volume_mute->height - 5;

        // Layer-specific content
        if (curr_layer == _QWERTY) {
            static led_t last_led_state = {0};
            if (hue_redraw || sat_redraw || last_led_state.raw != host_keyboard_led_state().raw) {
                last_led_state.raw = host_keyboard_led_state().raw;
                qp_drawimage_recolor(lcd, 239 - 12 - 32, 0, last_led_state.caps_lock ? lock_caps_on : lock_caps_off, curr_hue, curr_sat, last_led_state.caps_lock ? 255 : 32, curr_hue, curr_sat, 0);
                // qp_drawimage_recolor(lcd, 239 - 12 - (32 * 2), 0, last_led_state.num_lock ? lock_num_on : lock_num_off, curr_hue, curr_sat, last_led_state.num_lock ? 255 : 32, curr_hue, curr_sat, 0);
                // qp_drawimage_recolor(lcd, 239 - 12 - (32 * 1), 0, last_led_state.scroll_lock ? lock_scrl_on : lock_scrl_off, curr_hue, curr_sat, last_led_state.scroll_lock ? 255 : 32, curr_hue, curr_sat, 0);
            }
        }

        // Volume control icons at bottom for _MEDIA and _RGB layers
        if (curr_layer == _MEDIA || curr_layer == _RGB) {
            if (hue_redraw || sat_redraw || layer_state_redraw) {
                qp_drawimage_recolor(lcd, centered_x_offset(volume_down, -1, 5), icon_y, volume_down, curr_hue, curr_sat, 0, curr_hue, curr_sat, 255);
                qp_drawimage_recolor(lcd, centered_x_offset(volume_mute, 0, 5), icon_y, volume_mute, curr_hue, curr_sat, 0, curr_hue, curr_sat, 255);
                qp_drawimage_recolor(lcd, centered_x_offset(volume_up, 1, 5), icon_y, volume_up, curr_hue, curr_sat, 0, curr_hue, curr_sat, 255);
            }
        }
    }
}

//----------------------------------------------------------
// Sync

theme_runtime_config theme_state;

void rpc_theme_sync_callback(uint8_t m2s_size, const void *m2s_buffer, uint8_t s2m_size, void *s2m_buffer) {
    if (m2s_size == sizeof(theme_state)) {
        memcpy(&theme_state, m2s_buffer, m2s_size);
    }
}

void theme_init(void) {
    // Register keyboard state sync split transaction
    transaction_register_rpc(THEME_DATA_SYNC, rpc_theme_sync_callback);

    // Reset the initial shared data value between master and slave
    memset(&theme_state, 0, sizeof(theme_state));
}

void theme_state_update(void) {
    if (is_keyboard_master()) {
        // Keep the scan rate in sync
        theme_state.scan_rate = get_matrix_scan_rate();
    }
}

void theme_state_sync(void) {
    if (!is_transport_connected()) return;

    if (is_keyboard_master()) {
        // Keep track of the last state, so that we can tell if we need to propagate to slave
        static theme_runtime_config last_theme_state;
        static uint32_t             last_sync;
        bool                        needs_sync = false;

        // Check if the state values are different
        if (memcmp(&theme_state, &last_theme_state, sizeof(theme_runtime_config))) {
            needs_sync = true;
            memcpy(&last_theme_state, &theme_state, sizeof(theme_runtime_config));
        }

        // Send to slave every 125ms regardless of state change
        if (timer_elapsed32(last_sync) > 125) {
            needs_sync = true;
        }

        // Perform the sync if requested
        if (needs_sync) {
            if (transaction_rpc_send(THEME_DATA_SYNC, sizeof(theme_runtime_config), &theme_state)) {
                last_sync = timer_read32();
            } else {
                dprint("Failed to perform rpc call\n");
            }
        }
    }
}
