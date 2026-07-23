// Copyright 2018-2023 Nick Brassel (@tzarc)
// SPDX-License-Identifier: GPL-2.0-or-later
#include "djinn.h"
#include <string.h>
#include <hal_pal.h>
#include "serial.h"
#include "split_util.h"

painter_device_t lcd;

// clang-format off
#ifdef SWAP_HANDS_ENABLE
const keypos_t PROGMEM hand_swap_config[MATRIX_ROWS][MATRIX_COLS] = {
   { { 6,  6 }, { 5,  6 }, { 4,  6 }, { 3,  6 }, { 2,  6 }, { 1,  6 }, { 0,  6 } },
   { { 6,  7 }, { 5,  7 }, { 4,  7 }, { 3,  7 }, { 2,  7 }, { 1,  7 }, { 0,  7 } },
   { { 6,  8 }, { 5,  8 }, { 4,  8 }, { 3,  8 }, { 2,  8 }, { 1,  8 }, { 0,  8 } },
   { { 6,  9 }, { 5,  9 }, { 4,  9 }, { 3,  9 }, { 2,  9 }, { 1,  9 }, { 0,  9 } },
   { { 0,  0 }, { 0,  0 }, { 0,  0 }, { 6, 10 }, { 5, 10 }, { 4, 10 }, { 3, 10 } },
   { { 0,  0 }, { 6, 11 }, { 5, 11 }, { 4, 11 }, { 3, 11 }, { 2, 11 }, { 1, 11 } },

   { { 6,  0 }, { 5,  0 }, { 4,  0 }, { 3,  0 }, { 2,  0 }, { 1,  0 }, { 0,  0 } },
   { { 6,  1 }, { 5,  1 }, { 4,  1 }, { 3,  1 }, { 2,  1 }, { 1,  1 }, { 0,  1 } },
   { { 6,  2 }, { 5,  2 }, { 4,  2 }, { 3,  2 }, { 2,  2 }, { 1,  2 }, { 0,  2 } },
   { { 6,  3 }, { 5,  3 }, { 4,  3 }, { 3,  3 }, { 2,  3 }, { 1,  3 }, { 0,  3 } },
   { { 0,  0 }, { 0,  0 }, { 0,  0 }, { 6,  4 }, { 5,  4 }, { 4,  4 }, { 3,  4 } },
   { { 0,  0 }, { 6,  5 }, { 5,  5 }, { 4,  5 }, { 3,  5 }, { 2,  5 }, { 1,  5 } },
};
#    ifdef ENCODER_MAP_ENABLE
const uint8_t PROGMEM encoder_hand_swap_config[NUM_ENCODERS] = { 1, 0 };
#    endif // ENCODER_MAP_ENABLE
#endif // SWAP_HANDS_ENABLE
// clang-format on

void board_init(void) {
    usbpd_init();
}

// Forward declaration -- weak definition lives further down in this file,
// but djinn_lcd_power_on() (below) needs to call it on re-init.
void draw_ui_user(bool force_redraw);

//----------------------------------------------------------
// LCD power management
//
// NOTE: Historically this sequence only ever ran once, from
// keyboard_post_init_kb(). RGB_POWER_ENABLE_PIN is re-driven every
// housekeeping tick (self-healing against brief power glitches -- e.g.
// a KVM switch bouncing VBUS), but the LCD enable pin and the ILI9341
// init sequence were not, so a transient droop on the LCD's supply rail
// (without a full MCU reset) would leave the panel permanently blank
// until a physical unplug/replug re-ran post_init. This function makes
// LCD bring-up re-runnable so it can be called again from housekeeping,
// from a detected power fault, and from suspend/resume.

static bool lcd_device_created = false;

static void djinn_lcd_power_on(bool full_reinit) {
    // Turn on the LCD's power rail
    gpio_set_pin_output(LCD_POWER_ENABLE_PIN);
    gpio_write_pin_high(LCD_POWER_ENABLE_PIN);

    // Let the LCD get some power...
    wait_ms(150);

    if (!lcd_device_created) {
        // Only construct the device object once -- re-running qp_init() below
        // is what actually re-runs the ILI9341 hardware reset + init command
        // sequence, which is what's needed after a real power-rail glitch.
        lcd                 = qp_ili9341_make_spi_device(240, 320, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, 4, 0);
        lcd_device_created  = true;
        full_reinit         = true;
    }

    if (full_reinit) {
        qp_init(lcd, QP_ROTATION_0);
        qp_power(lcd, true);
        qp_rect(lcd, 0, 0, 239, 319, HSV_BLACK, true);
        draw_ui_user(true);
    } else {
        qp_power(lcd, true);
    }

    // Turn on the LCD backlight
    backlight_enable();
    backlight_level(BACKLIGHT_LEVELS);
}

static void djinn_lcd_power_off(void) {
    if (lcd_device_created) {
        qp_power(lcd, false);
    }
    backlight_disable();
    gpio_write_pin_low(LCD_POWER_ENABLE_PIN);
}

// Debounced monitor for the board/RGB power-fault comparator outputs.
// These pins were previously defined in config.h but never read anywhere
// in the firmware, so a hardware fault event (e.g. an under-voltage or
// over-current blip caused by a KVM switch) was invisible to the firmware
// and nothing ever recovered from it automatically.
static bool djinn_power_fault_seen(void) {
    static bool     fault_latched = false;
    static uint32_t last_check    = 0;

    if (timer_elapsed32(last_check) < 10) {
        return false; // rate-limit the GPIO reads
    }
    last_check = timer_read32();

#if defined(BOARD_POWER_FAULT_PIN) || defined(RGB_POWER_FAULT_PIN)
    bool fault_now = false;
#    ifdef BOARD_POWER_FAULT_PIN
    fault_now |= !gpio_read_pin(BOARD_POWER_FAULT_PIN); // active-low fault comparator output
#    endif
#    ifdef RGB_POWER_FAULT_PIN
    fault_now |= !gpio_read_pin(RGB_POWER_FAULT_PIN);
#    endif

    if (fault_now && !fault_latched) {
        fault_latched = true;
        return true; // rising edge into fault -- caller should recover
    }
    if (!fault_now) {
        fault_latched = false;
    }
#endif
    return false;
}

//----------------------------------------------------------
// Initialisation

void keyboard_post_init_kb(void) {
    // Register keyboard state sync split transaction
    transaction_register_rpc(RPC_ID_SYNC_STATE_KB, kb_state_sync_slave);

    // Reset the initial shared data value between master and slave
    memset(&kb_state, 0, sizeof(kb_state));

    // Turn off increased current limits
    gpio_set_pin_output(RGB_CURR_1500mA_OK_PIN);
    gpio_write_pin_low(RGB_CURR_1500mA_OK_PIN);
    gpio_set_pin_output(RGB_CURR_3000mA_OK_PIN);
    gpio_write_pin_low(RGB_CURR_3000mA_OK_PIN);

    // Turn on the RGB
    gpio_set_pin_output(RGB_POWER_ENABLE_PIN);
    gpio_write_pin_high(RGB_POWER_ENABLE_PIN);

#ifdef EXTERNAL_FLASH_SPI_SLAVE_SELECT_PIN
    gpio_set_pin_output(EXTERNAL_FLASH_SPI_SLAVE_SELECT_PIN);
    gpio_write_pin_high(EXTERNAL_FLASH_SPI_SLAVE_SELECT_PIN);
#endif // EXTERNAL_FLASH_SPI_SLAVE_SELECT_PIN

#if defined(BOARD_POWER_FAULT_PIN)
    gpio_set_pin_input(BOARD_POWER_FAULT_PIN);
#endif
#if defined(RGB_POWER_FAULT_PIN)
    gpio_set_pin_input(RGB_POWER_FAULT_PIN);
#endif

    // Turn on the LCD (full init)
    djinn_lcd_power_on(true);

    // Allow for user post-init
    keyboard_post_init_user();
}

//----------------------------------------------------------
// Suspend / resume (e.g. KVM-triggered USB suspend)
//
// Djinn previously had no suspend hooks at all, so QMK's generic suspend
// handling (which only knows about OLED/RGB/backlight by default) left
// the LCD completely unmanaged across a suspend/resume cycle.

void suspend_power_down_kb(void) {
    djinn_lcd_power_off();
    suspend_power_down_user();
}

void suspend_wakeup_init_kb(void) {
    djinn_lcd_power_on(true);
    suspend_wakeup_init_user();
}

//----------------------------------------------------------
// RGB brightness scaling dependent on USBPD state

#if defined(RGB_MATRIX_ENABLE)
rgb_t rgb_matrix_hsv_to_rgb(hsv_t hsv) {
    float scale;

#    ifdef DJINN_SUPPORTS_3A_FUSE
    // The updated BOM on the Djinn has properly-spec'ed fuses -- 1500mA/3000mA hold current
    switch (kb_state.current_setting) {
        default:
        case USBPD_500MA:
            scale = 0.35f;
            break;
        case USBPD_1500MA:
            scale = 0.75f;
            break;
        case USBPD_3000MA:
            scale = 1.0f;
            break;
    }
#    else
    // The original BOM on the Djinn had wrongly-spec'ed fuses -- 750mA/1500mA hold current
    switch (kb_state.current_setting) {
        default:
        case USBPD_500MA:
        case USBPD_1500MA:
            scale = 0.35f;
            break;
        case USBPD_3000MA:
            scale = 0.75f;
            break;
    }
#    endif

    hsv.v = (uint8_t)(hsv.v * scale);
    return hsv_to_rgb(hsv);
}
#endif

