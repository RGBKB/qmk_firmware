/*
 * Copyright 2022 <https://github.com/Legonut>
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
#include "jove.h"

#ifdef OLED_ENABLE

static void render_logo(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 12, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 12, 12,  0,  0,  0,  0,  0,  0,  0,  0,128,192, 96, 48, 24, 12,254,254,  0,  0,  0,  0,  0,  0, 48, 24, 12,  6,  6,  6,  6,  6,  6, 12, 24,240,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,248,252,  6,  3,  3,  3,  3,  7,254,252,  6,  3,  3,  3,  3,  6,252,248,  0,  0,  0,  0,  0,  0,255,255,  0,  0,  0,  0,  0,  0,248,252,  6,  3,  3,  3,  3,  3,  3,  6,252,248,  0,  0,  0,  0,  0,  0,255,255,  0,  0,  0,  0, 24, 28, 30, 27, 25, 24, 24, 24, 24, 24,255,255, 24, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,192, 96, 48, 24, 15,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,127,127,  0,  0,  0,  0,  0,  0,127,127,  0,  0,  0,  0,  0,  0,127,127,  0,  0,  0,  0,  0,  0,127,127,  0,  0,  0,  0,  0,  0,127,127,  0,  0,  0,  0,  0,  0,  0,  0,127,127,  0,  0,  0,  0,  0,  0,127,127,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,127,127,  0,  0,  0,  0,  0,  0, 96,112,120,108,102, 99, 97, 96, 96, 96, 96, 96, 96,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

oled_rotation_t oled_init_kb(oled_rotation_t rotation) {
    if (!is_keyboard_master()) {
        return OLED_ROTATION_180;  // flips the display 180 degrees if offhand
    }

    return rotation;
}

bool render_status(void) {
    // Host Keyboard Layer Status
    oled_write_P(PSTR("Layer: "), false);

    switch (get_highest_layer(layer_state)) {
        case 0:
            oled_write_P(PSTR("BASE\n"), false);
            break;
        case 1:
            oled_write_P(PSTR("LOWER\n"), false);
            break;
        case 2:
            oled_write_P(PSTR("RAISE\n"), false);
            break;
        case 3:
            oled_write_P(PSTR("ADJUST\n"), false);
            break;            
        default:
            // Or use the write_ln shortcut over adding '\n' to the end of your string
            oled_write_ln_P(PSTR("Undefined"), false);
    }

    // Host Keyboard LED Status
    led_t led_state = host_keyboard_led_state();
    oled_write_P(led_state.num_lock ? PSTR("NUM ") : PSTR("    "), false);
    oled_write_P(led_state.caps_lock ? PSTR("CAP ") : PSTR("    "), false);
    oled_write_P(led_state.scroll_lock ? PSTR("SCR ") : PSTR("    "), false);
    
    return false;
}

bool oled_task_kb(void) {
    if (!oled_task_user()) {
        return false;
    }
    if (is_keyboard_master()) {
        render_status();  // Renders the current keyboard state (layer, lock, caps, scroll, etc)
    } else {
        render_logo();  // Renders a static logo
        oled_scroll_left();  // Turns on scrolling
    }
    return false;
}

#endif

#ifdef RGB_MATRIX_ENABLE
// clang-format off
led_config_t g_led_config = { {
    {   0,   1,   2,   3,   4,   5},
    {  15,  13,  11,  10,   8,   6},
    {  16,  18,  20,  21,  23,  25},
    {  35,  33,  31,  30,  28,  26},
    {  36,  38,  40,  41,  43,  45},

    {  46,  47,  48,  49,  50,  51},
    {  61,  59,  57,  56,  54,  52},
    {  62,  64,  66,  67,  69,  71},
    {  81,  79,  77,  76,  74,  72},
    {  82,  84,  86,  87,  89,  91},
}, { 
    // not finished
    {  0,  0 }, { 10,   0}, { 20,   0}, { 30,  0}, { 40,  0}, { 50,  0}, { 60,  0}, { 70,  0}, { 80,  0}, { 90,  0}, {100,  0}, {110,  0}, {120,  0}, {130,  0}, {140,   0},
    {140, 10 }, {130,  10}, {120,  10}, {110, 10}, {100, 10}, { 90, 10}, { 80, 10}, { 70, 10}, { 60, 10}, { 50, 10}, { 40, 10}, { 30, 10}, { 20, 10}, { 10, 10}, {  0,  10},
    {  0, 20 }, { 10,  20}, { 20,  20}, { 30, 20}, { 40, 20}, { 50, 20}, { 60, 20}, { 70, 20}, { 80, 20}, { 90, 20}, {100, 20}, {110, 20}, {120, 20}, {130, 20}, {140,  20},
    {140, 30 }, {130,  30}, {120,  30}, {110, 30}, {100, 30}, { 90, 30}, { 80, 30}, { 70, 30}, { 60, 30}, { 50, 30}, { 40, 30}, { 30, 30}, { 20, 30}, { 10, 30}, {  0,  30},
    {  0, 40 }, { 10,  40}, { 20,  40}, { 30, 40}, { 40, 40}, { 50, 40}, { 60, 40}, { 70, 40}, { 80, 40}, { 90, 40}, {100, 40}, {110, 40}, {120, 40}, {130, 40}, {140,  40},
    {140, 50 }, {130,  50}, {120,  50}, {110, 50}, {100, 50}, { 90, 50}, { 85, 50}, { 80, 50}, { 70, 50}, { 60, 50}, { 55, 50}, { 50, 50}, { 40, 50}, { 30, 50}, { 20,  50}, { 10, 50}, {  0, 50},

    {  0,  0 }, { 10,   0}, { 20,   0}, { 30,  0}, { 40,  0}, { 50,  0}, { 60,  0}, { 70,  0}, { 80,  0}, { 90,  0}, {100,  0}, {110,  0}, {120,  0}, {130,  0}, {140,   0},
    {140, 10 }, {130,  10}, {120,  10}, {110, 10}, {100, 10}, { 90, 10}, { 80, 10}, { 70, 10}, { 60, 10}, { 50, 10}, { 40, 10}, { 30, 10}, { 20, 10}, { 10, 10}, {  0,  10},
    {  0, 20 }, { 10,  20}, { 20,  20}, { 30, 20}, { 40, 20}, { 50, 20}, { 60, 20}, { 70, 20}, { 80, 20}, { 90, 20}, {100, 20}, {110, 20}, {120, 20}, {130, 20}, {140,  20},
    {140, 30 }, {130,  30}, {120,  30}, {110, 30}, {100, 30}, { 90, 30}, { 80, 30}, { 70, 30}, { 60, 30}, { 50, 30}, { 40, 30}, { 30, 30}, { 20, 30}, { 10, 30}, {  0,  30},
    {  0, 40 }, { 10,  40}, { 20,  40}, { 30, 40}, { 40, 40}, { 50, 40}, { 60, 40}, { 70, 40}, { 80, 40}, { 90, 40}, {100, 40}, {110, 40}, {120, 40}, {130, 40}, {140,  40},
    {140, 50 }, {130,  50}, {120,  50}, {110, 50}, {100, 50}, { 90, 50}, { 85, 50}, { 80, 50}, { 70, 50}, { 60, 50}, { 55, 50}, { 50, 50}, { 40, 50}, { 30, 50}, { 20,  50}, { 10, 50}, {  0, 50},

}, {
    1, 4, 4, 4, 4, 4, 2, 2, 2, 2,
    1, 2, 4, 2, 4, 4, 2, 4, 2, 4,
    1, 2, 4, 2, 4, 4, 2, 4, 2, 4,
    1, 2, 4, 2, 4, 4, 2, 4, 2, 4,
    1, 2, 1, 2, 1, 1, 2, 1, 2, 1,
    4, 4, 4, 4, 4, 1, 2, 2, 2, 2,
    4, 2, 4, 2, 4, 4, 2, 4, 2, 1,
    4, 2, 4, 2, 4, 4, 2, 4, 2, 1,
    4, 2, 4, 2, 4, 4, 2, 4, 2, 1,
    1, 2, 1, 2, 1, 1, 2, 1, 2, 1,
    1, 4, 4, 4, 4, 4, 2, 2, 2, 2,
    1, 2, 4, 2, 4, 4, 2, 4, 2, 4,
    1, 2, 4, 2, 4, 4, 2, 4, 2, 4,
    1, 2, 4, 2, 4, 4, 2, 4, 2, 4,
    1, 2, 1, 2, 1, 1, 2, 1, 2, 1,
    4, 4, 4, 4, 4, 1, 2, 2, 2, 2,
    4, 2, 4, 2, 4, 4, 2, 4, 2, 1,
    4, 2, 4, 2, 4, 4, 2, 4, 2, 1,
    4, 2, 4, 2

} };
// clang-format on

void rgb_matrix_increase_flags(void)
{
    switch (rgb_matrix_get_flags()) {
        case LED_FLAG_ALL: {
            rgb_matrix_set_flags(LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER);
            rgb_matrix_set_color_all(0, 0, 0);
            }
            break;
        case LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER: {
            rgb_matrix_set_flags(LED_FLAG_UNDERGLOW);
            rgb_matrix_set_color_all(0, 0, 0);
            }
            break;
        case LED_FLAG_UNDERGLOW: {
            rgb_matrix_set_flags(LED_FLAG_NONE);
            rgb_matrix_disable_noeeprom();
            }
            break;
        default: {
            rgb_matrix_set_flags(LED_FLAG_ALL);
            rgb_matrix_enable_noeeprom();
            }
            break;
    }
}
#endif