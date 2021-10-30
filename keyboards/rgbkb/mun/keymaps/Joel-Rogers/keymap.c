#include QMK_KEYBOARD_H
//#include "raw_hid.h" // For sending data to host - doesn't work as Windows steals exclusive access to keyboards
#include "print.h" // For sending custom @!
#include "quantum.h"// I think I need this for mousekeys and stuff?
#include "common_oled.h" // Not sure if necessary for my keymap, but meh
#include "keymap_steno.h" // For stenography!
#include <math.h> // For maths (in touch encoder mouse cursor/special key stuff) -
// no idea how much QMK knows about core C packages tbh.

enum keymap_layers {
	_COLEJDR,
	_LYRHLD,
	_LYROS,
    _NUM,
    _PUNC,
	_RGBGUI,
	_FNJ,
	_TNUM,
	_QWERTY,
	_COLEMAK,
	_GAME,
	_FN,
	_ADJUST,
	_STENO,
	_MOUSE,
};

// Keys with special (function) handling
enum keymap_keycodes {
    // Disables touch processing
    TCH_TOG = SAFE_RANGE,
	MENU_BTN,
	MENU_UP,
	MENU_DN,
	MOUSELYR,
	SPEC_MASTER,// Keys for touchbar mousekeys/cursor
	SPEC_SLAVE
};


// Tap-dance stuff - https://www.reddit.com/r/MechanicalKeyboards/comments/aq5a3c/qmk_tap_dancing_and_oneshot_layers_quick_demo/?utm_medium=android_app&utm_source=share
// https://github.com/walkerstop/qmk_firmware/blob/fanoe/keyboards/wheatfield/blocked65/keymaps/walker/keymap.c
typedef struct {
  bool is_press_action;
  int state;
} tap;

enum {
  SINGLE_TAP = 1,
  SINGLE_HOLD = 2,
  DOUBLE_TAP = 3,
  DOUBLE_HOLD = 4,
  TRIPLE_TAP = 5,
  TRIPLE_HOLD = 6
};

//Tap dance enums
enum {
  SPCLYRHLD = 0,
//LYROSTO = 1,
  LYROSSFT = 1,
  ENTLYRTO = 2,
};

int ossft = 0; // for toggling off shift after one-shot layer
int lyrlyr = 0; // for toggling off layer layer after making your selection


// Touchbar cursor/mousekey stuff
	// Core functionality variables (non-configurable)
int vertpos = 0;
int horzpos = 0;
int touchbars_touched = 0; // Needed for entering and exiting mousekey layer cleanly using the touchbar
static bool master_pressed = false; // Tracks whether touchbar is being touched at all
static bool slave_pressed = false;
static bool mouse_upping = false;
static bool mouse_downing = false;
static bool mouse_lefting = false;
static bool mouse_righting = false;

	// User-configurable/-personalisable
int spec_master_deadzone = 10; // Sets the width around the centre over which no cursor movement will be sent
int spec_slave_deadzone = 10;
int spec_master_centre = 180; // Sets the position (0-255) of the centre touchpoint, to account for using the
// touchbar without the user taking their hands off of the homerow/wherever they're comfortable.
int spec_slave_centre = 75; // Default to moderately thumb-proximal values in a left-hand master setup.

int cur_dance (qk_tap_dance_state_t *state);
void lyrhld_finished (qk_tap_dance_state_t *state, void *user_data);
void lyrhld_reset (qk_tap_dance_state_t *state, void *user_data);
//void lyrostg_finished (qk_tap_dance_state_t *state, void *user_data);
//void lyrostg_reset (qk_tap_dance_state_t *state, void *user_data);
void lyros_finished (qk_tap_dance_state_t *state, void *user_data);
void lyros_reset (qk_tap_dance_state_t *state, void *user_data);
void lyrto_finished (qk_tap_dance_state_t *state, void *user_data);
void lyrto_reset (qk_tap_dance_state_t *state, void *user_data);


// End tap-dance stuff


// Default Layers
#define COLEJDR  DF(_COLEJDR)
#define QWERTY   DF(_QWERTY)
#define STENO    DF(_STENO)
//#define COLEMAK  DF(_COLEMAK)
//#define GAME     DF(_GAME)

// Toggled layers
#define RGBGUI   TG(_RGBGUI)
//#define MOUSE    TG(_MOUSE) // This has more complex behaviour now, handled by MOUSELYR

// Momentary Layers
#define FN       MO(_FN)
#define ADJUST   MO(_ADJUST)
#define NUM      MO(_NUM)
#define PUNC     MO(_PUNC)
//#define RGBGUI   MO(_RGBGUI)
#define FNJ      MO(_FNJ)
//#define TNUM     MO(_TNUM)

