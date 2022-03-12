#include "user.h"
#include "features/casemodes.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_HANDSDOWN] = LAYOUT_USER(
        //.--------+--------+--------+--------+--------.  .--------+--------+--------+--------+--------.
             DK_Q,    DK_C,    DK_H,    DK_P,    DK_V,       DK_K,    DK_Y,    DK_O,    DK_J,    QUOTE,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
             ALT_R,  GUI_S,   SFT_N,   MCTL_T,   DK_G,       DK_W,    CTL_U,   SFT_E,   GUI_I,   ALT_A,
        //|--------+--------+--------+--------+--------|  |+--------+--------+--------+--------+-------|
             DK_X,    DK_M,   DK_L,     DK_D,    DK_B,       DK_Z,    DK_F,  DK_COMM,  DK_DOT, DK_MINS,
        //.--------+--------+--------+--------+--------|  |+--------+--------+--------+--------+-------.
                                      RAI_ENT, CUT_SPC,     LOW_BPC, ADJ_TAB 
        //                           .--------+--------|  |+--------+--------.
    ),

/*
 * Lower Layer: Symbols
 */
    [_LOWER] = LAYOUT_USER(
        //.--------+--------+--------+--------+--------.  .--------+--------+--------+--------+--------.
           DK_GRV , DK_AT,   DK_LCBR, DK_RCBR, DK_PIPE,    DK_BSLS, DK_LABK, DK_RABK, DK_QUES, DK_UNDS,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
           DK_HASH, DK_DLR,  DK_LPRN, DK_RPRN, DK_EXLM,    DK_PLUS, DK_MINS, DK_SLSH, DK_ASTR, DK_PERC,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
           DK_PERC, DK_CIRC, DK_LBRC, DK_RBRC, DK_TILD,    DK_AMPR, DK_EQL , DK_UNDS, DK_DOT , XXXXXXX,
        //.--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------.
                                      _______, _______,    _______, _______ 
        //                           .--------+--------.  .--------+--------'
    ),
/*
 * Raise Layer: Number keys
 */

    [_RAISE] = LAYOUT_USER(
        //.--------+--------+--------+--------+--------.  .--------+--------+--------+--------+--------.
           XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,    XXXXXXX,   DK_7,    DK_8,    DK_9,  XXXXXXX,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
           ALTSTAB,  ALTTAB, WM_LEFT, WM_RGHT, XXXXXXX,    XXXXXXX,   DK_4,    DK_5,    DK_6,    DK_0,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
           XXXXXXX, XXXXXXX, KC_BTN4, KC_BTN5, XXXXXXX,    XXXXXXX,   DK_1,    DK_2,    DK_3,  KC_PENT,
        //.--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------.
                                      _______,  _______,    DK_0,   _______
        //                           .--------+--------.  .--------+--------'
    ),

    [_ADJUST] = LAYOUT_USER(
        //.--------+--------+--------+--------+--------.  .--------+--------+--------+--------+--------.
            KC_F1,   KC_F2,   KC_F3,   KC_F3,  XXXXXXX,    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  RESET,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
            KC_F5,   KC_F6,   KC_F7,   KC_F8,  KC_PENT,    XXXXXXX, KC_LEFT, KC_DOWN,  KC_UP,  KC_RGHT,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
            KC_F9,   KC_F10,  KC_F11,  KC_F12, XXXXXXX,    XXXXXXX, KC_HOME, KC_PGDN, KC_PGUP,  KC_END,
        //.--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------.
                                      _______,  _______,   _______, _______
        //                           .--------+--------.  .--------+--------'
    ),

    [_SHORTCUTS] = LAYOUT_USER(
        //.--------+--------+--------+--------+--------.  .--------+--------+--------+--------+--------.
           XXXXXXX,  SFIND,    FIND,  REPLACE,   SALL,     XXXXXXX, XXXXXXX, XXXXXXX,  SFIND,  XXXXXXX,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
             UNDO,    CUT,     COPY,   PASTE,    SAVE,     XXXXXXX,  KC_INS, KC_DEL,    FIND,  REPLACE,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
           XXXXXXX, XXXXXXX, XXXXXXX, FINDALL, C_A_DEL,    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        //.--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------.
                                      _______,  _______,    C_BSPC, _______
        //                           .--------+--------.  .--------+--------'
    )
};


// Combos
#include "g/keymap_combo.h"


// Mod keys
#include "features/modkeys.c"


bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (!process_mod_keys(keycode, record)) {
        return false;
    }

    if (!process_case_modes(keycode, record)) {
        return false;
    }

    switch (keycode) {
        case CASEWRD:
            if (record->event.pressed) {
                if (get_xcase_state() == XCASE_ON || caps_word_enabled() == true) {
                        disable_xcase();
                        disable_caps_word();
                } else if (get_mods() & MOD_MASK_SHIFT) {
                    if (get_xcase_state() == XCASE_ON) {
                        disable_xcase();
                    } else {
                        enable_xcase_with(DK_UNDS);
                    }
                } else if (get_mods() & MOD_MASK_CTRL) {
                    if (get_xcase_state() == XCASE_ON || caps_word_enabled() == true) {
                        disable_xcase();
                        disable_caps_word();
                    } else {
                        enable_xcase_with(DK_UNDS);
                        enable_caps_word();
                    }
                } else {
                    toggle_caps_word();
                }
            }
            return false;
        default:
            return true;
    }
}


void keyboard_post_init_user(void) {
    // Set default layer
    default_layer_set(1UL << _HANDSDOWN);

#ifdef RGBLIGHT_ENABLE
    //rgblight_enable_noeeprom(); // Enables RGB, without saving settings
    rgblight_sethsv_noeeprom(HSV_RED);
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
#endif
}


void suspend_power_down_user(void) {
#ifdef OLED_DRIVER_ENABLE
    oled_off();
#endif
}

void suspend_wakeup_init_user(void) {
#ifdef OLED_DRIVER_ENABLE
    oled_on();
#endif
}
