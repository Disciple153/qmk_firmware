// Copyright 2018-2022 Nick Brassel (@tzarc)
// SPDX-License-Identifier: GPL-2.0-or-later

/* NOTES
Max image size: 240*320

*/

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

#include "nier-automata-yorha.qgf.h"
#include "monado-iii.qgf.h"
#include "djinn-media-left.qgf.h"
#include "djinn-media-right.qgf.h"
#include "djinn-rgb-left.qgf.h"
#include "djinn-rgb-right.qgf.h"
//#include "djinn.qgf.h"
#include "lock-caps-ON.qgf.h"
#include "lock-scrl-ON.qgf.h"
#include "lock-num-ON.qgf.h"
#include "lock-caps-OFF.qgf.h"
#include "lock-scrl-OFF.qgf.h"
#include "lock-num-OFF.qgf.h"
#include "thintel15.qff.h"

static painter_image_handle_t monado_iii;
static painter_image_handle_t yorha;
static painter_image_handle_t media_left;
static painter_image_handle_t media_right;
static painter_image_handle_t rgb_left;
static painter_image_handle_t rgb_right;
static painter_image_handle_t lock_caps_on;
static painter_image_handle_t lock_caps_off;
static painter_image_handle_t lock_num_on;
static painter_image_handle_t lock_num_off;
static painter_image_handle_t lock_scrl_on;
static painter_image_handle_t lock_scrl_off;
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
    monado_iii    = qp_load_image_mem(gfx_monado_iii);
    yorha         = qp_load_image_mem(gfx_nier_automata_yorha);
    media_left    = qp_load_image_mem(gfx_djinn_media_left);
    media_right   = qp_load_image_mem(gfx_djinn_media_right);
    rgb_left      = qp_load_image_mem(gfx_djinn_rgb_left);
    rgb_right     = qp_load_image_mem(gfx_djinn_rgb_right);
    //djinn_logo    = qp_load_image_mem(gfx_djinn);
    lock_caps_on  = qp_load_image_mem(gfx_lock_caps_ON);
    lock_caps_off = qp_load_image_mem(gfx_lock_caps_OFF);
    lock_num_on   = qp_load_image_mem(gfx_lock_num_ON);
    lock_num_off  = qp_load_image_mem(gfx_lock_num_OFF);
    lock_scrl_on  = qp_load_image_mem(gfx_lock_scrl_ON);
    lock_scrl_off = qp_load_image_mem(gfx_lock_scrl_OFF);
    thintel       = qp_load_font_mem(font_thintel15);
}

void draw_ln(int xpos, int* ypos, int* prev_xpos, bool updated, char* buf, uint8_t hue, uint8_t sat) {
    if (updated) {
        xpos += qp_drawtext_recolor(lcd, xpos, *ypos, thintel, buf, hue, sat, 255, hue, sat, 0);
        if (*prev_xpos > xpos) {
            qp_rect(lcd, xpos, *ypos, *prev_xpos, *ypos + thintel->line_height, 0, 0, 0, true);
        }
        *prev_xpos = xpos;
    }

    *ypos += thintel->line_height + 4;
}

void draw_data(
    int layer,
    uint8_t hue, uint8_t sat, uint8_t effect, uint8_t led_b, uint8_t lcd_b, uint8_t speed, uint8_t wpm,
    bool layer_state_redraw, bool wpm_redraw, bool power_state_redraw, bool rgb_effect_redraw, bool scan_redraw, bool led_b_redraw, bool lcd_b_redraw, bool speed_redraw
) {
    if (!is_keyboard_left()) {
        return;
    }

    char buf[64] = {0};
    int  xpos    = 16;
    int  ypos    = 4;

    bool updated;

    // LINE 1
    static int prev_line_1_xpos = 0;

    updated = layer_state_redraw;
    xpos = 16;
    buf[0] = 0;

    if (layer_state_redraw) {
        extern const char *current_layer_name(void);
        const char        *layer_name = current_layer_name();
        snprintf(buf, sizeof(buf), "layer: %s", layer_name);
        updated = true;
    }

    draw_ln(xpos, &ypos, &prev_line_1_xpos, updated, buf, hue, sat);

    // LINE 2
    static int prev_line_2_xpos = 0;

    updated = layer_state_redraw;
    xpos = 16;
    buf[0] = 0;

    switch (layer) {
        case _QWERTY:
            if (layer_state_redraw || wpm_redraw) {
                snprintf(buf, sizeof(buf), "wpm: %d", wpm);
                updated = true;
            }
            break;

        case _ADJUST:
            if (layer_state_redraw || power_state_redraw) {
                snprintf(buf, sizeof(buf), "power: %s", usbpd_str(kb_state.current_setting));
                updated = true;
            }
            break;

#if defined(RGB_MATRIX_ENABLE)
        case _RGB:
            if (layer_state_redraw || rgb_effect_redraw) {
                snprintf(buf, sizeof(buf), "effect: %s", rgb_matrix_name(effect));

                for (int i = 5; i < sizeof(buf); ++i) {
                    if (buf[i] == 0)
                        break;
                    else if (buf[i] == '_')
                        buf[i] = ' ';
                    else if (buf[i - 1] == ' ')
                        buf[i] = toupper(buf[i]);
                    else if (buf[i - 1] != ' ')
                        buf[i] = tolower(buf[i]);
                }

                updated = true;
            }
            break;
#endif // defined(RGB_MATRIX_ENABLE)
    }

    draw_ln(xpos, &ypos, &prev_line_2_xpos, updated, buf, hue, sat);

    // LINE 3
    static int prev_line_3_xpos = 0;

    updated = layer_state_redraw;
    xpos = 16;
    buf[0] = 0;

    switch (layer) {
        case _ADJUST:
            if (layer_state_redraw || scan_redraw) {
                snprintf(buf, sizeof(buf), "scans: %d", (int)theme_state.scan_rate);
                updated = true;
            }
            break;

#if defined(RGB_MATRIX_ENABLE)
        case _RGB:
            if (layer_state_redraw) {
                snprintf(buf, sizeof(buf), "hue: %d", hue);
                updated = true;
            }
            break;
#endif // defined(RGB_MATRIX_ENABLE)
    }

    draw_ln(xpos, &ypos, &prev_line_3_xpos, updated, buf, hue, sat);

    // LINE 4
    static int prev_line_4_xpos = 0;

    updated = layer_state_redraw;
    xpos = 16;
    buf[0] = 0;

    switch (layer) {
#if defined(RGB_MATRIX_ENABLE)
        case _RGB:
            if (layer_state_redraw) {
                snprintf(buf, sizeof(buf), "saturation: %d", sat);
                updated = true;
            }
            break;
#endif // defined(RGB_MATRIX_ENABLE)
    }

    draw_ln(xpos, &ypos, &prev_line_4_xpos, updated, buf, hue, sat);

    // LINE 5
    static int prev_line_5_xpos = 0;

    updated = layer_state_redraw;
    xpos = 16;
    buf[0] = 0;

    switch (layer) {
#if defined(RGB_MATRIX_ENABLE)
        case _RGB:
            if (layer_state_redraw || led_b_redraw) {
                snprintf(buf, sizeof(buf), "led brightness: %d", led_b);
                updated = true;
            }
            break;
#endif // defined(RGB_MATRIX_ENABLE)
    }

    draw_ln(xpos, &ypos, &prev_line_5_xpos, updated, buf, hue, sat);

    // LINE 6
    static int prev_line_6_xpos = 0;

    updated = layer_state_redraw;
    xpos = 16;
    buf[0] = 0;

    switch (layer) {
#if defined(RGB_MATRIX_ENABLE)
        case _RGB:
            if (layer_state_redraw || lcd_b_redraw) {
                snprintf(buf, sizeof(buf), "lcd brightness: %d", lcd_b);
                updated = true;
            }
            break;
#endif // defined(RGB_MATRIX_ENABLE)
    }

    draw_ln(xpos, &ypos, &prev_line_6_xpos, updated, buf, hue, sat);

    // LINE 7
    static int prev_line_7_xpos = 0;

    updated = layer_state_redraw;
    xpos = 16;
    buf[0] = 0;

    switch (layer) {
#if defined(RGB_MATRIX_ENABLE)
        case _RGB:
            if (layer_state_redraw || speed_redraw) {
                snprintf(buf, sizeof(buf), "speed: %d", speed);
                updated = true;
            }
            break;
#endif // defined(RGB_MATRIX_ENABLE)
    }

    draw_ln(xpos, &ypos, &prev_line_7_xpos, updated, buf, hue, sat);
}

void draw_bg(int layer, int hue, int sat) {
        static int prev_left = 0;
        static int prev_right = 0;
        static int prev_top = 0;
        static int prev_bottom = 0;

        painter_image_handle_t image;
        int top = 0;

        switch (layer) {
            case _RGB:
                top = 155;

                if (is_keyboard_left()) {
                    image = rgb_left;
                }
                else {
                    image = rgb_right;
                }
                break;

            case _MEDIA:
                top = 240;

                if (is_keyboard_left()) {
                    image = media_left;
                }
                else {
                    image = media_right;
                }
                break;

            case _ADJUST:
                image = yorha;
                break;

            default:
                image = monado_iii;
                break;
        }

        int left = 120 - image->width / 2;
        int right = left + image->width;
        int bottom = top + image->height;

        qp_drawimage(lcd, left, top, image);

        if (prev_left < left) {
            qp_rect(lcd, prev_left, prev_top, left - 1, prev_bottom, 0, 0, 0, true);
        }

        if (prev_right > right) {
            qp_rect(lcd, right, prev_top, prev_right, prev_bottom, 0, 0, 0, true);
        }

        if (prev_top < top) {
            qp_rect(lcd, prev_left, prev_top, prev_right, top - 1, 0, 0, 0, true);
        }

        if (prev_bottom > bottom) {
            qp_rect(lcd, prev_left, bottom, prev_right, prev_bottom, 0, 0, 0, true);
        }

        prev_left = left;
        prev_right = right;
        prev_top = top;
        prev_bottom = bottom;

        qp_rect(lcd, 0, 0, 8, 319, hue, sat, 255, true);
        qp_rect(lcd, 231, 0, 239, 319, hue, sat, 255, true);
}