// Tap-hold keys
// Main layer
#define JWIN_A  LGUI_T(KC_A)
#define JALT_S  LALT_T(KC_S)
#define JCTL_I  LCTL_T(KC_I)
#define JSFT_N  LSFT_T(KC_N)
#define JSFT_T  LSFT_T(KC_T)
#define JCTL_R  LCTL_T(KC_R)
#define JALT_E  LALT_T(KC_E)
#define JWIN_O  LGUI_T(KC_O)
// 3rd layer
#define JWIN9  LGUI_T(KC_9)
#define JALT0  LALT_T(KC_0)
#define JCTLLBRC  LCTL_T(KC_LBRC)
#define JSFTRBRC  LSFT_T(KC_RBRC)
#define JSFTNUBS  LSFT_T(KC_NUBS)
#define JCTLNUHS  LCTL_T(KC_NUHS)
#define JALTPAST  LALT_T(KC_PAST)
#define JWINSCLN  LGUI_T(KC_SCLN)
// 5th layer
#define JWINF2  LGUI_T(KC_F2)
#define JALTF3  LALT_T(KC_F3)
#define JCTLF4  LCTL_T(KC_F4)
#define JSFTF5  LSFT_T(KC_F5)
#define JSFTF8  LSFT_T(KC_F8)
#define JCTLF9  LCTL_T(KC_F9)
#define JALTF10 LALT_T(KC_F10)
#define JWINF11 LGUI_T(KC_F11)
//#define SFT_TAB  LSFT_T(KC_TAB)
//#define CTL_SPC  LCTL_T(KC_SPC)

// Momentary layer/tap keys
#define FN_CAPS  LT(_FN, KC_CAPS)
//#define FN_ESC   LT(_FN, KC_ESC)
//#define SPCNUM   LT(_NUM, KC_SPC)
//#define ENTPUNC  LT(_PUNC, KC_ENT)
#define TABFNJ   LT(_FNJ, KC_TAB)
#define DOTGUI   LT(_RGBGUI, KC_DOT)
//#define ZROPUNC  LT(_PUNC, KC_0)
#define SLSHPUNC  LT(_PUNC, KC_SLSH)
//#define UTNUM    LT(_TNUM, KC_U)

// Tap-dance keys
#define SPCHLDLYR     TD(SPCLYRHLD)
//#define LYROSTG    TD(LYROSTO)
#define OSLYRSFT    TD(LYROSSFT)
#define ENTTOLYR    TD(ENTLYRTO)

// Layerhld keys


// Mod-tap keys
#define ALT_F4 LALT(KC_F4)
#define KC_PND LSFT(KC_3)

// One-shot keys
#define OS_ALT 	OSM(MOD_RALT)

// Swap hands key (when held)
// https://docs.qmk.fm/#/feature_swap_hands
#define SWP_BCK SH_T(KC_BSPC)

// https://docs.qmk.fm/#/keycodes

const keypos_t hand_swap_config[MATRIX_ROWS][MATRIX_COLS] = {
  // Left half
  {{0, 7}, {1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7}, {6, 7}},
  {{0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8}},
  {{0, 9}, {1, 9}, {2, 9}, {3, 9}, {4, 9}, {5, 9}, {6, 9}},
  {{0, 10}, {1, 10}, {2, 10}, {3, 10}, {4, 10}, {5, 10}, {6, 10}},
  {{0, 11}, {1, 11}, {2, 11}, {3, 11}, {4, 11}, {5, 11}, {6, 11}},
  // Left half encoders (last three positions are empty on both sides)
  {{0, 12}, {1, 12}, {2, 12}, {3, 12}, {4, 12}, {5, 12}, {6, 12}},
  // Left half touch encoders (last two positions are empty on both sides)
  {{0, 13}, {1, 13}, {2, 13}, {3, 13}, {4, 13}, {5, 13}, {6, 13}},// {7, 13}},// pretty sure I can't extend this.
  // Having a special key therefore reduces the maximal possible number of touch sections (tap/hold) by one.
  // Right half
  {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}},
  {{0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}},
  {{0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}, {6, 2}},
  {{0, 3}, {1, 3}, {2, 3}, {3, 3}, {4, 3}, {5, 3}, {6, 3}},
  {{0, 4}, {1, 4}, {2, 4}, {3, 4}, {4, 4}, {5, 4}, {6, 4}},
  // Right half encoders (last three positions are empty on both sides)
  {{0, 5}, {1, 5}, {2, 5}, {3, 5}, {4, 5}, {5, 5}, {6, 5}},
  // Right half touch encoders (last two positions are empty on both sides (perhaps only true when using
  // just three touch sections??)))
  {{0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 6}}//, {7, 6}},
};


/* This keyboard is enabled with an RGB Menu Control system.
This functionality is enabled, but still requires a little configuration based on your exact setup.
The RGB Menu will appear on the Right Half's OLED and can be controlled by the MENU keycodes:
MENU_BTN - Triggers a button action for the menu
MENU_UP - Triggers an increase action for the menu
MENU_DN - Triggers a decrease action for the menu

To finish configuration for your board, you will want to change the default keycodes for an encoder on the right half.
Encoder press keycode should be set to MENU_BTN, Clockwise should be MENU_UP, and Counter Clockwise should be MENU_DN.
Depending on where you add an encoder to the right half will determin in the default keymap where you should put those keycodes.
*/

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[_COLEJDR] = LAYOUT(
		RESET,   OS_ALT,  KC_3,    KC_4,    KC_5,    KC_UP,   KC_DOWN,  KC_LEFT,  KC_RIGHT, KC_6,    KC_7,    KC_8,    KC_NO,   ALT_F4,
		KC_NUHS, KC_2,    KC_V,    KC_Y,    KC_C,    KC_Z,    TCH_TOG,  KC_MUTE,  KC_Q,     KC_M,    KC_H,    KC_MINS, KC_9,    KC_INS,
		KC_J,    KC_U,    JALT_S,  JCTL_I,  JSFT_N,  KC_P,    STENO,    KC_NO,    KC_G,     JSFT_T,  JCTL_R,  JALT_E,  KC_K,    SLSHPUNC,
		TABFNJ,  JWIN_A,  KC_W,    KC_COMM, KC_F,    KC_X,    KC_NO,    KC_NO,    KC_B,     KC_D,    KC_L,    KC_QUOT, JWIN_O,  DOTGUI,
		KC_SCLN, KC_1,    KC_NO,   KC_SPC,  SWP_BCK,SPCHLDLYR,KC_DEL,   KC_BSPC,  OSLYRSFT, ENTTOLYR,KC_ESC,  KC_NO,   KC_0,    KC_NUBS,

		_______, _______,  _______, _______,                                                        _______, _______, _______,  _______,
		KC_WH_D, KC_WH_U,  MOUSELYR, KC_RIGHT,KC_LEFT, KC_NO,                       KC_NO,    KC_DEL,  KC_BSPC, KC_CAPS,  MOUSE,    QWERTY
	),

    [_NUM] = LAYOUT(
        _______,  _______, _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, KC_PSCR,
        _______,  _______, KC_VOLD, KC_VOLU, KC_SPC,  _______, _______, _______,  KC_PAST,  KC_P7,   KC_P8,   KC_P9,   _______, KC_NLCK,
		_______,  KC_MUTE, _______, _______, _______, KC_MPLY, _______, _______,  KC_PSLS,  KC_P4,   KC_P5,   KC_P6,   KC_EQL,  KC_MINS,
		_______,  _______, _______, _______, KC_MPRV, KC_MNXT, _______, _______,  KC_SCLN,  KC_P1,   KC_P2,   KC_P3,   KC_P0,   KC_BSPC,
		_______,  _______, _______, _______, _______, _______, _______, _______,  KC_RALT,  _______, KC_PDOT, _______, KC_PPLS, KC_COMM,

		_______,  _______, _______, _______,                                                         _______, _______, _______, _______,
		_______,  _______, _______, _______, _______, _______,                    _______,  _______, _______, _______, _______, _______
    ),

    [_PUNC] = LAYOUT(
		_______,  _______, _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,
		_______,  _______, KC_CIRC, KC_EXLM, KC_AT,   _______, _______, _______,  _______,  KC_SLSH, KC_AMPR, _______, _______, _______,
		_______,  KC_GRV,  JALT0,   JCTLLBRC,JSFTRBRC,JWINSCLN,_______, _______,  _______,  JSFTNUBS,JCTLNUHS,JALTPAST,_______, _______,
		_______,  JWIN9,   _______, _______, _______, _______, _______, _______,  _______,  KC_PND,  KC_DLR,  KC_PERC, _______, _______,
		_______,  _______, _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,

		_______,  _______, _______, _______,                                                         _______, _______, _______, _______,
		_______,  _______, _______, _______, _______, _______,                    _______,  _______, _______, _______, _______, _______
    ),

    [_RGBGUI] = LAYOUT(
		_______,  _______, _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,
		_______,  _______, RGB_SAI, RGB_HUD, RGB_HUI, _______, _______, _______,  _______,  KC_HOME, KC_UP,   KC_END,  _______, _______,
		_______,  RGB_SAD, _______, _______, _______, RGB_TOG, _______, _______,  KC_SPC,   KC_LEFT, KC_DOWN, KC_RIGHT,KC_PGUP, _______,
		_______,  _______, RGB_MOD, RGB_VAI, RGB_VAD, _______, _______, _______,  _______,  KC_BSPC, KC_DEL,  _______, KC_TAB,  KC_PGDN,
		_______,  RGB_RMOD,_______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,

		_______,  _______, _______, _______,                                                         _______, _______, _______, _______,
		_______,  _______, _______, _______, _______, _______,                    _______,  _______, _______, _______, _______, _______
    ),

	[_FNJ] = LAYOUT(
		_______,  _______, _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,
		_______,  _______, _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,
		_______,  _______, JALTF3,  JCTLF4,  JSFTF5,  KC_F6,   _______, _______,  KC_F7,    JSFTF8,  JCTLF9,  JALTF10, _______, _______,
		KC_F1,    JWINF2,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, JWINF11, KC_F12,
		_______,  _______, _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,

		_______,  _______, _______, _______,                                                         _______, _______, _______, _______,
		_______,  _______, _______, _______, _______, _______,                    _______,  _______, _______, _______, _______, _______
	),

	[_TNUM] = LAYOUT(
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______,  KC_7,    KC_8,    KC_9,    _______, _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______,  KC_4,    KC_5,    KC_6,    _______, _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______,  KC_1,    KC_2,    KC_3,    _______, _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, KC_0,    _______, _______, _______,

		_______,  _______, _______, _______,                                                         _______, _______, _______, _______,
		_______,  _______, _______, _______, _______, _______,                    _______,  _______, _______, _______, _______, _______
	),
	/* QWERTY
	 * .--------------------------------------------------------------.  .--------------------------------------------------------------.
	 * | `~/ESC | 1      | 2      | 3      | 4      | 5      |   -    |  |    =   | 6      | 7      | 8      | 9      | 0      | Bckspc |
	 * |--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
	 * | Tab    | Q      | W      | E      | R      | T      |   [    |  |    ]   | Y      | U      | I      | O      | P      | \      |
	 * |--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
	 * | FN/Caps| A      | S      | D      | F      | G      |   (    |  |    )   | H      | J      | K      | L      | :      | '      |
	 * |--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
	 * | Shift( | Z      | X      | C      | V      | B      |   {    |  |    }   | N      | M      | ,      | .      | /      | )Shift |
	 * |--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
	 * | Ctrl   | Win    | Alt    | RGBTOG | FN     | Space  | Bksp   |  | Enter  | Space  | Space  | FN     | Alt    | Win    | Ctrl   |
	 * '--------+--------+--------+--------+--------+--------+--------'  '--------+--------+--------+--------+--------+--------+--------'
	 *      Encoder 1         Encoder 2                                                                  Encoder 3         Encoder 4
	 * .-----------------------------------.                                                        .-----------------------------------.
	 * | VolUp  | VolDn  | VolUp  | VolDn  |                                                        | PgUp   | PgDn   | PgUp   | PgDn   |
	 * |--------+--------+--------+--------+--------.                                      .--------+--------+--------+--------+--------|
	 * | VolDn  | VolUp  | Next   | Play   | Prev   | Touch Encoder          Touch Encoder | RgbHuI | RgbHuD | RgbMdD | RgbTog | RgbMdI |
	 * '--------+--------+--------+--------+--------'                                      '--------+--------+--------+--------+--------'
	 */
	[_QWERTY] = LAYOUT(
		KC_GESC, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_MINS,   KC_EQL,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,
		KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_LBRC,   KC_RBRC,   KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSLASH,
		FN_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_LPRN,   KC_RPRN,   KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
		KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_LCBR,   KC_RCBR,   KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_SFTENT,
		KC_LCTL, KC_LGUI, KC_LALT, COLEJDR, ADJUST,  KC_SPC,  KC_DEL,    KC_ENT,    KC_SPC,  KC_LEFT, KC_DOWN, KC_UP,   KC_RIGHT,KC_RCTL,

		_______, _______,  _______, _______,                                                       _______, _______, _______, _______,
		KC_HOME, KC_END,  NUM,     PUNC,    KC_NO,   KC_NO,                       KC_NO, KC_DEL, KC_BSPC, RGBGUI,  KC_CAPS,  COLEJDR
	),

	[_ADJUST] = LAYOUT(
		_______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F11,  KC_F12,  KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  _______,
		_______, RGB_SAD, RGB_VAI, RGB_SAI, RESET,   _______, _______, _______, _______, KC_P7,   KC_P8,   KC_P9,   _______, _______,
		_______, RGB_HUD, RGB_VAD, RGB_HUI, _______, _______, _______, _______, _______, KC_P4,   KC_P5,   KC_P6,   _______, _______,
		_______, RGB_SPD, _______, RGB_SPI, _______, _______, _______, _______, _______, KC_P1,   KC_P2,   KC_P3,   _______, _______,
		_______, RGB_RMOD,_______, RGB_MOD, _______, _______, _______, _______, _______, KC_P0,   KC_PDOT, KC_NLCK, QWERTY, _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______
	),

	[_LYRHLD] = LAYOUT(
		_______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______, _______,
		_______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______, _______,
		_______,  _______,TG(_RGBGUI),TG(_PUNC),TG(_NUM),_______,_______, _______,_______,TG(_FNJ),TG(_TNUM),_______,  _______, _______,
		_______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______, _______,
		_______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______, _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,  _______,                    _______, _______, _______, _______, _______, _______
	),

	[_LYROS] = LAYOUT(
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,OSL(_RGBGUI),OSL(_PUNC),OSL(_NUM),_______, _______,_______,_______,OSL(_FNJ),OSL(_TNUM),_______, _______, _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,  _______,                    _______, _______, _______, _______, _______, _______
	),
	[_STENO] = LAYOUT(
		XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  XXXXXXX,
		XXXXXXX, XXXXXXX,  STN_TL,  STN_PL,  STN_HL,  STN_ST1, XXXXXXX, XXXXXXX,  STN_ST3, STN_FR,  STN_PR,  STN_LR,  XXXXXXX,  XXXXXXX,
		XXXXXXX, STN_S1,   STN_KL,  STN_WL,  STN_RL,  XXXXXXX, COLEJDR, XXXXXXX,  XXXXXXX, STN_RR,  STN_BR,  STN_GR,  STN_TR,   STN_DR,
		XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, STN_SR,   STN_ZR,
		XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX, STN_N1,  STN_A,   STN_O,   STN_E,    STN_U,   STN_N2,  XXXXXXX, XXXXXXX, XXXXXXX,  XXXXXXX,

		XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                                                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
		XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  _______,                    _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
	),
	// This layer is not yet complete (particularly RE: touchbar and mouse keys). Will work on later.
	[_MOUSE] = LAYOUT(
		_______,  _______, _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,
		_______,  _______, _______, KC_PGUP, KC_HOME, KC_END,  _______, _______,  _______,  _______, _______, _______, _______, _______,
		_______,  KC_UP,   _______, KC_PGDN, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,
		KC_LEFT,  KC_DOWN, KC_RIGHT,_______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,
		_______,  _______, _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  SPEC_MASTER,                    SPEC_SLAVE,  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
	),
};



