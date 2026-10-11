#pragma once
#include "qmk_test.h"
typedef struct { bool autocorrect_enable; } keymap_config_t;
extern keymap_config_t keymap_config;
void eeconfig_update_keymap(keymap_config_t *config);
