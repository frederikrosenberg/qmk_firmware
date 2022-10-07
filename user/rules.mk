# Set any rules.mk overrides for your specific keymap here.
# See rules at https://docs.qmk.fm/#/config_options?id=the-rulesmk-file

COMBO_ENABLE = yes
OLED_ENABLE = yes

LTO_ENABLE = yes

SRC += user.c
SRC += features/casemodes.c
SRC += features/oneshot.c

VPATH += keyboards/gboards


ifeq ($(OLED_ENABLE), yes)
	SRC += oled.c
	WPM_ENABLE = yes
	DEFERRED_EXEC_ENABLE = yes
endif


# Disable features
MOUSEKEY_ENABLE					=	no
BOOTMAGIC_ENABLE				=	no
GRAVE_ESC_ENABLE				=	no
POINTING_DEVICE_ENABLE			=	no
RAW_ENABLE						=	no
SPACE_CADET_ENABLE				=	no
UNICODE_ENABLE					=	no
CONSOLE_ENABLE                  =   no
AUTO_SHIFT_MODIFIERS            =   no
AUTO_SHIFT_ENABLE               =   no
BACKLIGHT_ENABLE                =   no
MAGIC_ENABLE                    =   no

AVR_USE_MINIMAL_PRINTF = yes
