# MCU name
MCU = STM32F303

# Touch encoder needs
SRC += ../common/touch_encoder.c
QUANTUM_LIB_SRC += i2c_master.c

# Build Options
#   change to "no" to disable the options, or define them in the Makefile in
#   the appropriate keymap folder that will get included automatically
#
BOOTMAGIC_ENABLE = no       # Virtual DIP switch configuration
## (Note that for BOOTMAGIC on Teensy LC you have to use a custom .ld script.)
MOUSEKEY_ENABLE = yes        # Mouse keys (nice for touchbar)
EXTRAKEY_ENABLE = yes       # Audio control and System control
CONSOLE_ENABLE = yes        # Console for debug
COMMAND_ENABLE = no         # Commands for debug and configuration
NKRO_ENABLE = yes           # Nkey Rollover - if this doesn't work, see here: https://github.com/tmk/tmk_keyboard/wiki/FAQ#nkro-doesnt-work
MIDI_ENABLE = no            # MIDI controls
AUDIO_ENABLE = no           # Audio output
UNICODE_ENABLE = no         # Unicode
BLUETOOTH_ENABLE = no       # Enable Bluetooth with the Adafruit EZ-Key HID
SWAP_HANDS_ENABLE = yes		# Enable swap hands commands

WS2812_DRIVER = pwm
RGBLIGHT_ENABLE = no        # Enable WS2812 RGB underlight.
RGB_MATRIX_ENABLE = yes
RGB_MATRIX_DRIVER = WS2812
SLEEP_LED_ENABLE = yes       # Breathing sleep LED during USB suspend
RGB_UNLIMITED_POWER = yes   # Enable max power to the LEDs

OLED_DRIVER_ENABLE = yes    # Enable the OLED Driver

ENCODER_ENABLE = yes

SPLIT_KEYBOARD = yes
SERIAL_DRIVER = usart
DEBOUNCE_TYPE = sym_eager_pk
LTO_ENABLE = no
OPT = 3

OPT_DEFS += -DOLED_FONT_H=\"../common/glcdfont.c\"
OPT_DEFS += -Ikeyboards/rgbkb/common

DEFAULT_FOLDER = rgbkb/mun/rev1

RAW_ENABLE = no 		# Enable raw HID - currently disabled as is of almost no use on Windows
CONSOLE_ENABLE = yes 	# Enable debugging info output - will hijack for sending layer info to Arduino

TAP_DANCE_ENABLE = yes