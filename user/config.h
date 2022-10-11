/* Copyright 2022 Frederik Rosenberg @frederikrosenberg
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

#pragma once

#define LAYER_STATE_8BIT

#define IGNORE_MOD_TAP_INTERRUPT
#define TAPPING_TERM 140
#define TAPPING_FORCE_HOLD_PER_KEY
#define COMBO_ALLOW_ACTION_KEYS
#define COMBO_VARIABLE_LEN
#define COMBO_TERM 50


#ifdef OLED_ENABLE
  #define OLED_DISPLAY_128X64
  #define SPLIT_OLED_ENABLE
  #define OLED_TIMEOUT 30000
  #define OLED_UPDATE_INTERVAL 100
  #define OLED_FONT_H "font.h"
  #define OLED_FONT_END 255
  #define OLED_BRIGHTNESS 64
  #define DEBUG_MATRIX_SCAN_RATE
  #define SPLIT_LAYER_STATE_ENABLE
#endif

#define NO_ACTION_MACRO
#define NO_ACTION_FUNCTION
#define DISABLE_LEADER
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE
