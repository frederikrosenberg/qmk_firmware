# Set any rules.mk overrides for your specific keymap here.
# See rules at https://docs.qmk.fm/#/config_options?id=the-rulesmk-file
COMBO_ENABLE = yes
OLED_DRIVER_ENABLE = no
OLED_ENABLE = no

SRC += user.c
SRC += features/casemodes.c
SRC += features/oneshot.c

VPATH += keyboards/gboards

# Disable features
MOUSEKEY_ENABLE					=	no
BOOTMAGIC_ENABLE				=	no
GRAVE_ESC_ENABLE				=	no
POINTING_DEVICE_ENABLE			=	no
RAW_ENABLE						=	no
SPACE_CADET_ENABLE				=	no
UNICODE_ENABLE					=	no
