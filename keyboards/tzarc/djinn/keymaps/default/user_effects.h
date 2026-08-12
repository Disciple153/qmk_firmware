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

static enum ColorSource color_source_fg = CS_PRIMARY;
static enum ColorSource color_source_bg = CS_PRIMARY;
static HSV cols_fg[MATRIX_COLS];
static HSV cols_bg[MATRIX_COLS];
static RGB cols_rgb[MATRIX_COLS];
static float cols_weight[MATRIX_COLS];
static HSV album_colors[MATRIX_COLS * 2];

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

    // if (theme_state.color_source_fg != color_source_fg || theme_state.color_source_bg != color_source_bg) {
    //     color_source_fg = theme_state.color_source_fg;
    //     color_source_bg = theme_state.color_source_bg;
    //     update_color_source_();
    // }

    color_source_fg = theme_state.color_source_fg;
    color_source_bg = theme_state.color_source_bg;
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
    }

    switch (theme_state.position_effect) {
        case PE_NONE:
            position_effect_none_();
            break;

        case PE_SCROLL:
            position_effect_scroll_();
            break;
    }

    // if (invert_direction) {
    //     for (size_t i = 0; i < MATRIX_COLS / 2; i++) {
    //     tmp_color = led_strip[i].get();

    //     led_strip[i] = led_strip[MATRIX_COLS - 1 - i].get();
    //     led_strip[MATRIX_COLS - 1 - i] = tmp_color;
    //     }
    // }

    // RGB rgb;

    // for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
    //     uint8_t flags = g_led_config.flags[i];

    //     if (flags & LED_FLAG_UNDERGLOW) {
    //         rgb = hsv_to_rgb(cols_fg[i % 7]);
    //         rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    //     }
    // }

    // for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    //     for (uint8_t col = 0; col < MATRIX_COLS; col++) {
    //         uint8_t led = g_led_config.matrix_co[row][col];

    //         if (led != NO_LED) {
    //             rgb = cols_rgb[col];
    //             rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
    //             // if (row == 0) {
    //             //     rgb = cols_rgb[col];
    //             //     rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
    //             // } else if (row == 1) {
    //             //     rgb = hsv_to_rgb(cols_fg[col]);
    //             //     rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
    //             // } else if (row == 2) {
    //             //     rgb = hsv_to_rgb(cols_bg[col]);
    //             //     rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
    //             // } else if (row == 3) {
    //             //     rgb = hsv_to_rgb(album_colors[col]);
    //             //     rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
    //             // } else {
    //             //     rgb = cols_rgb[col];
    //             //     rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
    //             // }
    //         }

    //     }
    // }

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

    for (int i = 0; i < theme_state.album_colors_count * 2; i += 2) {
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

    // result.r = a.r + (b.r - a.r) * t;
    // result.g = a.g + (b.g - a.g) * t;
    // result.b = a.b + (b.b - a.b) * t;

    result.r = (a.r * t) + (b.r * (1 - t));
    result.g = (a.g * t) + (b.g * (1 - t));
    result.b = (a.b * t) + (b.b * (1 - t));

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

void update_color_source_() {

    switch (color_source_fg) {
        case CS_PRIMARY:
            for (int i = 0; i < MATRIX_COLS; i++) {
                cols_fg[i] = rgb_matrix_config.hsv;
            }
            break;

        case CS_SECONDARY:
            for (int i = 0; i < MATRIX_COLS; i++) {
                cols_fg[i] = (HSV) {
                    rgb_matrix_config.hsv.h,
                    rgb_matrix_config.hsv.s,
                    rgb_matrix_config.hsv.v / 2
                };
            }
            break;

        case CS_ALBUM:
            set_album_colors_();
            set_leds_with_gradient_(
                album_colors,
                theme_state.album_colors_count * 2,
                cols_fg
            );
            break;

        // case CS_GRADIENT:
        //   set_leds_with_gradient_(gradient_colors_fg, cols_fg);
        //   break;
    }

    switch (color_source_bg) {
        case CS_PRIMARY:
            for (int i = 0; i < MATRIX_COLS; i++) {
                cols_bg[i] = rgb_matrix_config.hsv;
            }
            break;

        case CS_SECONDARY:
            for (int i = 0; i < MATRIX_COLS; i++) {
                cols_bg[i] = (HSV) {
                    rgb_matrix_config.hsv.h,
                    rgb_matrix_config.hsv.s,
                    rgb_matrix_config.hsv.v / 2
                };
            }
            break;

        case CS_ALBUM:
            set_album_colors_();
            set_leds_with_gradient_(
                album_colors,
                theme_state.album_colors_count * 2,
                cols_bg
            );
            break;

        // case CS_GRADIENT:
        //   set_leds_with_gradient_(gradient_colors_bg, cols_bg);
        //   break;
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
            cols_weight[fft_col] = 0;
        } else {
            cols_weight[fft_col] = theme_state.fft_bands[fft_col] / 255.0f;
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
    // for (int i = 0; i < MATRIX_COLS; i++) {
    //     cols_rgb[i] = lerp_rgb_(
    //         hsv_to_rgb(cols_bg[i]),
    //         hsv_to_rgb(cols_fg[i]),
    //         cols_weight[i]
    //     );
    // }

    uint8_t fft_row;
    float value;
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

    int row_count = 5;
    int rows_end;

    // TODO: There must be a better way than this nonsense.
    if (is_keyboard_left()) {
        rows_end = row_count;
    } else {
        rows_end = MATRIX_ROWS - 1;
    }

    int rows_begin = rows_end - row_count;

    for (uint8_t row = rows_begin; row < rows_end; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];

            if (led != NO_LED) {
                fft_row = rows_end - 1 - row;

                value = cols_weight[col];

                // if (fft_row == 0) {
                //     rgb_matrix_set_color(led, 50, 0, 0);
                // } else if (fft_row == 1) {
                //     rgb_matrix_set_color(led, 50, 50, 0);
                // } else if (fft_row == 2) {
                //     rgb_matrix_set_color(led, 0, 50, 0);
                // } else if (fft_row == 3) {
                //     rgb_matrix_set_color(led, 0, 50, 50);
                // } else if (fft_row == 4) {
                //     rgb_matrix_set_color(led, 0, 0, 50);
                // } else {
                //     rgb_matrix_set_color(led, 50, 0, 50);
                // }

                value = (value * (float) row_count) - (float) fft_row;
                if (value < 0 || !theme_state.fft_active) {
                    value = 0;
                } else if (value > 1) {
                    value = 1;
                }

                // fft_hsv.v = ((value * hsv.v / 255) + hsv.v) / 2;
                // rgb = hsv_to_rgb(fft_hsv);
                rgb  = lerp_rgb_(
                    hsv_to_rgb(cols_bg[led]),
                    hsv_to_rgb(cols_fg[led]),
                    value
                );
                // rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);

                if (value > 1) {
                    rgb_matrix_set_color(led, 50, 0, 50);
                } else if (value < 0) {
                    rgb_matrix_set_color(led, 0, 0, 50);
                } else {

                    rgb_matrix_set_color(led,
                        (1 - value) * 40,
                        value * 40,
                        0
                    );
                }
            }
        }
    }
}

void position_effect_scroll_() {
  float scroll_progress, scroll_progress_full;
  float index, index_decimal;
  int index_low, index_high;
  RGB fg, bg;

  scroll_progress_full = (((float) sync_timer_read32() * MATRIX_COLS) / (theme_state.scroll_time_ms * 100));
  scroll_progress = scroll_progress_full - floor(scroll_progress_full);

  index = scroll_progress * MATRIX_COLS;
  index_decimal = index - floor(index);

  for (int i = 0; i < MATRIX_COLS; i++) {
    index_low = (i + (int) floor(index)) % MATRIX_COLS;
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

    cols_rgb[i] = lerp_rgb_(bg, fg, cols_weight[i] );
  }
}
