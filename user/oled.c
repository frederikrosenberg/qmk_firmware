/* Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com> - inspiration
 * Copyright 2021 HellSingCoder - Luna animation
 * Copyright 2022 Frederik Rosenberg @frederikrosenberg - adaption
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "oled.h"
#include "user.h"
#include <string.h>


static void render_logos(void) {
    static const char PROGMEM logo[] = {
        0x20, 0x20, 0x20, 0x88, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x96, 0x97, 0x98, 0x20, 0x20, 0x20, 0x20, 10,
        0x20, 0x20, 0x20, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0x20, 0x20, 0x20, 0x20, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0x20, 10,
        0x20, 0x20, 0x20, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0x20, 0xCE, 0xCF, 0x20, 0xD6, 0xD7, 0x20, 0xD9, 0xDA, 0xDB, 0xDC, 10,     
        0x20, 0x20, 0x20, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x20, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x20, 10,
        0x20, 0x20, 0x20, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0x20, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0x20, 10,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xC3, 0xC4, 0x20, 0xC6, 0xC7, 0x20, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0x20, 0x20, 0
      };
    oled_write_P(logo, false);
}


uint32_t oled_sleep = 0;

// WPM-responsive animation stuff here
#define OLED_SIT_FRAMES 2
#define OLED_SIT_SPEED 10 // below this wpm value your animation will idle

#define OLED_WALK_FRAMES 2               // uncomment if >1
#define OLED_WALK_SPEED OLED_SIT_SPEED // below this wpm value your animation will idle

#define OLED_RUN_FRAMES 3
#define OLED_RUN_SPEED 40 // above this wpm value typing animation to triggere

#define OLED_BARK_FRAMES 2

#define OLED_SNEAK_FRAMES 2

#define OLED_ANIM_SIZE 32
#define OLED_ANIM_ROWS 3
#define OLED_ANIM_MAX_FRAMES 2

static uint8_t animation_frame = 0;
static uint8_t animation_type  = 0;

void render_luna(uint8_t col, uint8_t line) {
    // Credit to obosob for initial animation approach.

    // clang-format off
    static const char PROGMEM animation[5][OLED_ANIM_MAX_FRAMES][OLED_ANIM_ROWS][OLED_ANIM_SIZE] = {
        // sit frames
        {
            {
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1c, 0x02, 0x05, 0x02, 0x24, 0x04, 0x04, 0x02, 0xa9, 0x1e, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x08, 0x68, 0x10, 0x08, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x06, 0x82, 0x7c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0c, 0x10, 0x10, 0x20, 0x20, 0x20, 0x28, 0x3e, 0x1c, 0x20, 0x20, 0x3e, 0x0f, 0x11, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
            },
            {
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1c, 0x02, 0x05, 0x02, 0x24, 0x04, 0x04, 0x02, 0xa9, 0x1e, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x90, 0x08, 0x18, 0x60, 0x10, 0x08, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0x82, 0x7c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0c, 0x10, 0x10, 0x20, 0x20, 0x20, 0x28, 0x3e, 0x1c, 0x20, 0x20, 0x3e, 0x0f, 0x11, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
            }
        },
        // walk frames
        {
            {
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0x10, 0x90, 0x90, 0x90, 0xa0, 0xc0, 0x80, 0x80, 0x80, 0x70, 0x08, 0x14, 0x08, 0x90, 0x10, 0x10, 0x08, 0xa4, 0x78, 0x80, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x08, 0xfc, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x18, 0xea, 0x10, 0x0f, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1c, 0x20, 0x20, 0x3c, 0x0f, 0x11, 0x1f, 0x03, 0x06, 0x18, 0x20, 0x20, 0x3c, 0x0c, 0x12, 0x1e, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
            },
            {
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0x20, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x28, 0x10, 0x20, 0x20, 0x20, 0x10, 0x48, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x20, 0xf8, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x30, 0xd5, 0x20, 0x1f, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x20, 0x30, 0x0c, 0x02, 0x05, 0x09, 0x12, 0x1e, 0x02, 0x1c, 0x14, 0x08, 0x10, 0x20, 0x2c, 0x32, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
            }
        },
        // run frames
        {
            {
                { 0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x08, 0x08, 0xc8, 0xb0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x3c, 0x14, 0x04, 0x08, 0x90, 0x18, 0x04, 0x08, 0xb0, 0x40, 0x80, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0xc4, 0xa4, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc8, 0x58, 0x28, 0x2a, 0x10, 0x0f, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x09, 0x04, 0x04, 0x04, 0x04, 0x02, 0x03, 0x02, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x10, 0x26, 0x2b, 0x32, 0x04, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 },
            },
            {
                { 0x00, 0x00, 0x00, 0xe0, 0x10, 0x10, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x78, 0x28, 0x08, 0x10, 0x20, 0x30, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x03, 0x04, 0x08, 0x10, 0x11, 0xf9, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0xb0, 0x50, 0x55, 0x20, 0x1f, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x0c, 0x10, 0x20, 0x28, 0x37, 0x02, 0x1e, 0x20, 0x20, 0x18, 0x0c, 0x14, 0x1e, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
            },
        },
        // bark frames
        {
            {
                { 0x00, 0xc0, 0x20, 0x10, 0xd0, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40, 0x3c, 0x14, 0x04, 0x08, 0x90, 0x18, 0x04, 0x08, 0xb0, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x03, 0x04, 0x08, 0x10, 0x11, 0xf9, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc8, 0x48, 0x28, 0x2a, 0x10, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x0c, 0x10, 0x20, 0x28, 0x37, 0x02, 0x02, 0x04, 0x08, 0x10, 0x26, 0x2b, 0x32, 0x04, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
            },
            {
                { 0x00, 0xe0, 0x10, 0x10, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40, 0x40, 0x2c, 0x14, 0x04, 0x08, 0x90, 0x18, 0x04, 0x08, 0xb0, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x03, 0x04, 0x08, 0x10, 0x11, 0xf9, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0x48, 0x28, 0x2a, 0x10, 0x0f, 0x20, 0x4a, 0x09, 0x10 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x0c, 0x10, 0x20, 0x28, 0x37, 0x02, 0x02, 0x04, 0x08, 0x10, 0x26, 0x2b, 0x32, 0x04, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
            }
        },
        // sneak frames
        {
            {
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x40, 0x40, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x40, 0x40, 0x80, 0x00, 0x80, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x1e, 0x21, 0xf0, 0x04, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x04, 0x04, 0x04, 0x03, 0x01, 0x00, 0x00, 0x09, 0x01, 0x80, 0x80, 0xab, 0x04, 0xf8, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1c, 0x20, 0x20, 0x3c, 0x0f, 0x11, 0x1f, 0x02, 0x06, 0x18, 0x20, 0x20, 0x38, 0x08, 0x10, 0x18, 0x04, 0x04, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00 }
            },
            {
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x40, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xa0, 0x20, 0x40, 0x80, 0xc0, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x3e, 0x41, 0xf0, 0x04, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x04, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x40, 0x40, 0x55, 0x82, 0x7c, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x20, 0x30, 0x0c, 0x02, 0x05, 0x09, 0x12, 0x1e, 0x04, 0x18, 0x10, 0x08, 0x10, 0x20, 0x28, 0x34, 0x06, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 }
            }
        }
    };
    // clang-format on

    for (uint8_t i = 0; i < 3; i++) {
        oled_set_cursor(col, line + i);
        oled_write_raw_P(animation[animation_type][animation_frame][i], OLED_ANIM_SIZE);
    }
}

uint32_t kitty_animation_phases(uint32_t triger_time, void *cb_arg) {
    static uint32_t anim_frame_duration = 500;
#ifdef WPM_ENABLE
    int current_wpm = get_current_wpm();
#endif

    uint8_t mods = get_mods();
#ifndef NO_ACTION_ONESHOT
    mods |= get_oneshot_mods();
#endif
    if (mods & MOD_MASK_SHIFT || host_keyboard_led_state().caps_lock) {
        animation_frame     = (animation_frame + 1) % OLED_BARK_FRAMES;
        animation_type      = 3;
        anim_frame_duration = 200;
    } else if (mods & MOD_MASK_CAG) {
        animation_frame     = (animation_frame + 1) % OLED_SNEAK_FRAMES;
        animation_type      = 4;
        anim_frame_duration = 200;

    } else
    {
#ifdef WPM_ENABLE
        if (current_wpm <= OLED_SIT_SPEED) {
#endif
            animation_frame     = (animation_frame + 1) % OLED_SIT_FRAMES;
            animation_type      = 0;
            anim_frame_duration = 200;
#ifdef WPM_ENABLE
        } else if (current_wpm > OLED_WALK_SPEED && current_wpm < OLED_RUN_SPEED) {
            animation_frame     = (animation_frame + 1) % OLED_WALK_FRAMES;
            animation_type      = 1;
            anim_frame_duration = 200;
        } else if (current_wpm >= OLED_RUN_SPEED) {
            animation_frame     = (animation_frame + 1) % OLED_RUN_FRAMES;
            animation_type      = 2;
            anim_frame_duration = 200;
        }
#endif
    }


    return anim_frame_duration;
}


/**
 * @brief Renders the matrix scan rate to the host system
 *
 */
