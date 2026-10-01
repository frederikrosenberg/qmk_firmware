# Set any rules.mk overrides for your specific keymap here.
# See rules at https://docs.qmk.fm/#/config_options?id=the-rulesmk-file

COMBO_ENABLE = yes
OLED_ENABLE = no
RGBLIGTH_ENABLE = no
AUTOCORRECT_ENABLE = yes
OS_DETECTION_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes

LTO_ENABLE = yes

INTROSPECTION_KEYMAP_C += user.c
SRC += features/casemodes.c
SRC += features/oneshot.c
SRC += features/host_os.c

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

ifeq ($(KEYBOARD_DEBUG), 1)
    CONSOLE_ENABLE = yes
    # Omit the autocorrect dictionary to leave space for key/matrix diagnostics.
    AUTOCORRECT_ENABLE = no
    OPT_DEFS += -DKEYBOARD_DEBUG
endif
