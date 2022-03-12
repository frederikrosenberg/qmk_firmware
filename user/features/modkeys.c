
void mods_tap_key(uint16_t code_no_mod, uint16_t code_mod) {
    uint8_t mods = get_mods();
    if (mods & MOD_MASK_SHIFT) {
        del_mods(MOD_MASK_SHIFT);
        tap_code16(code_mod);
        set_mods(mods);
    } else {
        tap_code16(code_no_mod);
    }
}

#define SHIFT_CASE(key, code_no_mod, code_mod) \
    case key:                                  \
        mods_tap_key(code_no_mod, code_mod);   \
        return false;


#undef SHIFT
#define SHIFT SHIFT_CASE
bool process_mod_keys(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return true;
    switch (keycode)
    {
    #include "modkeys.def"
    }

    return true;
}
#undef SHIFT