// Touchbar mousekey stuff
void touch_encoder_raw_position(uint8_t touch_handedness, uint8_t position) {
	// The sole purpose of this function is to be called from within touch_encoder.c
	// whenever the touchbar updates its position,
	// hence updating a keymap.c local variable for position. This local variable should then be used
	// in the internal logic of specially made "keycodes"/functions for the mousekey layer of the
	// touchbar (both swipe and tap/hold buttons!), for working out mouse cursor speed..
	xprintf("raw position %d %d\n", index, raw);
	// If the key's not being pressed/held/whatever, then don't need to do anything.
	if (touch_handedness && master_pressed) {
		vertpos = abs(position - spec_master_centre) - spec_master_deadzone; // Gets the useful value of the position,
		// only greater than zero if further from the centre than the deadzone value.
		if (vertpos > 0) { // Then something needs to be being held
			mk_max_speed_vert = pow(vertpos,2); // need to update the mouse cursor speed (vertical)
			// whatever the direction
			if (position > spec_master_centre) { // Then touch is 'clockwise'/rightwards from centre -
				// this code wants this to mean mouse cursor moves upwards
				if (!mouse_upping) {// Then need to start pressing the key
					// Could maybe skip these booleans/checks, but would be sending a lot of register_code's
					// unnecessarily - not sure what the computational cost of this is.
					register_code(KC_MS_UP);
					mouse_upping = true; // And toggle this boolean on, to record that
					if (mouse_downing) { // And toggle off downwards movement, if necessary
						unregister_code(KC_MS_DOWN);
						mouse_downing = false;
					}
				}
			} else { // Then touch is 'anti-clockwise'/leftwards from centre
				if (!mouse_downing) {// Then need to start pressing the key
					register_code(KC_MS_DOWN);
					mouse_downing = true; // And toggle this boolean on, to record that
					if (mouse_upping) { // And toggle off downwards movement, if necessary
						unregister_code(KC_MS_UP);
						mouse_upping = false;
					}
				}
			}
		} else {
			mk_max_speed_vert = 0; // Effectively disables (vertical) mousekey movement.
			// NOTE THAT THIS NEEDS TO BE SPLIT INTO horizontal and vertical in quantum/mousekey.c!
		}
	}
	else if (!touch_handedness && slave_pressed) {
		horzpos = abs(position - spec_slave_centre) - spec_slave_deadzone; // Gets the useful value of the position,
		// only greater than zero if further from the centre than the deadzone value.
		if (horzpos > 0) { // Then something needs to be being held
			mk_max_speed_horz = pow(horzpos,2); // need to update the mouse cursor speed (horizontal)
			// whatever the direction
			if (position > spec_slave_centre) { // Then touch is 'clockwise'/rightwards from centre -
				// this code wants this to mean mouse cursor moves rightwards
				if (!mouse_righting) {// Then need to start pressing the key
					// Could maybe skip these booleans/checks, but would be sending a lot of register_code's
					// unnecessarily - not sure what the computational cost of this is.
					register_code(KC_MS_RIGHT);
					mouse_righting = true; // And toggle this boolean on, to record that
					if (mouse_lefting) { // And toggle off downwards movement, if necessary
						unregister_code(KC_MS_LEFT);
						mouse_lefting = false;
					}
				}
			} else { // Then touch is 'anti-clockwise'/leftwards from centre
				if (!mouse_lefting) {// Then need to start pressing the key
					register_code(KC_MS_LEFT);
					mouse_lefting = true; // And toggle this boolean on, to record that
					if (mouse_righting) { // And toggle off downwards movement, if necessary
						unregister_code(KC_MS_RIGHT);
						mouse_righting = false;
					}
				}
			}
		} else {
			mk_max_speed_horz = 0; // Effectively disables (horizontal) mousekey movement.
			// NOTE THAT THIS NEEDS TO BE SPLIT INTO horizontal and vertical in quantum/mousekey.c!
		}
	}
}


