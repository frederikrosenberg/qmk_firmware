#include <assert.h>
#include <stdio.h>
#include "features/host_os.h"
#include "features/oneshot.h"
#include "os_detection.h"

static uint32_t eeprom;
static unsigned writes;
static uint16_t pressed_code, released_code;
static uint8_t real_mod;
static bool key_down;
static uint8_t real_mods, weak_mods, suppressed_mods, override_mods, report_mods;
static bool report_keys[256];
static uint32_t now;
static const key_override_t *overrides[] = {&word_backspace_override, &word_left_override, &word_right_override};

uint32_t eeconfig_read_user(void) { return eeprom; }
void eeconfig_update_user(uint32_t value) { eeprom = value; ++writes; }
uint8_t extract_mod_bits(uint16_t keycode) {
    uint8_t mods = (keycode >> 8) & 0x1F;
    return mods & 0x10 ? (mods & 0x0F) << 4 : mods;
}
void register_code16(uint16_t keycode) {
    pressed_code = keycode; key_down = true;
    weak_mods |= extract_mod_bits(keycode);
    register_code(keycode & 0xFF);
}
void unregister_code16(uint16_t keycode) {
    released_code = keycode; key_down = false;
    unregister_code(keycode & 0xFF);
    weak_mods &= ~extract_mod_bits(keycode);
    send_keyboard_report();
}
void register_code(uint8_t keycode) {
    if (IS_MODIFIER_KEYCODE(keycode)) {
        real_mod = keycode; real_mods |= MOD_BIT(keycode);
    } else {
        add_key(keycode);
    }
    send_keyboard_report();
}
void unregister_code(uint8_t keycode) {
    if (IS_MODIFIER_KEYCODE(keycode)) {
        real_mod = 0; real_mods &= ~MOD_BIT(keycode);
    } else {
        del_key(keycode);
    }
    send_keyboard_report();
}
uint8_t get_mods(void) { return real_mods; }
void set_mods(uint8_t mods) { real_mods = mods; }
uint8_t get_oneshot_mods(void) { return 0; }
uint8_t get_oneshot_locked_mods(void) { return 0; }
void add_key(uint8_t keycode) { report_keys[keycode] = true; }
void del_key(uint8_t keycode) { report_keys[keycode] = false; }
void send_keyboard_report(void) { report_mods = ((real_mods | weak_mods) & ~suppressed_mods) | override_mods; }
void set_weak_override_mods(uint8_t mods) { override_mods = mods; }
void clear_weak_override_mods(void) { override_mods = 0; }
void set_suppressed_override_mods(uint8_t mods) { suppressed_mods = mods; }
void clear_suppressed_override_mods(void) { suppressed_mods = 0; }
uint32_t timer_read32(void) { return now; }
uint32_t timer_elapsed32(uint32_t start) { return now - start; }
void wait_ms(uint16_t ms) { now += ms; }
uint8_t read_source_layers_cache(keypos_t key) { (void)key; return _NAV; }
uint16_t key_override_count(void) { return ARRAY_SIZE(overrides); }
const key_override_t *key_override_get(uint16_t index) { return overrides[index]; }
bool is_oneshot_cancel_key(uint16_t keycode) { return keycode == KC_ESC; }
bool is_oneshot_ignored_key(uint16_t keycode) {
    return keycode == OS_SFT || keycode == OS_CTR || keycode == OS_ALT || keycode == OS_MOD || keycode == NUM || keycode == NAV;
}

static void expect_key(uint16_t source, uint16_t output) {
    keyrecord_t record = {.event.pressed = true};
    assert(!process_host_keys(source, &record));
    assert(key_down && pressed_code == output);
    record.event.pressed = false;
    assert(!process_host_keys(source, &record));
    assert(!key_down && released_code == output);
}

