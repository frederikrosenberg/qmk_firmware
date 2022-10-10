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

#include QMK_KEYBOARD_H

#include "keymap_danish.h"

enum layers {
    _CANARY,
    _HANDSDOWN,
    _NUM,
    _NAV,
    _SYM,
    _SHORTCUTS,
};

enum my_keycodes {
    CASEWRD = SAFE_RANGE,
    OS_ALT,
    OS_MOD,
    OS_SFT,
    OS_CTR,
    TOGBASE,
    QUOTE
};

#define NAV MO(_NAV)
#define NUM MO(_NUM)

// Thumbs
#define SPC_SYM LT(_SYM, KC_SPC)

// HANDS DOWN
#define ALT_R   ALT_T(DK_R)
#define ALT_A   ALT_T(DK_A)
#define GUI_S   GUI_T(DK_S)
#define GUI_I   GUI_T(DK_I)
#define SFT_N   SFT_T(DK_N)
#define SFT_E   SFT_T(DK_E)
#define MCTL_T  CTL_T(DK_T)
#define CTL_U   CTL_T(DK_U)

// General
#define UNDO    C(DK_Z)
#define CUT     C(DK_X)
#define COPY    C(DK_C)
#define PASTE   C(DK_V)
#define SAVE    C(DK_S)

#define C_A_DEL C(A(KC_DEL))
#define C_BSPC  C(KC_BSPC)

// Window movement
#define WM_LEFT G(C(KC_LEFT))
#define WM_RGHT G(C(KC_RGHT))