//Steno initialisation
void matrix_init_user(void) {
  // ...
  steno_set_mode(STENO_MODE_GEMINI);
};


// Tap-dance stuff

int cur_dance (qk_tap_dance_state_t *state) {
  if (state->count == 1) {
    if (state->pressed) return SINGLE_HOLD;
    else return SINGLE_TAP;
  }
  else if (state->count == 2) {
    if (state->pressed) return DOUBLE_HOLD;
    else return DOUBLE_TAP;
  }
  else if (state->count == 3) {
    if (state->interrupted || !state->pressed)  return TRIPLE_TAP;
    else return TRIPLE_HOLD;
  }
  else return 8;
}

static tap lyrhld_state = {
  .is_press_action = true,
  .state = 0
};

static tap lyros_state = {
  .is_press_action = true,
  .state = 0
};

static tap lyrto_state = {
  .is_press_action = true,
  .state = 0
};

void lyrhld_finished (qk_tap_dance_state_t *state, void *user_data) {
  lyrhld_state.state = cur_dance(state);
  switch (lyrhld_state.state) {
    case SINGLE_TAP:
    	tap_code(KC_SPC);
    	break;
    case SINGLE_HOLD:
    	layer_on(_LYRHLD); // note that this will mask any lower layers! I'm yet to find any solution which
    	// avoids this (one shot doesn't work)
    	lyrlyr = 1;  // this might work to avoid masking of any lower layers - it results in the layer being disabled on the next key-up event
    	// (which in proper usage should be the layer selection keypress)
    	break;
    default: ;//in any other case, just send repeated Space's!
		int i;
		for (i = 1; i < state->count + 1; i++){
			tap_code(KC_SPC);
		}
		break;
  }
}

