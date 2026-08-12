// !!! DO NOT ADD #pragma once !!! //

/* NOTES
Row 0 is at the top

TODO:
- Implement reactive effects
- Display notifications on the LCD
- Customize the base color for FFT effects
- Display more Album Art colors
*/

#include "theme_djinn_default.h"
#include "rgb_matrix.h"
#include "stdint.h"
#include "sync_timer.h"
#include <math.h>

#define ROW_RANGE (256 / MATRIX_ROWS)
#define ROW_COUNT 5

static enum ColorSource color_source_fg = CS_VOID;
static enum ColorSource color_source_bg = CS_VOID;
static HSV cols_fg[MATRIX_COLS];
static HSV cols_bg[MATRIX_COLS];
static float cols_weight[MATRIX_COLS];
static HSV album_colors[MATRIX_COLS];

////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////

void update_color_source_(void);
void color_effect_none_(void);
void color_effect_spectrum_(void);
// void color_effect_pulse_(void);
void position_effect_none_(void);
void position_effect_scroll_(void);
void set_album_colors_(void);

////////////////////////////////////////////////////////////////////////////////
// EFFECTS
////////////////////////////////////////////////////////////////////////////////

static bool audio_fft(effect_params_t* params) {
  uint8_t fft_col, fft_row, value, lit_rows;
  HSV hsv = rgb_matrix_config.hsv;
  HSV fft_hsv = {hsv.h, hsv.s, hsv.v};
  RGB rgb;

  for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
    uint8_t flags = g_led_config.flags[i];

    if (flags & LED_FLAG_UNDERGLOW) {
      rgb = hsv_to_rgb(fft_hsv);
      rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
  }

  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
      uint8_t led = g_led_config.matrix_co[row][col];

      if (led != NO_LED) {
        if (is_keyboard_left()) {
          fft_col = col;
        } else {
          fft_col = MAX_FFT_BANDS - 1 - col;
        }

        fft_row = MATRIX_ROWS - 1 - row;
        value = theme_state.fft_bands[fft_col];

        lit_rows = value / ROW_RANGE;

        if (fft_row > lit_rows || !theme_state.fft_active) {
          value = 0;
        } else if (fft_row == lit_rows) {
          value = (value % ROW_RANGE) * 255 / ROW_RANGE;
        } else {
          value = 255;
        }

        fft_hsv.v = ((value * hsv.v / 255) + hsv.v) / 2;
        rgb = hsv_to_rgb(fft_hsv);
        rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
      }
    }
  }

  return false;
}

static bool audio_fft_album(effect_params_t* params) {
    uint8_t fft_col, fft_row, value, lit_rows;
    HSV hsv = rgb_matrix_config.hsv;
    HSV album_hsv = theme_state.album_colors[0];
    HSV fft_hsv = {
        album_hsv.h,
        album_hsv.s,
        hsv.v
    };
    RGB rgb;

    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        uint8_t flags = g_led_config.flags[i];

        if (flags & LED_FLAG_UNDERGLOW) {
            rgb = hsv_to_rgb(fft_hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];

            if (led != NO_LED) {
                if (is_keyboard_left()) {
                    fft_col = col;
                } else {
                    fft_col = MAX_FFT_BANDS - 1 - col;
                }

                fft_row = MATRIX_ROWS - 1 - row;
                value = theme_state.fft_bands[fft_col];

                lit_rows = value / ROW_RANGE;

                if (fft_row > lit_rows || !theme_state.fft_active) {
                    value = 0;
                } else if (fft_row == lit_rows) {
                    value = (value % ROW_RANGE) * 255 / ROW_RANGE;
                } else {
                    value = 255;
                }

                fft_hsv.v = ((value * hsv.v / 255) + hsv.v) / 2;
                rgb = hsv_to_rgb(fft_hsv);
                rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
            }
        }
    }

  return false;
}

