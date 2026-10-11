# Set any rules.mk overrides for your specific keymap here.
# See rules at https://docs.qmk.fm/#/config_options?id=the-rulesmk-file

COMBO_ENABLE = yes
# The reduced dictionary fits with all OLED functions. Set
# AUTOCORRECT_PROFILE=full and OLED_ENABLE=no for the complete dictionary.
OLED_ENABLE = yes
RGBLIGHT_ENABLE = no
AUTOCORRECT_ENABLE = yes
AUTOCORRECT_PROFILE ?= reduced
ifeq ($(AUTOCORRECT_PROFILE), full)
    OPT_DEFS += -DAUTOCORRECT_FULL_DICTIONARY
else ifneq ($(AUTOCORRECT_PROFILE), reduced)
    $(error AUTOCORRECT_PROFILE must be reduced or full)
endif
ENCODER_ENABLE = no
# Set QWERTY_ENABLE=yes to include the alternate base layer and NUM toggle.
QWERTY_ENABLE ?= no
ifeq ($(QWERTY_ENABLE), yes)
    OPT_DEFS += -DQWERTY_ENABLE
endif
OS_DETECTION_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes

LTO_ENABLE = yes

INTROSPECTION_KEYMAP_C += user.c
SRC += features/casemodes.c
SRC += features/oneshot.c
SRC += features/host_os.c

VPATH += keyboards/gboards


ifeq ($(OLED_ENABLE), yes)
	# Keep scan-rate telemetry on the OLED without enabling the USB console.
	OPT_DEFS += -DDEBUG_MATRIX_SCAN_RATE
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
COMMAND_ENABLE                  =   no
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