void lyrhld_reset (qk_tap_dance_state_t *state, void *user_data) {
  switch (lyrhld_state.state) {
    case SINGLE_TAP:
    	break;
    case SINGLE_HOLD:
    	layer_clear();
    	break;//clears all layers on release (except base layer)
    default:
    	break;
  }
  lyrhld_state.state = 0;
}

//void lyrostg_finished (qk_tap_dance_state_t *state, void *user_data) {
//  lyrostg_state.state = cur_dance(state);
//  //layer_clear();  // This needs doing as long as the layer layer is below the layers being switched to,
//  // otherwise you get trapped when using the toggle function. It does mean you can't stack layers though.
//  switch (lyrostg_state.state) {
//    case SINGLE_TAP:
//    	set_oneshot_layer(_LYROS, ONESHOT_START);
//    	clear_oneshot_layer_state(ONESHOT_PRESSED);
//    	break;
//    case SINGLE_HOLD:
//    	layer_on(_LYRHLD);
//    	break; // I reckon these can share a layer
//    case DOUBLE_TAP:
//    	set_oneshot_layer(_LYROS, ONESHOT_START);
//    	register_code(KC_LSFT);
//    	ossft = 2;  // This is just a way to get around oneshot not allowing mod presses - by setting this variable to
//    	// 2 here, shift will be unregistered after two more key-ups (usually a layer-select plus on-layer keystroke).
//		clear_oneshot_layer_state(ONESHOT_PRESSED);
//		break;
//  }
//}
//
//void lyrostg_reset (qk_tap_dance_state_t *state, void *user_data) {
//  switch (lyrostg_state.state) {
//    case SINGLE_TAP:
//    	break;
//    case SINGLE_HOLD:
//    	layer_off(_LYRHLD);
//    	break;
//    case DOUBLE_TAP:
//        break;
//  }
//  lyrostg_state.state = 0;
//}


