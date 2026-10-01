// Minimal host-side QMK API shim. Keycodes come from the actual QMK checkout.
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "quantum_keycodes.h"
#include "keycode.h"

#define PROGMEM
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define pgm_read_word(address) (*(address))

typedef uint8_t layer_state_t;
typedef struct { uint8_t row, col; } keypos_t;
typedef struct {
    struct { bool pressed; keypos_t key; } event;
} keyrecord_t;

#include "process_key_override.h"

uint32_t eeconfig_read_user(void);
void eeconfig_update_user(uint32_t value);
void register_code16(uint16_t keycode);
void unregister_code16(uint16_t keycode);
void register_code(uint8_t keycode);
void unregister_code(uint8_t keycode);
uint8_t get_mods(void);
void set_mods(uint8_t mods);
uint8_t get_oneshot_mods(void);
uint8_t get_oneshot_locked_mods(void);
void add_key(uint8_t keycode);
void del_key(uint8_t keycode);
void send_keyboard_report(void);
uint32_t timer_read32(void);
uint32_t timer_elapsed32(uint32_t start);
void wait_ms(uint16_t ms);
uint8_t read_source_layers_cache(keypos_t key);
uint16_t key_override_count(void);
struct key_override_t;
const struct key_override_t *key_override_get(uint16_t index);
