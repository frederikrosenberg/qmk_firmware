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