// Match the userspace/QMK ordering: release queued one-shots before overrides.
static void word_event(uint16_t keycode, bool pressed, oneshot_state *ctrl, oneshot_state *shift) {
    keyrecord_t record = {.event.pressed = pressed};
    if (ctrl) update_oneshot(ctrl, KC_LCTL, OS_CTR, keycode, &record);
    if (shift) update_oneshot(shift, KC_LSFT, OS_SFT, keycode, &record);
    if (process_key_override(keycode, &record) && keycode < 256) {
        if (pressed) register_code(keycode); else unregister_code(keycode);
    }
}

static bool app_event(uint16_t keycode, bool pressed, oneshot_state *shift) {
    keyrecord_t record = {.event.pressed = pressed};
    bool normal = process_app_switching(keycode, &record);
    uint16_t oneshot_keycode = keycode == NUM && !normal ? KC_TAB : keycode;
    if (normal) normal = process_host_keys(keycode, &record);
    if (shift) update_oneshot(shift, KC_LSFT, OS_SFT, oneshot_keycode, &record);
    if (normal && process_key_override(keycode, &record) && keycode < 256) {
        if (pressed) register_code(keycode); else unregister_code(keycode);
    }
    return normal;
}

static void test_app_switching(void) {
    const host_mode_t modes[] = {HOST_MODE_MAC, HOST_MODE_WINDOWS};
    for (unsigned mode = 0; mode < ARRAY_SIZE(modes); ++mode) {
        host_os_set_mode(modes[mode]);
        uint8_t expected_mod = modes[mode] == HOST_MODE_MAC ? MOD_BIT(KC_LGUI) : MOD_BIT(KC_LALT);
        oneshot_state shift = os_up_unqueued;

        // NAV remains a layer key; NUM is consumed only once NAV is held.
        assert(app_event(NAV, true, &shift));
        assert(report_mods == 0 && !report_keys[KC_TAB]);
        for (unsigned i = 0; i < 3; ++i) {
            assert(!app_event(NUM, true, &shift));
            assert(report_keys[KC_TAB] && report_mods == expected_mod);
            assert(!app_event(NUM, false, &shift));
            assert(!report_keys[KC_TAB] && report_mods == expected_mod);
        }

        // A queued Shift reverses one tap, then forward cycling resumes.
        app_event(OS_SFT, true, &shift);
        app_event(OS_SFT, false, &shift);
        app_event(NUM, true, &shift);
        assert(report_keys[KC_TAB] && report_mods == (expected_mod | MOD_BIT(KC_LSFT)));
        app_event(NUM, false, &shift);
        assert(report_mods == expected_mod && shift == os_up_unqueued);
        app_event(NUM, true, &shift);
        assert(report_mods == expected_mod);
        app_event(NUM, false, &shift);
        assert(app_event(NAV, false, &shift));
        assert(report_mods == 0 && !report_keys[KC_TAB]);

        // Held Shift continues reversing until released.
        app_event(NAV, true, &shift);
        app_event(OS_SFT, true, &shift);
        for (unsigned i = 0; i < 2; ++i) {
            app_event(NUM, true, &shift);
            assert(report_mods == (expected_mod | MOD_BIT(KC_LSFT)));
            app_event(NUM, false, &shift);
            assert(report_mods == (expected_mod | MOD_BIT(KC_LSFT)));
        }
        app_event(OS_SFT, false, &shift);
        assert(report_mods == expected_mod);
        app_event(NAV, false, &shift);
        assert(report_mods == 0);

        // NAV released before NUM clears Tab and its modifier immediately.
        app_event(NAV, true, &shift);
        app_event(NUM, true, &shift);
        app_event(NAV, false, &shift);
        assert(!report_keys[KC_TAB] && report_mods == 0);
        assert(!app_event(NUM, false, &shift));
        assert(report_mods == 0);

        // Other input ends the app switcher before sending that key.
        app_event(NAV, true, &shift);
        app_event(NUM, true, &shift);
        app_event(NUM, false, &shift);
        app_event(KC_A, true, &shift);
        assert(report_keys[KC_A] && report_mods == 0);
        app_event(KC_A, false, &shift);
        app_event(NAV, false, &shift);

        // Mode changes (including reselecting the same mode) cancel safely.
        app_event(NAV, true, &shift);
        app_event(NUM, true, &shift);
        host_os_set_mode(modes[mode]);
        assert(!report_keys[KC_TAB] && report_mods == 0);
        assert(!app_event(NUM, false, &shift));
        app_event(NAV, false, &shift);

        // Individual NUM and ordinary Alt+Tab keep their normal path.
        assert(app_event(NUM, true, &shift));
        assert(app_event(NUM, false, &shift));
        assert(app_event(NUM, true, &shift));
        assert(app_event(NAV, true, &shift));
        assert(!report_keys[KC_TAB] && report_mods == 0);
        assert(app_event(NUM, false, &shift));
        assert(app_event(NAV, false, &shift));
        app_event(KC_LALT, true, &shift);
        assert(app_event(KC_TAB, true, &shift));
        assert(report_keys[KC_TAB] && report_mods == MOD_BIT(KC_LALT));
        app_event(KC_TAB, false, &shift);
        app_event(KC_LALT, false, &shift);
        assert(report_mods == 0);
        expect_key(COPY, modes[mode] == HOST_MODE_MAC ? G(KC_C) : C(KC_C));

        // The gesture never releases an independently held host modifier.
        uint8_t modifier = modes[mode] == HOST_MODE_MAC ? KC_LGUI : KC_LALT;
        app_event(modifier, true, &shift);
        app_event(NAV, true, &shift);
        app_event(NUM, true, &shift);
        app_event(NUM, false, &shift);
        app_event(NAV, false, &shift);
        assert(report_mods == MOD_BIT(modifier));
        app_event(modifier, false, &shift);
        assert(report_mods == 0);
    }

    // Automatic detection changing during a session releases the old host keys.
    host_os_set_mode(HOST_MODE_AUTO);
    process_detected_host_os_user(OS_MACOS);
    app_event(NAV, true, NULL);
    app_event(NUM, true, NULL);
    process_detected_host_os_user(OS_WINDOWS);
    assert(!report_keys[KC_TAB] && report_mods == 0);
    assert(!app_event(NUM, false, NULL));
    app_event(NAV, false, NULL);
}
static void test_word_editing(void) {
    const uint16_t keys[] = {KC_BSPC, KC_LEFT, KC_RGHT};
    host_os_set_mode(HOST_MODE_MAC);

    // Both Control keys, Shift selection, and held-key reports.
    for (unsigned side = 0; side < 2; ++side) {
        uint8_t ctrl = side ? KC_RCTL : KC_LCTL;
        for (unsigned selecting = 0; selecting < 2; ++selecting) {
            for (unsigned i = 0; i < ARRAY_SIZE(keys); ++i) {
                word_event(ctrl, true, NULL, NULL);
                if (selecting) word_event(KC_LSFT, true, NULL, NULL);
                word_event(keys[i], true, NULL, NULL);
                assert(report_keys[keys[i]]);
                assert(report_mods == (MOD_BIT(KC_LALT) | (selecting ? MOD_BIT(KC_LSFT) : 0)));
                assert(get_mods() & MOD_BIT(ctrl));
                now += 1000;
                key_override_task();
                assert(report_keys[keys[i]] && !(report_mods & MOD_MASK_CTRL));
                word_event(keys[i], false, NULL, NULL);
                assert(!report_keys[keys[i]] && (report_mods & MOD_BIT(ctrl)));
                if (selecting) word_event(KC_LSFT, false, NULL, NULL);
                word_event(ctrl, false, NULL, NULL);
                assert(report_mods == 0);
            }
        }
    }

    // Releasing Control first never inserts an extra plain Backspace.
    word_event(KC_LCTL, true, NULL, NULL);
    word_event(KC_BSPC, true, NULL, NULL);
    word_event(KC_LCTL, false, NULL, NULL);
    now += 1000;
    key_override_task();
    assert(!report_keys[KC_BSPC] && report_mods == 0);
    word_event(KC_BSPC, false, NULL, NULL);

    // A following terminal Ctrl+C restores Control and removes Option.
    word_event(KC_LCTL, true, NULL, NULL);
    word_event(KC_LEFT, true, NULL, NULL);
    word_event(KC_C, true, NULL, NULL);
    assert(report_keys[KC_C] && report_mods == MOD_BIT(KC_LCTL));
    word_event(KC_C, false, NULL, NULL);
    word_event(KC_LEFT, false, NULL, NULL);
    word_event(KC_LCTL, false, NULL, NULL);

    // The custom queued and held one-shots work with the real override engine.
    for (unsigned held = 0; held < 2; ++held) {
        oneshot_state ctrl = os_up_unqueued;
        oneshot_state shift = os_up_unqueued;
        word_event(OS_CTR, true, &ctrl, &shift);
        if (!held) word_event(OS_CTR, false, &ctrl, &shift);
        word_event(OS_SFT, true, &ctrl, &shift);
        word_event(OS_SFT, false, &ctrl, &shift);
        word_event(KC_RGHT, true, &ctrl, &shift);
        assert(report_keys[KC_RGHT] && report_mods == (MOD_BIT(KC_LALT) | MOD_BIT(KC_LSFT)));
        word_event(KC_RGHT, false, &ctrl, &shift);
        if (held) word_event(OS_CTR, false, &ctrl, &shift);
        assert(ctrl == os_up_unqueued && shift == os_up_unqueued && report_mods == 0);
    }

    // Windows, plain Backspace, and Ctrl+Alt/GUI chords remain unchanged.
    host_os_set_mode(HOST_MODE_WINDOWS);
    word_event(KC_LCTL, true, NULL, NULL);
    word_event(KC_BSPC, true, NULL, NULL);
    assert(report_keys[KC_BSPC] && report_mods == MOD_BIT(KC_LCTL));
    word_event(KC_BSPC, false, NULL, NULL);
    word_event(KC_LCTL, false, NULL, NULL);
    host_os_set_mode(HOST_MODE_MAC);
    word_event(KC_BSPC, true, NULL, NULL);
    assert(report_keys[KC_BSPC] && report_mods == 0);
    word_event(KC_BSPC, false, NULL, NULL);
    const uint8_t other_mods[] = {KC_LALT, KC_LGUI};
    for (unsigned i = 0; i < ARRAY_SIZE(other_mods); ++i) {
        word_event(KC_LCTL, true, NULL, NULL);
        word_event(other_mods[i], true, NULL, NULL);
        word_event(KC_LEFT, true, NULL, NULL);
        assert(report_mods == (MOD_BIT(KC_LCTL) | MOD_BIT(other_mods[i])));
        word_event(KC_LEFT, false, NULL, NULL);
        word_event(other_mods[i], false, NULL, NULL);
        word_event(KC_LCTL, false, NULL, NULL);
    }

    // A host mode change while held clears the replacement and suppression.
    word_event(KC_LCTL, true, NULL, NULL);
    word_event(KC_LEFT, true, NULL, NULL);
    host_os_set_mode(HOST_MODE_WINDOWS);
    assert(!report_keys[KC_LEFT] && report_mods == MOD_BIT(KC_LCTL));
    word_event(KC_LEFT, false, NULL, NULL);
    word_event(KC_LCTL, false, NULL, NULL);
    assert(report_mods == 0);
}