void render_matrix_scan_rate(uint8_t col, uint8_t line) {
#ifdef DEBUG_MATRIX_SCAN_RATE
    oled_set_cursor(col, line);
    oled_write_P(PSTR("MS:       "), false);
    oled_write(get_u16_str(get_matrix_scan_rate(), ' '), false);
#endif
}

void render_wpm(uint8_t col, uint8_t line) {
#ifdef WPM_ENABLE
    oled_set_cursor(col, line);
    oled_write_P(PSTR("WPM:      "), false);
    oled_write(get_u16_str(get_current_wpm(), ' '), false);
#endif
}

void render_mods(uint8_t col, uint8_t line) {
    static const char PROGMEM mods[4][3] = {{ 0x9A, 0x9B, 0 }, { 0x8D, 0x8E, 0 }, { 0x9E, 0x9F, 0 }, { 0x9C, 0x9D, 0 }};
    oled_set_cursor(col, line);
    oled_write_P(PSTR("MODS:  "), false);
    uint8_t modifiers = get_mods() | get_oneshot_mods();

    oled_write_P(mods[0], modifiers & MOD_MASK_ALT);
    oled_write_P(mods[2], modifiers & MOD_MASK_GUI);
    oled_write_P(mods[3], modifiers & MOD_MASK_SHIFT);
    oled_write_P(mods[1], modifiers & MOD_MASK_CTRL);

}

