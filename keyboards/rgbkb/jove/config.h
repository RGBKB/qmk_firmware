/*
 * Copyright 2022 Kevin Gee <info@controller.works>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#pragma once

#include "config_common.h"

/* Debounce reduces chatter (unintended double-presses) - set 0 if debouncing is not needed */
/* #define DEBOUNCE 5 */

#define WS2812_PIO_USE_PIO15
#define RGB_DI_PIN GP15
#define RGBLED_NUM 184
#define DRIVER_LED_TOTAL 184
#define RGB_MATRIX_SPLIT { 92, 92 }
#define RGBLIGHT_LIMIT_VAL 255
#define RGB_MATRIX_CENTER { 105, 38 }
#define RGB_MATRIX_LED_FLUSH_LIMIT 33
#define RGB_MATRIX_LED_PROCESS_LIMIT 10
#define RGB_DISABLE_WHEN_USB_SUSPENDED

#define RGB_MATRIX_DRIVER WS2812


#define ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON


#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_LED_STATE_ENABLE
#define SPLIT_MODS_ENABLE
#define SPLIT_USB_DETECT
#define SERIAL_USART_FULL_DUPLEX
#define SERIAL_USART_RX_PIN GP17
#define SERIAL_USART_TX_PIN GP16
#define SPLIT_HAND_PIN GP14
//#define SERIAL_USART_TX_PAL_MODE 7
//#define SERIAL_USART_TIMEOUT 5
//#define SERIAL_USART_DRIVER SD1
//#define SERIAL_USART_RX_PAL_MODE 7
//#define SERIAL_USART_SPEED (2 * 1024 * 1024)

//#define SPLIT_TRANSPORT_MIRROR
//#define SPLIT_LAYER_STATE_ENABLE
//#define SPLIT_LED_STATE_ENABLE
//#define SPLIT_TRANSACTION_IDS_KB TOUCH_ENCODER_SYNC, RGB_MENU_SYNC

#undef I2C_DRIVER
#define I2C_DRIVER I2CD1
#undef I2C1_SCL_PIN
#define I2C1_SCL_PIN GP25
#undef I2C1_SDA_PIN
#define I2C1_SDA_PIN GP24

/* #define EE_HANDS  */
/* #define MASTER_LEFT */
/* #define MASTER_RIGHT */

/* Top left key on left half */
#define BOOTMAGIC_LITE_ROW    0
#define BOOTMAGIC_LITE_COLUMN 0
/* Top right key on right half */
#define BOOTMAGIC_LITE_ROW_RIGHT    0
#define BOOTMAGIC_LITE_COLUMN_RIGHT 0
/*
 * Feature disable options
 *  These options are also useful to firmware size reduction.
 */

/* disable debug print */
//#define NO_DEBUG

/* disable print */
//#define NO_PRINT

/* disable action features */
//#define NO_ACTION_LAYER
//#define NO_ACTION_TAPPING
//#define NO_ACTION_ONESHOT


