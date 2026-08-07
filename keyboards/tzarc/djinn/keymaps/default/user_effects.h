// !!! DO NOT ADD #pragma once !!! //

/* NOTES
Row 0 is at the top

TODO:
- Interpolate FFT updates
- Implement reactive effects
- Display notifications on the LCD
- Turn off FFT effect when updates are not being received
- Customize the base color for FFT effects
*/

#include "theme_djinn_default.h"
#include "rgb_matrix.h"
#include "stdint.h"

#define DIMMER 5
#define ROW_RANGE (256 / MATRIX_ROWS)

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

        if (fft_row > lit_rows) {
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

        if (fft_row > lit_rows) {
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

static uint8_t some_global_state;
static void my_cool_effect2_complex_init(effect_params_t* params) {
  some_global_state = 1;
}
static bool my_cool_effect2_complex_run(effect_params_t* params) {
  RGB_MATRIX_USE_LIMITS(led_min, led_max);
  for (uint8_t i = led_min; i < led_max; i++) {
    rgb_matrix_set_color(i, 0xff, some_global_state++, 0xff);
  }
  return rgb_matrix_check_finished_leds(led_max);
}
static bool my_cool_effect2(effect_params_t* params) {
  if (params->init) my_cool_effect2_complex_init(params);
  return my_cool_effect2_complex_run(params);
}
