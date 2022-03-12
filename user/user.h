#pragma once

#include QMK_KEYBOARD_H

#include "keymap_danish.h"

enum layers {
    _HANDSDOWN,
    _LOWER,
    _RAISE,
    _ADJUST,
    _SHORTCUTS,
};

enum my_keycodes {
    CASEWRD = SAFE_RANGE,
    TOGBASE,
    QUOTE,
    FINDALL,
    ALTTAB,
    ALTSTAB,
    COLSEMI,
    DOTAND,
    SLASHS,
    DQQUST,
    SQEXLM,
    COMPIP,
    MIPLUS
};

// Thumbs
#define RAI_ENT LT(_RAISE, KC_ENT)
#define CUT_SPC LT(_SHORTCUTS, KC_SPC)
#define LOW_BPC LT(_LOWER, KC_BSPC)
#define ADJ_TAB LT(_ADJUST, KC_TAB)


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
#define FIND    C(DK_F)
#define SFIND   C(S(DK_F))
#define REPLACE C(DK_R)
#define SALL    C(DK_A)

#define C_A_DEL C(A(KC_DEL))
#define C_BSPC  C(KC_BSPC)

// Window movement
#define WM_LEFT G(C(KC_LEFT))
#define WM_RGHT G(C(KC_RGHT))