void render_layer(uint8_t col, uint8_t line) {
    static const char PROGMEM layers[4][15] = 
    {
        {
            0x20, 0xE0, 0xE1, 0xE2, 10,
            0x20, 0xE9, 0xEA, 0xEB, 10,
            0x20, 0xF5, 0xF6, 0xF7, 0
        },
        {
            0x20, 0xE3, 0xE4, 0xE5, 10,
            0x20, 0xEC, 0xED, 0xEE, 10,
            0x20, 0xF5, 0xF6, 0xF7, 0
        },
        {
            0x20, 0xE0, 0xE1, 0xE2, 10,
            0x20, 0xEF, 0xF0, 0xF1, 10,
            0x20, 0xF8, 0xF9, 0xFA, 0
        },
        {
            0x20, 0xE6, 0xE7, 0xE8, 10,
            0x20, 0xF2, 0xF3, 0xF4, 10,
            0x20, 0xFB, 0xFC, 0xFD, 0
        }
    };
    oled_set_cursor(col, line);
    uint8_t layer;
    switch (get_highest_layer(layer_state | default_layer_state)) {
        case _CANARY:
            layer = 0;
            break;
        case _SYM:
            layer = 1;
            break;
        case _NUM:
            layer = 2;
            break;
        case _NAV:
        default:
            layer = 3;
            break;
    }
    oled_write_P(layers[layer], false);
}

void render_layer_text(uint8_t col, uint8_t line) {
    oled_set_cursor(col, line);
    oled_write_P(PSTR("Layer: "), false);
    switch (get_highest_layer(layer_state | default_layer_state)) {
        case _CANARY:
            oled_write_P(PSTR("  Canary"), false);
            break;
        case _SYM:
            oled_write_P(PSTR(" Symbols"), false);
            break;
        case _NUM:
            oled_write_P(PSTR(" Numbers"), false);
            break;
        case _NAV:
            oled_write_P(PSTR("     Nav"), false);
            break;
        default:
            oled_write_P(PSTR(" Unknown"), false);
            break;
    }
}

void suspend_power_down_user(void) {
    oled_off();
}

void suspend_wakeup_init_user(void) {
    oled_on();
}

void render_master(void) {
    render_luna(0, 1);
    render_layer(0, 5);
    oled_set_cursor(6, 0);
    oled_write("     Kyria     ", true);
    render_matrix_scan_rate(6, 2);
    render_wpm(6, 3);
    render_mods(6, 4);
    render_layer_text(6, 5);
    oled_set_cursor(6, 6);
    oled_write_P(PSTR("Layout:  Canary"), false);
}

bool oled_task_user(void) {
    if (is_keyboard_master()) {
        if (timer_elapsed32(oled_sleep) > OLED_TIMEOUT) {
            oled_off();
        } else {
            oled_on();
        }
    }

    if (!is_oled_on()) return false;

    if (is_keyboard_master()) {
        render_master();
    } else {
        oled_set_cursor(0, 1);
        render_logos();
    }

    return false;
}

bool process_record_oled(uint16_t keycode, keyrecord_t *record) {
    oled_reset_timer();
    return true;
}


void oled_reset_timer(void) {
    oled_sleep = timer_read32();
}

deferred_token lunatoken;

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    lunatoken = defer_exec(3000, kitty_animation_phases, NULL);
    return OLED_ROTATION_180;
}