void lyrto_finished (qk_tap_dance_state_t *state, void *user_data) {
  lyrto_state.state = cur_dance(state);
  //layer_clear();  // This needs doing as long as the layer layer is below the layers being switched to,
  // otherwise you get trapped when using the toggle function. It does mean you can't stack layers though.
  switch (lyrto_state.state) {
    case SINGLE_TAP:
    	tap_code(KC_ENT);
    	break;
    case SINGLE_HOLD:
    	layer_on(_LYRHLD);
    	break; // I reckon these can share a layer
    default: ;//in any other case, just send repeated Enters!
    	int i;
    	for (i = 1; i < state->count + 1; i++){
    		tap_code(KC_ENT);
    	}
    	break;
  }
}

void lyrto_reset (qk_tap_dance_state_t *state, void *user_data) {
  switch (lyrto_state.state) {
    case SINGLE_TAP:
    	break;
    case SINGLE_HOLD:
    	layer_off(_LYRHLD);
    	break;
    default:
        break;
  }
  lyrto_state.state = 0;
}


void lyros_finished (qk_tap_dance_state_t *state, void *user_data) {
  lyros_state.state = cur_dance(state);
  //layer_clear();  // This needs doing as long as the layer layer is below the layers being switched to,
  // otherwise you get trapped when using the toggle function. It does mean you can't stack layers though.
  switch (lyros_state.state) {
    case SINGLE_TAP:
    	set_oneshot_layer(_LYROS, ONESHOT_START);
    	clear_oneshot_layer_state(ONESHOT_PRESSED);
    	break;
    case SINGLE_HOLD:
    	register_code(KC_LSFT);
    	break; // I reckon these can share a layer
    case DOUBLE_TAP:
    	set_oneshot_layer(_LYROS, ONESHOT_START);
    	register_code(KC_LSFT);
    	ossft = 2;  // This is just a way to get around oneshot not allowing mod presses - by setting this variable to
    	// 2 here, shift will be unregistered after two more key-ups (usually a layer-select plus on-layer keystroke).
		clear_oneshot_layer_state(ONESHOT_PRESSED);
		break;
  }
}

void lyros_reset (qk_tap_dance_state_t *state, void *user_data) {
  switch (lyros_state.state) {
    case SINGLE_TAP:
    	break;
    case SINGLE_HOLD:
    	unregister_code(KC_LSFT);
    	break;
    case DOUBLE_TAP:
        break;
  }
  lyros_state.state = 0;
}
//
//
qk_tap_dance_action_t tap_dance_actions[] = {
  [SPCLYRHLD]    = ACTION_TAP_DANCE_FN_ADVANCED(NULL,lyrhld_finished, lyrhld_reset),
  [LYROSSFT]     = ACTION_TAP_DANCE_FN_ADVANCED(NULL,lyros_finished, lyros_reset),
  [ENTLYRTO]     = ACTION_TAP_DANCE_FN_ADVANCED(NULL,lyrto_finished, lyrto_reset),
};

// End tap-dance stuff


