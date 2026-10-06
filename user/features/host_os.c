// SPDX-License-Identifier: GPL-3.0-or-later
#include "host_os.h"
#include "os_detection.h"

// Only explicit overrides are saved; detected hosts never write to EEPROM.
#define HOST_CONFIG_SIGNATURE 0x484F5300UL
static host_mode_t mode = HOST_MODE_AUTO;
static os_variant_t host_os = OS_UNSURE;
static bool word_editing_enabled;
static bool nav_held;
static bool app_switching;
static bool app_num_down;
static bool app_modifier_added;
static uint8_t app_modifier;

static void stop_app_switching(void) {
    if (!app_switching) return;
    // Consume the pending NUM release even when NAV is released first.
    if (app_num_down) unregister_code(KC_TAB);
    if (app_modifier_added) unregister_code(app_modifier);
    app_switching = false;
}

bool process_app_switching(uint16_t keycode, keyrecord_t *record) {
    if (keycode == NAV) {
        nav_held = record->event.pressed;
        if (!nav_held) stop_app_switching();
    }
    if (keycode == NUM) {
        if (!record->event.pressed && app_num_down) {
            unregister_code(KC_TAB);
            app_num_down = false;
            return false;
        }
        if (record->event.pressed && nav_held) {
            if (!app_switching) {
                app_modifier = host_os_is_mac() ? KC_LGUI : KC_LALT;
                app_modifier_added = !(get_mods() & MOD_BIT(app_modifier));
                if (app_modifier_added) register_code(app_modifier);
                app_switching = true;
            }
            register_code(KC_TAB);
            app_num_down = true;
            return false;
        }
    }
    // Shift can be added while cycling. Any other key ends the session first.
    if (app_switching && record->event.pressed && keycode != NAV && keycode != OS_SFT && keycode != KC_LSFT && keycode != KC_RSFT) {
        stop_app_switching();
    }
    return true;
}

// Suppress either Control key only while a word-editing override is active.
// Shift remains available for selection; Alt/GUI chords keep their own meaning.
#define WORD_EDITING_OVERRIDE(key) { \
    .trigger = (key), \
    .trigger_mods = MOD_MASK_CTRL, \
    .layers = ~(layer_state_t)0, \
    .negative_mod_mask = MOD_MASK_ALT | MOD_MASK_GUI, \
    .suppressed_mods = MOD_MASK_CTRL, \
    .replacement = A(key), \
    .options = ko_options_default | ko_option_no_reregister_trigger, \
    .enabled = &word_editing_enabled, \
}

const key_override_t word_backspace_override = WORD_EDITING_OVERRIDE(KC_BSPC);
const key_override_t word_left_override = WORD_EDITING_OVERRIDE(KC_LEFT);
const key_override_t word_right_override = WORD_EDITING_OVERRIDE(KC_RGHT);

static void update_word_editing(void) {
    bool enabled = host_os_is_mac();
    if (app_switching && app_modifier != (enabled ? KC_LGUI : KC_LALT)) stop_app_switching();
    if (word_editing_enabled && !enabled) {
        // Release an active replacement before leaving Mac mode.
        key_override_off();
        key_override_on();
    }
    word_editing_enabled = enabled;
}

// Keep the Danish input source selected on both computers.
// Mac Option mappings: Unicode CLDR keyboards/osx/da-t-k0-osx.xml.
// Dollar uses Option+Shift+3 to avoid the ISO/ANSI grave-key difference.
// Angle brackets use the grave position when Mac maps the ISO key to $/§.
static const uint16_t PROGMEM host_keys[][2] = {
    {UNDO,    G(DK_Z)},
    {CUT,     G(DK_X)},
    {COPY,    G(DK_C)},
    {PASTE,   G(DK_V)},
    {SAVE,    G(DK_S)},
    {WM_LEFT, C(KC_LEFT)},
    {WM_RGHT, C(KC_RGHT)},
    {C_A_DEL, G(A(KC_ESC))},
    {C_BSPC,  A(KC_BSPC)},
    {KC_HOME, G(KC_LEFT)},
    {KC_END,  G(KC_RGHT)},
    {DK_AT,   A(DK_QUOT)},
    {DK_DLR,  S(A(DK_3))},
    {DK_LABK, KC_GRV},
    {DK_RABK, S(KC_GRV)},
    {DK_LCBR, S(A(DK_8))},
    {DK_RCBR, S(A(DK_9))},
    {DK_LBRC, A(DK_8)},
    {DK_RBRC, A(DK_9)},
    {DK_PIPE, A(DK_I)},
    {DK_BSLS, S(A(DK_7))},
    {DK_TILD, A(DK_DIAE)},
};

// Release the code chosen on keydown even if detection changes while held.
static uint16_t active_keys[ARRAY_SIZE(host_keys)];

void host_os_init(void) {
    mode = HOST_MODE_AUTO;
    uint32_t config = eeconfig_read_user();
    uint8_t saved_mode = config & 0xFF;
    if ((config & 0xFFFFFF00UL) == HOST_CONFIG_SIGNATURE && saved_mode <= HOST_MODE_MAC) {
        mode = (host_mode_t)saved_mode;
    }
    update_word_editing();
}

void host_os_set_mode(host_mode_t new_mode) {
    stop_app_switching();
    if (mode == new_mode) {
        return;
    }
    mode = new_mode;
    update_word_editing();
    eeconfig_update_user(HOST_CONFIG_SIGNATURE | mode);
}

host_mode_t host_os_mode(void) {
    return mode;
}

bool host_os_is_mac(void) {
    return mode == HOST_MODE_MAC || (mode == HOST_MODE_AUTO && (host_os == OS_MACOS || host_os == OS_IOS));
}

bool process_detected_host_os_user(os_variant_t detected_os) {
    host_os = detected_os;
    update_word_editing();
#ifdef KEYBOARD_DEBUG
    uprintf("Detected host OS: %u; mode: %u\n", detected_os, mode);
#endif
    return true;
}

bool process_host_keys(uint16_t keycode, keyrecord_t *record) {
    for (uint8_t i = 0; i < ARRAY_SIZE(host_keys); ++i) {
        if (keycode != pgm_read_word(&host_keys[i][0])) {
            continue;
        }
        if (record->event.pressed) {
            active_keys[i] = pgm_read_word(&host_keys[i][host_os_is_mac() ? 1 : 0]);
            register_code16(active_keys[i]);
        } else if (active_keys[i]) {
            // Release before clearing a queued one-shot modifier.
            unregister_code16(active_keys[i]);
            active_keys[i] = 0;
        }
        return false;
    }
    return true;
}
