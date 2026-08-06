// !!! DO NOT ADD #pragma once !!! //

/* NOTES
Row 0 is at the top


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
RGB_MATRIX_EFFECT(test_effect)
RGB_MATRIX_EFFECT(my_cool_effect2)

// Step 2.
// Define effects inside the `RGB_MATRIX_CUSTOM_EFFECT_IMPLS` ifdef block
#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include "theme_djinn_default.h"

#define DIMMER 5
#define ROW_RANGE (256 / MATRIX_ROWS)

// e.g: A simple effect, self-contained within a single method
static bool audio_fft(effect_params_t* params) {
  uint8_t fft_col;
  uint8_t fft_row;
  uint8_t value;
  uint8_t lit_rows;

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

        value = value / DIMMER;

        rgb_matrix_set_color(led, value, value, value);
      }
    }
  }

  return false;
}

static bool test_effect(effect_params_t* params) {
  RGB_MATRIX_USE_LIMITS(led_min, led_max);
  uint8_t fft_col;

  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
      uint8_t led = g_led_config.matrix_co[row][col];

      if (led != NO_LED) {
        if (is_keyboard_left()) {
          fft_col = col;
        } else {
          fft_col = MAX_FFT_BANDS - 1 - col;
        }

        if (row == 0) {
          rgb_matrix_set_color(led, 50, 0, 0);
        } else if (row == 1) {
          rgb_matrix_set_color(led, 50, 50, 0);
        } else if (row == 2) {
          rgb_matrix_set_color(led, 0, 50, 0);
        } else if (row == 3) {
          rgb_matrix_set_color(led, 0, 50, 50);
        } else if (row == 4) {
          rgb_matrix_set_color(led, 0, 0, 50);
        } else {
          rgb_matrix_set_color(
            led,
            theme_state.fft_bands[fft_col] / DIMMER,
            theme_state.fft_bands[fft_col] / DIMMER,
            theme_state.fft_bands[fft_col] / DIMMER
          );
        }
      }
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
