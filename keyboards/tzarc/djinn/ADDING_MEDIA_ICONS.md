# Adding Media Control Icons to Djinn Display

## Step 1: Prepare PNG Images

Create or download 32x32 or 48x48 PNG images for:
- `media-prev.png` (previous track)
- `media-play.png` (play/pause)
- `media-next.png` (next track)

Place them in: `keyboards/tzarc/djinn/graphics/`

## Step 2: Convert to QMK Format

```bash
cd /home/disciple153/Documents/qmk_firmware

# Convert each image
qmk painter-convert-graphics -i keyboards/tzarc/djinn/graphics/media-prev.png -o keyboards/tzarc/djinn/graphics/media-prev -f mono2
qmk painter-convert-graphics -i keyboards/tzarc/djinn/graphics/media-play.png -o keyboards/tzarc/djinn/graphics/media-play -f mono2
qmk painter-convert-graphics -i keyboards/tzarc/djinn/graphics/media-next.png -o keyboards/tzarc/djinn/graphics/media-next -f mono2
```

This creates `.qgf.c` and `.qgf.h` files.

## Step 3: Add to theme_djinn_default.c

Add includes at top:
```c
#include "media-prev.qgf.h"
#include "media-play.qgf.h"
#include "media-next.qgf.h"
```

Add handles after line 32:
```c
static painter_image_handle_t media_prev;
static painter_image_handle_t media_play;
static painter_image_handle_t media_next;
```

Load in keyboard_post_init_display() around line 87:
```c
media_prev = qp_load_image_mem(gfx_media_prev);
media_play = qp_load_image_mem(gfx_media_play);
media_next = qp_load_image_mem(gfx_media_next);
```

## Step 4: Uncomment Drawing Code

In draw_ui_user(), uncomment the TODO sections and replace with:

For LEFT display (_MEDIA or _RGB layers):
```c
if (curr_layer == 1 || curr_layer == 2) {
    int icon_y = 280;  // Bottom of screen
    qp_drawimage_recolor(lcd, 20, icon_y, media_prev, curr_hue, curr_sat, 255, curr_hue, curr_sat, 0);
    qp_drawimage_recolor(lcd, 90, icon_y, media_play, curr_hue, curr_sat, 255, curr_hue, curr_sat, 0);
    qp_drawimage_recolor(lcd, 160, icon_y, media_next, curr_hue, curr_sat, 255, curr_hue, curr_sat, 0);
}
```

For RIGHT display (_MEDIA or _RGB layers):
```c
if (curr_layer == 1 || curr_layer == 2) {
    int icon_y = 280;
    qp_drawimage_recolor(lcd, 20, icon_y, media_prev, curr_hue, curr_sat, 255, curr_hue, curr_sat, 0);
    qp_drawimage_recolor(lcd, 90, icon_y, media_play, curr_hue, curr_sat, 255, curr_hue, curr_sat, 0);
    qp_drawimage_recolor(lcd, 160, icon_y, media_next, curr_hue, curr_sat, 255, curr_hue, curr_sat, 0);
}
```

## Current Status

The display code is now layer-aware:
- **All layers**: Show layer name
- **_QWERTY (0)**: Left shows WPM, Right shows Caps Lock
- **_MEDIA (1)**: Ready for media icons (add images)
- **_RGB (2)**: Left shows RGB effect, ready for media icons

Flash and test!
