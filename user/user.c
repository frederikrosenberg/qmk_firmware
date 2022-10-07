#include "user.h"
#include "features/casemodes.h"
#include "features/oneshot.h"

#ifdef OLED_ENABLE
#include "oled.h"
#endif

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_CANARY] = LAYOUT_USER(
        //.--------+--------+--------+--------+--------.  .--------+--------+--------+--------+--------.
             DK_W,    DK_L,    DK_Y,    DK_P,    DK_B,       DK_Z,    DK_F,    DK_O,    DK_U,    QUOTE,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
             DK_C,    DK_R,    DK_S,    DK_T,    DK_G,       DK_M,    DK_N,   DK_E,     DK_I,    DK_A,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
             DK_Q,    DK_J,   DK_V,     DK_D,    DK_K,       DK_X,    DK_H,  DK_SLSH, DK_COMM,  DK_DOT,
        //.--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------.
                                        NUM,   SPC_SYM,     OS_SFT,   NAV 
        //                           .--------+--------|  |--------+--------.
    ),

    [_NUM] = LAYOUT_USER(
        //.--------+--------+--------+--------+--------.  .--------+--------+--------+--------+--------.
           XXXXXXX, CASEWRD, WM_LEFT, WM_RGHT, XXXXXXX,    XXXXXXX,   DK_7,    DK_8,    DK_9,  TOGBASE,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
            OS_ALT,  OS_MOD,  OS_SFT,  OS_CTR, KC_RCTL,    XXXXXXX,   DK_4,    DK_5,    DK_6,    DK_0,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
             UNDO,    CUT,     COPY,   PASTE,  XXXXXXX,    XXXXXXX,   DK_1,    DK_2,    DK_3,  KC_PENT,
        //.--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------.
                                        NUM,   _______,    KC_ENT,    NAV
        //                           .--------+--------.  .--------+--------'
    ),

    [_NAV] = LAYOUT_USER(
        //.--------+--------+--------+--------+--------.  .--------+--------+--------+--------+--------.
            KC_F1,   KC_F2,   KC_F3,   KC_F4,  XXXXXXX,     KC_DEL, KC_HOME, KC_PGDN, KC_PGUP,  KC_END,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
            KC_F5,   KC_F6,   KC_F7,   KC_F8,  KC_PENT,     KC_INS,  OS_CTR,  OS_SFT,  OS_MOD,  OS_ALT,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
            KC_F9,   KC_F10,  KC_F11,  KC_F12, XXXXXXX,    C_A_DEL, KC_LEFT, KC_DOWN,  KC_UP,  KC_RGHT,
        //.--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------.
                                        NUM,    KC_TAB,    _______,   NAV
        //                           .--------+--------.  .--------+--------'
    ),

    [_SYM] = LAYOUT_USER(
        //.--------+--------+--------+--------+--------.  .--------+--------+--------+--------+--------.
           DK_GRV , DK_AT,   DK_LCBR, DK_RCBR, DK_PIPE,    DK_BSLS, DK_LABK, DK_RABK, DK_QUES, XXXXXXX,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
           DK_HASH, DK_DLR,  DK_LPRN, DK_RPRN, DK_EXLM,    DK_PLUS, DK_MINS, DK_SLSH, DK_ASTR, DK_PERC,
        //|--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------|
           XXXXXXX, DK_CIRC, DK_LBRC, DK_RBRC, DK_TILD,    DK_AMPR, DK_EQL , DK_UNDS, DK_DOT , XXXXXXX,
        //.--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------.
                                        NUM,   _______,    KC_BSPC, KC_ENT
        //                           .--------+--------.  .--------+--------'
    ),
};


// Combos
#include "g/keymap_combo.h"


// Mod keys
#include "features/modkeys.c"

oneshot_state os_alt_state = os_up_unqueued;
oneshot_state os_mod_state = os_up_unqueued;
oneshot_state os_sft_state = os_up_unqueued;
oneshot_state os_ctr_state = os_up_unqueued;

uint16_t base_layer = _HANDSDOWN;

bool is_oneshot_cancel_key(uint16_t keycode) {
    switch (keycode) {
        case KC_ESC:
        case KC_NO:
            return true;
        default:
            return false;
    }
}

bool is_oneshot_ignored_key(uint16_t keycode) {
    switch (keycode) {
        case NUM:
        case NAV:
        case OS_ALT:
        case OS_MOD:
        case OS_SFT:
        case OS_CTR:
            return true;
        default:
            return false;
    }

}


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
#ifdef OLED_ENABLE
    if (record->event.pressed) {
        process_record_oled(keycode, record);
    }
#endif

    if (!process_mod_keys(keycode, record)) {
        return false;
    }

    if (!process_case_modes(keycode, record)) {
        return false;
    }

    update_oneshot(&os_sft_state, KC_LSFT, OS_SFT, keycode, record);
    update_oneshot(&os_ctr_state, KC_LCTL, OS_CTR, keycode, record);
    update_oneshot(&os_alt_state, KC_LALT, OS_ALT, keycode, record);
    update_oneshot(&os_mod_state, KC_LGUI, OS_MOD, keycode, record);

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
    default_layer_set(1UL << _CANARY);

#ifdef RGBLIGHT_ENABLE
    //rgblight_enable_noeeprom(); // Enables RGB, without saving settings
    rgblight_sethsv_noeeprom(HSV_RED);
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
#endif
}