void keyboard_post_init_user(void) {
  // Customise these values to desired behaviour
  //debug_enable=true;// enables relatively extensive debugging output
  //debug_matrix=true;
  //debug_keyboard=true;
  //debug_mouse=true;
}


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
	if (ossft){// check status on each keypress
		if (!record->event.pressed) {//on key-up (skips mods as long as they're held)
			ossft -= 1;// reduce by one on each non-mod keypress
			if (!ossft){
				unregister_code(KC_LSFT);
			}
		}
	}
	if (lyrlyr){// check status on each keypress
		if (!record->event.pressed) {//on key-up (skips mods as long as they're held)
			lyrlyr -= 1;// reduce by one on each non-mod keypress
			if (!lyrlyr){
				layer_off(_LYRHLD);
			}
		}
	}
    switch (keycode) {
    	case MENU_BTN:
			if (record->event.pressed) {
				rgb_menu_selection();
			}
			return false;
		case MENU_UP:
			if (record->event.pressed) {
				rgb_menu_action(true);
			}
			return false;
		case MENU_DN:
			if (record->event.pressed) {
				rgb_menu_action(false);
			}
			return false;
		case MOUSELYR:
			if (!record->event.pressed) {// On key release
				TG(_MOUSE); // Go to mousekeys layer
			}
		case SPEC_MASTER: // Switches for toggling on/off booleans for special touchbar keys
			master_pressed = record->event.pressed;
			if (record->event.pressed) {
				touchbars_touched += 1;
			} else {
				// Release any relevant mousekeys being held on this side
				if (mouse_upping) {
					unregister_code(KC_MS_UP);
					mouse_upping = false;
				}
				if (mouse_downing) {
					unregister_code(KC_MS_DOWN);
					mouse_downing = false;
				}
				touchbars_touched -= 1;
				if (touchbars_touched = 0) { // Then both touchbars have been released, and the mousekey layer
					// can be disabled
					layer_off(_MOUSE);
				}
			}
			return false; // Nothing else to process
		case SPEC_SLAVE:
			slave_pressed = record->event.pressed;
			if (record->event.pressed) {
				touchbars_touched += 1;
			} else {
				if (mouse_righting) {
					unregister_code(KC_MS_RIGHT);
					mouse_righting = false;
				}
				if (mouse_lefting) {
					unregister_code(KC_MS_LEFT);
					mouse_lefting = false;
				}
				touchbars_touched -= 1;
				if (touchbars_touched = 0) { // Then both touchbars have been released, and the mousekey layer
					// can be disabled
					layer_off(_MOUSE);
				}
			}
			return false;
        case TCH_TOG:
        	//if (record->event.pressed) { // Necessary/helpful?
            	touch_encoder_toggle();
            //} // ??
            return false;  // Skip all further processing of this key
//        case NUM:
//			print("layer num\n");
//			return true;  // And process the key normally!
//        case PUNC:
//			print("layer punc\n");
//			return true;  // And process the key normally!
//		case RGBGUI:
//			print("layer rgbgui\n");
//			return true;  // And process the key normally!
//        case ENTNUM:
//			print("layer num\n");
//			return true;  // And process the key normally!
//        case SPCPUNC:
//			print("layer punc\n");
//			return true;  // And process the key normally!
        case DOTGUI:
			print("layer rgbgui\n"); // this isn't currently handled correctly by Python/Arduino,
			// probably because tapping Tab sends two "layer rgbgui"s on one line. Should add logic
			// to check whether the layer has actually been activated before sending.
			return true;  // And process the key normally!
        case TABFNJ:
			print("layer fnj\n");
			return true;  // And process the key normally!
        case KC_CAPS:
			if (record->event.pressed) {
				// when keycode is pressed
				print("layer caps\n");
			}
			return true;  // And process the key normally!
		// Tap-dance stuff
        case KC_TRNS:
		case KC_NO:
		  /* Always cancel one-shot layer when another key gets pressed */
		  if (record->event.pressed && is_oneshot_layer_active())
		  clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
	      unregister_code(KC_LSFT);  // and disable shift (if active) - NOT CONFIDENT THIS WON'T BREAK THINGS!
	      ossft = 0;
		  return true;
		case RESET:
		  /* Don't allow reset from oneshot layer state */
		  if (record->event.pressed && is_oneshot_layer_active()){
			clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
			unregister_code(KC_LSFT);  // and disable shift (if active) - NOT CONFIDENT THIS WON'T BREAK THINGS!
			ossft = 0;
			return false;
		  }
		  return true;
		// End tap-dance stuff
        default:
            return true;  // Process all other keycodes normally
    }
};

//#if defined(OLED_DRIVER_ENABLE) //Everything below this is new stuff/ modified from previous OLED handling
static void render_layer(void) {
    // Host Keyboard Layer Status
    oled_write_P(PSTR("Layer"), false);
    switch (get_highest_layer(layer_state)) {
        case _QWERTY:
            oled_write_ln_P(PSTR("QWRTY"), false);
            break;
        case _COLEMAK:
            oled_write_ln_P(PSTR("Colemk"), false);
            break;
        case _GAME:
            oled_write_ln_P(PSTR("Game  "), false);
            break;
        case _FN:
            oled_write_ln_P(PSTR("FN   "), false);
            break;
        case _ADJUST:
            oled_write_ln_P(PSTR("Adjst"), false);
            break;
        default:
            oled_write_ln_P(PSTR("Undef"), false);
    }
}

static void render_leds(void)
{
    // Host Keyboard LED Status
    led_t led_state = host_keyboard_led_state();
    oled_write_P(led_state.num_lock ? PSTR("NUMLK")     : PSTR("     "), false);
    oled_write_P(led_state.caps_lock ? PSTR("CAPLK")    : PSTR("     "), false);
    oled_write_P(led_state.scroll_lock ? PSTR("SCRLK")  : PSTR("     "), false);
}

static void render_touch(void)
{
    // Host Touch LED Status
    oled_write_P(!touch_encoder_toggled() ? PSTR("TOUCH")  : PSTR("     "), false);
    oled_write_P(touch_encoder_calibrating() ? PSTR("CLBRT")  : PSTR("     "), false);
}

void oled_task_user(void) {
    if (is_keyboard_left()) {
        render_icon();
        oled_write_P(PSTR("     "), false);
        render_layer();
        oled_write_P(PSTR("     "), false);
        render_leds();
        oled_write_P(PSTR("     "), false);
        render_touch();
    }
    else {
        render_icon();
        oled_write_P(PSTR("     "), false);
        render_rgb_menu();
    }
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_270;
}
//#endif
