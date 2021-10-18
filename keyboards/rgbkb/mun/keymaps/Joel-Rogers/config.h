/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <https://github.com/Legonut> wrote this file.  As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day, and
 * you think this stuff is worth it, you can buy me a beer in return. David Rauseo
 * ----------------------------------------------------------------------------
 */

// I have no idea if this file is necessary! Check with e.g. Xulkal. Probably RGB_DISABLE is
// a good idea, but should then drop this from the mun/config.h file!!!
#pragma once

// No need for the single versions when multi performance isn't a problem =D
#define DISABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE
#define DISABLE_RGB_MATRIX_SOLID_REACTIVE_CROSS
#define DISABLE_RGB_MATRIX_SOLID_REACTIVE_NEXUS
#define DISABLE_RGB_MATRIX_SPLASH
#define DISABLE_RGB_MATRIX_SOLID_SPLASH

// 20m timeout (20m * 60s * 1000mil)
// #define RGB_DISABLE_TIMEOUT 1200000
//#define RGB_DISABLE_WHEN_USB_SUSPENDED //already defined in mun/config.h

#define STM32_ONBOARD_EEPROM_SIZE 2048