//----------------------------------------------------------
// UI Drawing
void draw_ui_user(bool force_redraw) {
    bool            hue_redraw = force_redraw;
    bool            sat_redraw = force_redraw;
    bool            led_b_redraw = force_redraw;
    bool            lcd_b_redraw = force_redraw;
    bool            speed_redraw = force_redraw;
    static uint16_t last_hue   = 0xFFFF;
    static uint16_t last_sat   = 0xFFFF;
    static uint16_t last_led_b   = 0xFFFF;
    static uint16_t last_lcd_b   = 0xFFFF;
    static uint16_t last_speed   = 0xFFFF;

    int layer = get_highest_layer(layer_state);

#if defined(RGB_MATRIX_ENABLE)
    uint16_t curr_hue = rgb_matrix_get_hue();
    uint16_t curr_sat = rgb_matrix_get_sat();
    uint16_t curr_led_b = rgb_matrix_get_val();
    uint16_t curr_lcd_b = get_backlight_level();
    uint16_t curr_speed = rgb_matrix_get_speed();
    uint16_t curr_wpm = get_current_wpm();

#else  // defined(RGB_MATRIX_ENABLE)
    uint16_t curr_hue = 0;
    uint16_t curr_sat = 255;
#endif // defined(RGB_MATRIX_ENABLE)
    if (last_hue != curr_hue) {
        last_hue   = curr_hue;
        hue_redraw = true;
    }

    if (last_sat != curr_sat) {
        last_sat   = curr_sat;
        sat_redraw = true;
    }

    if (last_led_b != curr_led_b) {
        last_led_b   = curr_led_b;
        led_b_redraw = true;
    }

    if (last_lcd_b != curr_lcd_b) {
        last_lcd_b   = curr_lcd_b;
        lcd_b_redraw = true;
    }

    if (last_speed != curr_speed) {
        last_speed   = curr_speed;
        speed_redraw = true;
    }

    bool            layer_state_redraw = false;
    static uint32_t last_layer_state   = 0;
    if (last_layer_state != layer_state) {
        last_layer_state   = layer_state;
        layer_state_redraw = true;
    }

    bool                     power_state_redraw = false;
    static usbpd_allowance_t last_current_state = (usbpd_allowance_t)(~0);
    if (last_current_state != kb_state.current_setting) {
        last_current_state = kb_state.current_setting;
        power_state_redraw = true;
    }

    bool            scan_redraw      = false;
    static uint32_t last_scan_update = 0;
    if (timer_elapsed32(last_scan_update) > 125) {
        last_scan_update = timer_read32();
        scan_redraw      = true;
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
#endif // defined(RGB_MATRIX_ENABLE)

    layer_state_redraw = layer_state_redraw || hue_redraw || sat_redraw;

    // Draw background
    if (layer_state_redraw) {
        draw_data(layer,
            curr_hue, curr_sat, curr_effect, curr_led_b, curr_lcd_b, curr_speed, curr_wpm,
            layer_state_redraw, wpm_redraw, power_state_redraw, rgb_effect_redraw,
            scan_redraw, led_b_redraw, lcd_b_redraw, speed_redraw
        );
        draw_bg(layer, curr_hue, curr_sat);
    }

    draw_data(layer,
        curr_hue, curr_sat, curr_effect, curr_led_b, curr_lcd_b, curr_speed, curr_wpm,
        layer_state_redraw, wpm_redraw, power_state_redraw, rgb_effect_redraw,
        scan_redraw, led_b_redraw, lcd_b_redraw, speed_redraw
    );

    // Show LED lock indicators on the right side
    if (!is_keyboard_left()) {
        static led_t last_led_state = {0};
        if (hue_redraw || sat_redraw || last_led_state.raw != host_keyboard_led_state().raw) {
            last_led_state.raw = host_keyboard_led_state().raw;
            qp_drawimage_recolor(lcd, 239 - 12 - 32, (32 * 0), last_led_state.caps_lock ? lock_caps_on : lock_caps_off, curr_hue, curr_sat, last_led_state.caps_lock ? 255 : 32, curr_hue, curr_sat, 0);
            // qp_drawimage_recolor(lcd, 239 - 12 - 32, (32 * 1), last_led_state.num_lock ? lock_num_on : lock_num_off, curr_hue, curr_sat, last_led_state.num_lock ? 255 : 32, curr_hue, curr_sat, 0);
            // qp_drawimage_recolor(lcd, 239 - 12 - 32, (32 * 2), last_led_state.scroll_lock ? lock_scrl_on : lock_scrl_off, curr_hue, curr_sat, last_led_state.scroll_lock ? 255 : 32, curr_hue, curr_sat, 0);
        }
    }

    // On layer change to qwerty, draw background last
    if (layer_state_redraw && layer != _RGB) {
        draw_bg(layer, curr_hue, curr_sat);
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