int main(void) {
    // Fresh and legacy EEPROM values select Auto without wearing EEPROM.
    host_os_init();
    assert(host_os_mode() == HOST_MODE_AUTO && !host_os_is_mac());
    eeprom = 0xFFFFFFFF;
    host_os_init();
    assert(host_os_mode() == HOST_MODE_AUTO && writes == 0);

    process_detected_host_os_user(OS_MACOS);
    assert(host_os_is_mac() && writes == 0);
    expect_key(COPY, G(KC_C));
    expect_key(UNDO, G(KC_Z));
    expect_key(CUT, G(KC_X));
    expect_key(PASTE, G(KC_V));
    expect_key(KC_HOME, G(KC_LEFT));
    expect_key(KC_END, G(KC_RGHT));
    expect_key(C_A_DEL, G(A(KC_ESC)));
    expect_key(WM_LEFT, C(KC_LEFT));
    expect_key(DK_AT, A(KC_NUHS));
    expect_key(DK_DLR, S(A(KC_3)));
    expect_key(DK_LABK, KC_GRV);
    expect_key(DK_RABK, S(KC_GRV));
    expect_key(DK_LCBR, S(A(KC_8)));
    expect_key(DK_RCBR, S(A(KC_9)));
    expect_key(DK_LBRC, A(KC_8));
    expect_key(DK_RBRC, A(KC_9));
    expect_key(DK_PIPE, A(KC_I));
    expect_key(DK_BSLS, S(A(KC_7)));
    expect_key(DK_TILD, A(KC_RBRC));

    // An override wins over later callbacks and survives initialization.
    host_os_set_mode(HOST_MODE_WINDOWS);
    assert(writes == 1 && !host_os_is_mac());
    process_detected_host_os_user(OS_MACOS);
    assert(!host_os_is_mac());
    host_os_init();
    assert(host_os_mode() == HOST_MODE_WINDOWS && !host_os_is_mac());
    host_os_set_mode(HOST_MODE_WINDOWS);
    assert(writes == 1);
    expect_key(COPY, C(KC_C));
    expect_key(DK_AT, RALT(KC_2));
    expect_key(DK_LABK, KC_NUBS);
    expect_key(DK_RABK, S(KC_NUBS));
    expect_key(KC_HOME, KC_HOME);

    host_os_set_mode(HOST_MODE_MAC);
    process_detected_host_os_user(OS_WINDOWS);
    assert(host_os_is_mac());
    host_os_init();
    assert(host_os_mode() == HOST_MODE_MAC);
    host_os_set_mode(HOST_MODE_AUTO);
    assert(!host_os_is_mac());
    host_os_init();
    assert(host_os_mode() == HOST_MODE_AUTO);
    process_detected_host_os_user(OS_IOS);
    assert(host_os_is_mac());
    process_detected_host_os_user(OS_LINUX);
    assert(!host_os_is_mac());
    process_detected_host_os_user(OS_UNSURE);
    assert(!host_os_is_mac());

    // Detection changing while a shortcut is held releases the original code.
    keyrecord_t record = {.event.pressed = true};
    process_detected_host_os_user(OS_MACOS);
    assert(!process_host_keys(PASTE, &record));
    process_detected_host_os_user(OS_WINDOWS);
    record.event.pressed = false;
    assert(!process_host_keys(PASTE, &record));
    assert(released_code == G(KC_V) && !key_down);

    // A queued modifier lasts through an adapted key, then clears on release.
    oneshot_state shift = os_up_unqueued;
    record.event.pressed = true;
    update_oneshot(&shift, KC_LSFT, OS_SFT, OS_SFT, &record);
    record.event.pressed = false;
    update_oneshot(&shift, KC_LSFT, OS_SFT, OS_SFT, &record);
    assert(shift == os_up_queued && real_mod == KC_LSFT);
    process_detected_host_os_user(OS_MACOS);
    record.event.pressed = true;
    assert(!process_host_keys(DK_LBRC, &record));
    update_oneshot(&shift, KC_LSFT, OS_SFT, DK_LBRC, &record);
    assert(real_mod == KC_LSFT);
    record.event.pressed = false;
    assert(!process_host_keys(DK_LBRC, &record));
    update_oneshot(&shift, KC_LSFT, OS_SFT, DK_LBRC, &record);
    assert(shift == os_up_unqueued && real_mod == 0 && !key_down);

    // Ordinary typing and the Danish letter combos remain on QMK's normal path.
    assert(process_host_keys(DK_A, &record));
    assert(process_host_keys(DK_ARNG, &record));
    assert(process_host_keys(DK_AE, &record));
    assert(process_host_keys(DK_OSTR, &record));
    test_word_editing();
    test_app_switching();
    puts("Host OS tests passed");
    return 0;
}
