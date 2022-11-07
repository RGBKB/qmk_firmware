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
#include "zygomorph3.h"

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
    // Left Half
    {   0,   0 }, {  19,   0 }, {  38,   0 }, {  57,   0 }, {  76,   0 }, {  95,  0  },
    {  95,  19 }, {  86,  10 }, {  76,  19 }, {  67,  10 }, {  57,  19 }, {  38,  19 }, {  29,  10 }, {  19,  19 }, {  10,  10 }, {   0,  19 },
    {   0,  38 }, {  10,  29 }, {  19,  38 }, {  29,  29 }, {  38,  38 }, {  57,  38 }, {  67,  29 }, {  76,  38 }, {  86,  29 }, {  95,  38 },
    {  95,  57 }, {  86,  48 }, {  76,  57 }, {  67,  48 }, {  57,  57 }, {  38,  57 }, {  29,  48 }, {  19,  57 }, {  10,  48 }, {   0,  38 },
    {   0,  76 }, {  10,  67 }, {  19,  76 }, {  29,  67 }, {  38,  76 }, {  57,  76 }, {  67,  67 }, {  76,  76 }, {  86,  67 }, {  95,  76 },
    // Right Half
    { 114,   0 }, { 133,   0 }, { 152,   0 }, { 171,   0 }, { 190,   0 }, { 209,  0  },
    { 209,  19 }, { 200,  10 }, { 190,  19 }, { 181,  10 }, { 171,  19 }, { 152,  19 }, { 143,  10 }, { 133,  19 }, { 124,  10 }, { 114,  19 },
    { 114,  38 }, { 124,  29 }, { 133,  38 }, { 143,  29 }, { 152,  38 }, { 171,  38 }, { 181,  29 }, { 190,  38 }, { 200,  29 }, { 209,  38 },
    { 209,  57 }, { 200,  48 }, { 190,  57 }, { 181,  48 }, { 171,  57 }, { 152,  57 }, { 143,  48 }, { 133,  57 }, { 124,  48 }, { 114,  38 },
    { 114,  76 }, { 124,  67 }, { 133,  76 }, { 143,  67 }, { 152,  76 }, { 171,  76 }, { 181,  67 }, { 190,  76 }, { 200,  67 }, { 209,  76 },
}, {
    1, 4, 4, 4, 4, 4,
    1, 2, 4, 2, 4, 4, 2, 4, 2, 4,
    1, 2, 4, 2, 4, 4, 2, 4, 2, 4,
    1, 2, 4, 2, 4, 4, 2, 4, 2, 4,
    1, 2, 1, 2, 1, 1, 2, 1, 2, 1,

    4, 4, 4, 4, 4, 1,
    4, 2, 4, 2, 4, 4, 2, 4, 2, 1,
    4, 2, 4, 2, 4, 4, 2, 4, 2, 1,
    4, 2, 4, 2, 4, 4, 2, 4, 2, 1,
    1, 2, 1, 2, 1, 1, 2, 1, 2, 1

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