// static uint8_t some_global_state;
// static void my_cool_effect2_complex_init(effect_params_t* params) {
//   some_global_state = 1;
// }
// static bool my_cool_effect2_complex_run(effect_params_t* params) {
//   RGB_MATRIX_USE_LIMITS(led_min, led_max);
//   for (uint8_t i = led_min; i < led_max; i++) {
//     rgb_matrix_set_color(i, 0xff, some_global_state++, 0xff);
//   }
//   return rgb_matrix_check_finished_leds(led_max);
// }
// static bool my_cool_effect2(effect_params_t* params) {
//   if (params->init) my_cool_effect2_complex_init(params);
//   return my_cool_effect2_complex_run(params);
// }

////////////////////////////////////////////////////////////////////////////////
// MULTI EFFECT
////////////////////////////////////////////////////////////////////////////////

static bool multi_effect(effect_params_t* params) {

    update_color_source_();

    switch (theme_state.color_effect) {
        case CE_NONE:
            color_effect_none_();
            break;

        case CE_SPECTRUM:
            color_effect_spectrum_();
            break;

        // case CE_PULSE:
        //     color_effect_pulse_();
        //     break;

        default:
            break;
    }

    switch (theme_state.position_effect) {
        case PE_NONE:
            position_effect_none_();
            break;

        case PE_SCROLL:
            position_effect_scroll_();
            break;

        default:
            break;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
// UTILITIES
////////////////////////////////////////////////////////////////////////////////

HSV get_sub_pixel_(int strip_index, HSV *colors, int color_count) {
    int color_index_low, color_index_high;
    float color_index, color_index_decimal;
    HSV color_low, color_high;

    // No palette to sample from — return black rather than dividing by zero.
    if (color_count == 0) return (HSV) {0,0,0};

    color_index = ((float) strip_index / MATRIX_COLS) * color_count;
    color_index_decimal = color_index - floor(color_index);
    color_index_low = floor(color_index);
    color_index_high = (int) ceil(color_index) % color_count;

    color_low = colors[color_index_low];
    color_high = colors[color_index_high];

    // Linearly interpolate each channel between the two stops.
    return (HSV) {
        (uint8_t) (color_low.h * (1 - color_index_decimal) + color_high.h * color_index_decimal),
        (uint8_t) (color_low.s * (1 - color_index_decimal) + color_high.s * color_index_decimal),
        (uint8_t) (color_low.v * (1 - color_index_decimal) + color_high.v * color_index_decimal)
    };
}

void set_leds_with_gradient_(HSV *colors, int color_count, HSV *out) {
    for (int i = 0; i < MATRIX_COLS; i++) {
        out[i] = get_sub_pixel_(i, colors, color_count);
    }
}

void set_album_colors_() {
    HSV color;
    uint8_t value = rgb_matrix_config.hsv.v;

    for (int i = 0; i < theme_state.album_colors_count; i += 2) {
        color = theme_state.album_colors[i];
        color.v = value;

        album_colors[i] = color;
        album_colors[i + 1] = color;
    }
}

HSV rgb_to_hsv_(RGB rgb) {
    HSV hsv;
    uint8_t rgb_min = MIN(MIN(rgb.r, rgb.g), rgb.b);
    uint8_t rgb_max = MAX(MAX(rgb.r, rgb.g), rgb.b);
    uint8_t delta   = rgb_max - rgb_min;

    hsv.v = rgb_max;

    if (delta == 0) {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = (uint8_t)(255 * (uint16_t)delta / rgb_max);

    int16_t hue;
    if (rgb_max == rgb.r) {
        hue = 43 * ((int16_t)rgb.g - rgb.b) / delta;
    } else if (rgb_max == rgb.g) {
        hue = 85 + 43 * ((int16_t)rgb.b - rgb.r) / delta;
    } else {
        hue = 171 + 43 * ((int16_t)rgb.r - rgb.g) / delta;
    }

    if (hue < 0) hue += 255;
    hsv.h = (uint8_t)hue;

    return hsv;
}

RGB lerp_rgb_(RGB a, RGB b, float t) {
    RGB result;

    if (t < 0) {
        t = 0;
    } else if (t > 1) {
        t = 1;
    }

    result.r = a.r + (b.r - a.r) * t;
    result.g = a.g + (b.g - a.g) * t;
    result.b = a.b + (b.b - a.b) * t;

    return result;
}

HSV lerp_hsv_(HSV a, HSV b, float t) {
    RGB rgb_a = hsv_to_rgb(a);
    RGB rgb_b = hsv_to_rgb(b);
    RGB rgb_result = lerp_rgb_(rgb_a, rgb_b, t);
    return rgb_to_hsv_(rgb_result);
}

////////////////////////////////////////////////////////////////////////////////
// COLOR SOURCE
////////////////////////////////////////////////////////////////////////////////

bool cmp_hsv(HSV a, HSV b) {
    return a.h == b.h && a.s == b.s && a.v == b.v;
}

void update_color_source_() {
    if (
        theme_state.color_source_fg != color_source_fg ||
        (color_source_fg == CS_ALBUM && cmp_hsv(cols_fg[0], theme_state.album_colors[0])) ||
        (color_source_fg == CS_PRIMARY && cmp_hsv(cols_fg[0], rgb_matrix_config.hsv)) ||
        (color_source_fg == CS_SECONDARY && cmp_hsv(cols_fg[0], theme_state.secondary_color))
    ) {

        color_source_fg = theme_state.color_source_fg;

        switch (color_source_fg) {
            case CS_PRIMARY:
                for (int i = 0; i < MATRIX_COLS; i++) {
                    cols_fg[i] = rgb_matrix_config.hsv;
                }
                break;

            case CS_SECONDARY:
                for (int i = 0; i < MATRIX_COLS; i++) {
                    cols_fg[i] = (HSV) {
                        theme_state.secondary_color.h,
                        theme_state.secondary_color.s,
                        theme_state.secondary_color.v
                    };
                }
                break;

            case CS_ALBUM:
                set_album_colors_();
                set_leds_with_gradient_(
                    album_colors,
                    theme_state.album_colors_count,
                    cols_fg
                );
                break;

            // case CS_GRADIENT:
            //   set_leds_with_gradient_(gradient_colors_fg, cols_fg);
            //   break;

            default:
                break;
        }
    }

    if (
        theme_state.color_source_bg != color_source_bg ||
        (color_source_bg == CS_ALBUM  && cmp_hsv(cols_bg[0], theme_state.album_colors[0])) ||
        (color_source_bg == CS_PRIMARY && cmp_hsv(cols_bg[0], rgb_matrix_config.hsv)) ||
        (color_source_bg == CS_SECONDARY && cmp_hsv(cols_bg[0], theme_state.secondary_color))
    ) {
        color_source_bg = theme_state.color_source_bg;

        switch (color_source_bg) {
            case CS_PRIMARY:
                for (int i = 0; i < MATRIX_COLS; i++) {
                    cols_bg[i] = rgb_matrix_config.hsv;
                }
                break;

            case CS_SECONDARY:
                for (int i = 0; i < MATRIX_COLS; i++) {
                    cols_bg[i] = (HSV) {
                        theme_state.secondary_color.h,
                        theme_state.secondary_color.s,
                        theme_state.secondary_color.v
                    };
                }
                break;

            case CS_ALBUM:
                set_album_colors_();
                set_leds_with_gradient_(
                    album_colors,
                    theme_state.album_colors_count,
                    cols_bg
                );
                break;

            // case CS_GRADIENT:
            //   set_leds_with_gradient_(gradient_colors_bg, cols_bg);
            //   break;

            default:
                break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// COLOR EFFECTS
////////////////////////////////////////////////////////////////////////////////

void color_effect_none_() {
  for (int i = 0; i < MATRIX_COLS; i++) {
    cols_weight[i] = 1;
  }
}

void color_effect_spectrum_() {
    uint8_t fft_col;

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {

        if (is_keyboard_left()) {
            fft_col = col;
        } else {
            fft_col = MAX_FFT_BANDS - 1 - col;
        }

        if (!theme_state.fft_active) {
            cols_weight[col] = 0;
        } else {
            cols_weight[col] = theme_state.fft_bands[fft_col] / 255.0f;
        }
    }
}

// void color_effect_pulse_() {
//   int timer_ms;
//   float pulse_progress, pulse_progress_full;

//   pulse_progress_full = (((float) millis() * MATRIX_COLS) / (pulse_time_ms * 100));
//   pulse_progress = pulse_progress_full - floor(pulse_progress_full);

//   if (pulse_time_ms == 0) pulse_time_ms = 1;
//   timer_ms = millis() % pulse_time_ms;

//   float level = (cos(2 * M_PI * pulse_progress) + 1) / 2.0f;

//   for (int i = 0; i < MATRIX_COLS; i++) strip_weight[i] = level;
// }

////////////////////////////////////////////////////////////////////////////////
// POSITION EFFECTS
////////////////////////////////////////////////////////////////////////////////

void position_effect_none_() {
    uint8_t fft_row, rows_begin, rows_end;
    float value;
    RGB rgb;
    HSV fft_hsv = {
        cols_fg[0].h,
        cols_fg[0].s,
        rgb_matrix_config.hsv.v
    };

    // TODO: There must be a better way than this nonsense.
    if (is_keyboard_left()) {
        rows_end = ROW_COUNT;
    } else {
        rows_end = MATRIX_ROWS - 1;
    }

    rows_begin = rows_end - ROW_COUNT;

    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        uint8_t flags = g_led_config.flags[i];

        if (flags & LED_FLAG_UNDERGLOW) {
            rgb = hsv_to_rgb(fft_hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }

    for (uint8_t row = rows_begin; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];

            if (led != NO_LED) {
                fft_row = rows_end - 1 - row;

                value = cols_weight[col];

                value = (value * (float) ROW_COUNT) - (float) fft_row;
                if (value < 0) {
                    value = 0;
                } else if (value > 1) {
                    value = 1;
                }

                rgb = lerp_rgb_(
                    hsv_to_rgb(cols_bg[col]),
                    hsv_to_rgb(cols_fg[col]),
                    value
                );
                rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
            }
        }
    }
}

void position_effect_scroll_() {
    uint8_t fft_row, rows_begin, rows_end;
    float value;
    RGB rgb;
    HSV fft_hsv = {
        cols_fg[0].h,
        cols_fg[0].s,
        rgb_matrix_config.hsv.v
    };

    // TODO: There must be a better way than this nonsense.
    if (is_keyboard_left()) {
        rows_end = ROW_COUNT;
    } else {
        rows_end = MATRIX_ROWS - 1;
    }

    rows_begin = rows_end - ROW_COUNT;

    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        uint8_t flags = g_led_config.flags[i];

        if (flags & LED_FLAG_UNDERGLOW) {
            rgb = hsv_to_rgb(fft_hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }

    float scroll_progress, scroll_progress_full;
    float index, index_decimal;
    int index_low, index_high;
    RGB fg, bg;

    scroll_progress_full = (((float) sync_timer_read32() * MATRIX_COLS) / (theme_state.scroll_time_ds * 100));
    scroll_progress = scroll_progress_full - floor(scroll_progress_full);

    index = scroll_progress * MATRIX_COLS;
    index_decimal = index - floor(index);


    for (uint8_t row = rows_begin; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];

            if (led != NO_LED) {
                fft_row = rows_end - 1 - row;

                value = cols_weight[col];

                value = (value * (float) ROW_COUNT) - (float) fft_row;
                if (value < 0) {
                    value = 0;
                } else if (value > 1) {
                    value = 1;
                }

                index_low = (col + (int) floor(index)) % MATRIX_COLS;
                index_high = (index_low + 1) % MATRIX_COLS;

                fg = lerp_rgb_(
                    hsv_to_rgb(cols_fg[index_low]),
                    hsv_to_rgb(cols_fg[index_high]),
                    index_decimal
                );

                bg = lerp_rgb_(
                    hsv_to_rgb(cols_bg[index_low]),
                    hsv_to_rgb(cols_bg[index_high]),
                    index_decimal
                );

                rgb = lerp_rgb_(bg, fg, value );
                rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
            }
        }
    }
}