//----------------------------------------------------------
// UI Placeholder, implemented in themes

__attribute__((weak)) void draw_ui_user(bool force_redraw) {}

//----------------------------------------------------------
// Housekeeping

void housekeeping_task_kb(void) {
    // Update kb_state so we can send to slave
    kb_state_update();

    // Data sync from master to slave
    kb_state_sync();

    // Work out if we've changed our current limit, update the limiter circuit switches
    static uint8_t current_setting = USBPD_500MA;
    if (current_setting != kb_state.current_setting) {
        current_setting = kb_state.current_setting;

#ifdef DJINN_SUPPORTS_3A_FUSE
        // The updated BOM on the Djinn has properly-spec'ed fuses -- 1500mA/3000mA hold current
        switch (current_setting) {
            default:
            case USBPD_500MA:
                gpio_write_pin_low(RGB_CURR_1500mA_OK_PIN);
                gpio_write_pin_low(RGB_CURR_3000mA_OK_PIN);
                break;
            case USBPD_1500MA:
                gpio_write_pin_high(RGB_CURR_1500mA_OK_PIN);
                gpio_write_pin_low(RGB_CURR_3000mA_OK_PIN);
                break;
            case USBPD_3000MA:
                gpio_write_pin_high(RGB_CURR_1500mA_OK_PIN);
                gpio_write_pin_high(RGB_CURR_3000mA_OK_PIN);
                break;
        }
#else
        // The original BOM on the Djinn had wrongly-spec'ed fuses -- 750mA/1500mA hold current
        switch (current_setting) {
            default:
            case USBPD_500MA:
            case USBPD_1500MA:
                gpio_write_pin_low(RGB_CURR_1500mA_OK_PIN);
                gpio_write_pin_low(RGB_CURR_3000mA_OK_PIN);
                break;
            case USBPD_3000MA:
                gpio_write_pin_high(RGB_CURR_1500mA_OK_PIN);
                gpio_write_pin_low(RGB_CURR_3000mA_OK_PIN);
                break;
        }
#endif

        // If we've changed the current limit, toggle rgb off and on if it was on, to force a brightness update on all LEDs
        if (is_keyboard_master() && rgb_matrix_is_enabled()) {
            rgb_matrix_disable_noeeprom();
            rgb_matrix_enable_noeeprom();
        }
    }

    // Turn on/off the LCD
    bool peripherals_on = last_input_activity_elapsed() < LCD_ACTIVITY_TIMEOUT;

    // Enable/disable RGB
    if (peripherals_on) {
        // Turn on RGB
        gpio_write_pin_high(RGB_POWER_ENABLE_PIN);
        // Modify the RGB state if different to the LCD state
        if (rgb_matrix_is_enabled() != peripherals_on) {
            // Wait for a small amount of time to allow the RGB capacitors to charge, before enabling RGB output
            wait_ms(10);
            // Enable RGB
            rgb_matrix_enable_noeeprom();
        }
    } else {
        // Turn off RGB
        gpio_write_pin_low(RGB_POWER_ENABLE_PIN);
        // Disable the PWM output for the RGB
        if (rgb_matrix_is_enabled() != peripherals_on) {
            rgb_matrix_disable_noeeprom();
        }
    }

    // Self-heal the LCD the same way RGB already does above: if a power
    // fault was detected (e.g. a KVM switch glitching VBUS/the board's
    // power rail) do a full re-init rather than assuming the panel
    // survived. This is the fix for "screen doesn't come back until I
    // unplug/replug" -- previously nothing ever re-ran LCD bring-up after
    // the very first boot.
    if (djinn_power_fault_seen()) {
        djinn_lcd_power_on(true);
    } else if (peripherals_on) {
        // Cheaply re-assert the enable pin every tick, same as RGB above.
        // This alone won't recover a controller that lost its internal
        // state, but it does recover a pin/latch that got left low.
        gpio_write_pin_high(LCD_POWER_ENABLE_PIN);
    }

    // Match the backlight to the LCD state
    if (is_keyboard_master() && is_backlight_enabled() != peripherals_on) {
        if (peripherals_on)
            backlight_enable();
        else
            backlight_disable();
    }

    // Draw the UI
    if (peripherals_on) {
        draw_ui_user(false);
    }

    // Go into low-scan interrupt-based mode if we haven't had any matrix activity in the last 250 milliseconds
    if (last_input_activity_elapsed() > 250) {
        matrix_wait_for_interrupt();
    }
}
