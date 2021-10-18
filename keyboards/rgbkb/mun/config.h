/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <https://github.com/Legonut> wrote this file.  As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day, and
 * you think this stuff is worth it, you can buy me a beer in return. David Rauseo
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "config_common.h"

/* USB Device descriptor parameter */
#define VENDOR_ID       0x3535
#define PRODUCT_ID      0x3505
// Used to be a couple of page/usage IDs here, now removed
//#define RAW_USAGE_PAGE  0xFF60
//#define RAW_USAGE_ID   0x61
#define MANUFACTURER    RGBKB
#define PRODUCT         Mün

#define USB_POLLING_INTERVAL_MS 1
//#define DEBOUNCE 10 // old

/* Matrix Configuration - Rows are doubled up */
#define MATRIX_ROWS 14
// B1, A2 reserved for encoder / touch encoder support
#define MATRIX_ROW_PINS { A1, A3, B3, A13, B15, B1, A2 }
#define MATRIX_COLS 7
#define MATRIX_COL_PINS { A0, B11, B0, B10, B12, B2, A8 }
#define MATRIX_IO_DELAY 5

#define BUSY_WAIT
#define BUSY_WAIT_INSTRUCTIONS 35 // Increase if two rows are pressed at the same time.
#define GPIO_INPUT_PIN_DELAY 10

/* Touchbar adjustments */
#define TOUCH_DEADZONE 50 // width of a "button" for tap/hold purposes, wider inputs will be interpreted as a swipe
#define TOUCH_TERM 350 // time of a "button" touch, longer inputs will be a swipe or hold - 350 (ms) works well,
// but can set arbitrarily high (e.g. 50000) for hold behaviour
#define TOUCH_RESOLUTION 10 // sensitivity of swipes, lower=faster

/* Encoder Configuration */
#define ENCODERS_PAD_A { B8, B9 }
#define ENCODERS_PAD_B { A14, A15 }
#define TOUCH_SEGMENTS 3

/* COL2ROW or ROW2COL */
#define DIODE_DIRECTION COL2ROW

/* CRC Configuration */
#define CRC8_OPTIMIZE_SPEED
#define CRC8_USE_TABLE

/* Split Keyboard Configuration */
#define EE_HANDS
#define SPLIT_USB_DETECT
// also handles the SERIAL_USART_TX_PIN define
//#define SPLIT_TRANSPORT_MIRROR //old
#define SOFT_SERIAL_PIN A9
//#define SERIAL_USART_SPEED (1 * 1024 * 1024)
//#define SERIAL_USART_DRIVER SD1 //old
#define SERIAL_USART_TX_PAL_MODE 7
#define SERIAL_USART_TIMEOUT 5
#define SERIAL_USART_DRIVER SD1
//#define SERIAL_USART_FULL_DUPLEX - Waiting on reunification pr
#if SERIAL_USART_FULL_DUPLEX
    #define SERIAL_USART_RX_PIN A10
    #define SERIAL_USART_RX_PAL_MODE 7
    // Mun connects TX to TX and RX to RX as we were planning on i2c split, so we need pin swap for full duplex
    #define SERIAL_USART_PIN_SWAP
    #define SERIAL_USART_SPEED (2 * 1024 * 1024)
#else
    #define SERIAL_USART_SPEED (1 * 1024 * 1024)
#endif

/* Split Transport Features */
#define SPLIT_TRANSPORT_MIRROR
#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_LED_STATE_ENABLE
#define SPLIT_TRANSACTION_IDS_KB TOUCH_ENCODER_SYNC, RGB_MENU_SYNC

/* RGB LED Configuration */
#define RGB_DI_PIN B5
#define RGBLED_NUM 98
#define RGBLED_SPLIT { 49, 49 }
#define RGBLIGHT_ANIMATIONS

#define DRIVER_LED_TOTAL RGBLED_NUM
#define RGB_MATRIX_SPLIT RGBLED_SPLIT
#define RGB_MATRIX_CENTER { 128, 34 }
#define RGB_MATRIX_LED_FLUSH_LIMIT 33
#define RGB_MATRIX_LED_PROCESS_LIMIT 10
#define RGB_MATRIX_KEYPRESSES
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#define RGB_DISABLE_WHEN_USB_SUSPENDED

// I used to skip the "if" here, so need to be sure I call for unlimited power somewhere next.
//NB: Seems to be in mun/rules.mk
#if RGB_UNLIMITED_POWER
  #define RGBLIGHT_LIMIT_VAL 255
#else
  #define RGBLIGHT_LIMIT_VAL 127
#endif
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS RGBLIGHT_LIMIT_VAL

#define WS2812_PWM_DRIVER PWMD3
#define WS2812_PWM_CHANNEL 2
#define WS2812_PWM_PAL_MODE 2
#define WS2812_DMA_STREAM STM32_DMA1_STREAM3
#define WS2812_DMA_CHANNEL 3

#define TOUCH_UPDATE_INTERVAL 33
#define OLED_UPDATE_INTERVAL 33
#define TAP_CODE_DELAY 5


// My personal tweaks
#define MOUSEKEY_WHEEL_TIME_TO_MAX 0 // Set time to reach max scroll speed (mouse keys; default 40)
#define MOUSEKEY_WHEEL_MAX_SPEED 100  // Set max number of scroll steps (default 8)

#define TAPPING_TERM 200  // Tap-dance/tap-hold term duration
#define PERMISSIVE_HOLD  // Better for home row mods

// Disable the default touchbar behaviour (currently necessary if you want to use the touchbar for
// layer switching)
//#define MUN_CUSTOM_TOUCH_ENCODERS //No longer necessary

// Not currently interested in including the hold behaviour as I have no use for it, but I should check whether or not it works at some point.
// Would also be interesting to overload this with tap-hold (-swipe??) behaviour!
//#define TOUCHBAR_HOLD_ENABLE
