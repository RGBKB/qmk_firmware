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
#include "mun2.h"

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
    {   0,   1,   2,   3,   4,   5,   6},
    {  13,  12,  11,  10,   9,   8,   7},
    {  14,  15,  16,  17,  18,  19,  20},
    {  27,  26,  25,  24,  23,  22,  21},
    {  28,  29,  30,  31,  32,  33,  34},

    {  50,  51,  52,  53,  54,  55,  56},
    {  63,  62,  61,  60,  59,  58,  57},
    {  64,  65,  66,  67,  68,  69,  70},
    {  77,  76,  75,  74,  73,  72,  71},
    {  78,  79,  80,  81,  82,  83,  84},
}, { 
    // Left Half
    {   0,   0 }, {  24,   0 }, {  43,   0 }, {  62,   0 }, {  81,   0 }, { 100,  0 }, { 119,   0 },
    { 119,  19 }, { 100,  19 }, {  81,  19 }, {  62,  19 }, {  43,  19 }, {  24, 19 }, {   0,  19 },
    {   0,  38 }, {  24,  38 }, {  43,  38 }, {  62,  38 }, {  81,  38 }, { 100, 38 }, { 119,  38 },
    { 119,  57 }, { 100,  57 }, {  81,  57 }, {  62,  57 }, {  43,  57 }, {  24, 57 }, {   0,  57 },
    {   0,  76 }, {  24,  76 }, {  43,  76 }, {  62,  76 }, {  81,  76 }, { 100, 76 }, { 119,  76 },

    // Left Underglow 
    {   0,  0  }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,  0  },
    {   0,  0  }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,  0  },
    {   0,  0  }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,  0  },
    
    // Right Half
    {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,  0  }, {   0,   0 },
    {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,  0  }, {   0,   0 },
    {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,  0  }, {   0,   0 },
    {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,  0  }, {   0,   0 },
    {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,  0  }, {   0,   0 },

    // Right Underglow 
    {   0,  0  }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,  0  },
    {   0,  0  }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,  0  },
    {   0,  0  }, {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,  0  },
    
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