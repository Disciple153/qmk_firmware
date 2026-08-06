// !!! DO NOT ADD #pragma once !!! //

/* NOTES
Row 0 is at the top

TODO:
- Interpolate FFT updates
- Get color(s) from album art
- Implement reactive effects
- Remove FFT logging from display
- Display custom "os" string when connected to the hid-bridge service
- Only display time, CPU, and RAM when connected to the hid-bridge service
- Increase keyboard sleep timeout
- Get this file to play nicely in the editor, or just move the effects to a separate file
*/

// #ifndef RGB_MATRIX_EFFECT
// #define RGB_MATRIX_EFFECT(x)
// #endif
// #ifndef RGB_MATRIX_CUSTOM_EFFECT_IMPLS
// #define RGB_MATRIX_CUSTOM_EFFECT_IMPLS(x)
// #endif
// #include "rgb_matrix.h"

// Step 1.
// Declare custom effects using the RGB_MATRIX_EFFECT macro
// (note the lack of semicolon after the macro!)
RGB_MATRIX_EFFECT(audio_fft)
RGB_MATRIX_EFFECT(my_cool_effect2)

// Step 2.
// Define effects inside the `RGB_MATRIX_CUSTOM_EFFECT_IMPLS` ifdef block
#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include "theme_djinn_default.h"

#define DIMMER 5
#define ROW_RANGE (256 / MATRIX_ROWS)

// e.g: A simple effect, self-contained within a single method
static bool audio_fft(effect_params_t* params) {
  uint8_t fft_col, fft_row, value, lit_rows;
  HSV hsv = rgb_matrix_config.hsv;
  HSV fft_hsv = {hsv.h, hsv.s, hsv.v};
  RGB rgb;

  // Key LEDs
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

        // Get the bottom row which should be lit
        lit_rows = value / ROW_RANGE;

        if (fft_row > lit_rows) {
          // Leave the top row unlit
          value = 0;
        } else if (fft_row == lit_rows) {
          // Scale the brightness of the top lit row
          value = (value % ROW_RANGE) * 255 / ROW_RANGE;
        } else {
          // Fully light the rows below the top lit row
          value = 255;
        }

        fft_hsv.v = ((value * hsv.v / 255) + hsv.v) / 2;
        rgb = hsv_to_rgb(fft_hsv);
        rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
      }
    }
  }

  // Underglow LEDs
  for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
    uint8_t flags = g_led_config.flags[i];

    if (flags & LED_FLAG_UNDERGLOW) {
      rgb = hsv_to_rgb(hsv);
      rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
  }

  return false;
}

// e.g: A more complex effect, relying on external methods and state, with
// dedicated init and run methods
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

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
