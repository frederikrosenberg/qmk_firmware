// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "user.h"

typedef enum {
    HOST_MODE_AUTO,
    HOST_MODE_WINDOWS,
    HOST_MODE_MAC,
} host_mode_t;

void host_os_init(void);
void host_os_set_mode(host_mode_t mode);
host_mode_t host_os_mode(void);
bool host_os_is_mac(void);
bool process_host_keys(uint16_t keycode, keyrecord_t *record);
bool process_app_switching(uint16_t keycode, keyrecord_t *record);

extern const key_override_t word_backspace_override;
extern const key_override_t word_left_override;
extern const key_override_t word_right_override;
