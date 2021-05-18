#include QMK_KEYBOARD_H
//#include "raw_hid.h" // For sending data to host - doesn't work as Windows steals exclusive access to keyboards
#include "print.h" // For sending custom @!

enum keymap_layers {
	_COLEJDR,
    _NUM,
    _PUNC,
	_RGBGUI,
	_FNJ,
	_TNUM,
	_QWERTY,
	_COLEMAK,
	_GAME,
	_FN,
	_ADJUST
};

enum keymap_keycodes {
    // Disables touch processing
    TCH_TOG = SAFE_RANGE
};


// Tap-dance stuff - https://www.reddit.com/r/MechanicalKeyboards/comments/aq5a3c/qmk_tap_dancing_and_oneshot_layers_quick_demo/?utm_medium=android_app&utm_source=share
// https://github.com/walkerstop/qmk_firmware/blob/fanoe/keyboards/wheatfield/blocked65/keymaps/walker/keymap.c
// I only use this for one-shot numpad access on a shift key at the moment
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
  LSFT_OSL2 = 0
};

int cur_dance (qk_tap_dance_state_t *state);
void sft_finished (qk_tap_dance_state_t *state, void *user_data);
void sft_reset (qk_tap_dance_state_t *state, void *user_data);


// End tap-dance stuff


// Default Layers
#define COLEJDR  DF(_COLEJDR)
#define QWERTY   DF(_QWERTY)
#define COLEMAK  DF(_COLEMAK)
#define GAME     DF(_GAME)

// Toggled layers
#define RGBGUI   TG(_RGBGUI)

// Momentary Layers
#define FN       MO(_FN)
#define ADJUST   MO(_ADJUST)
#define NUM      MO(_NUM)
#define PUNC     MO(_PUNC)
//#define RGBGUI   MO(_RGBGUI)
#define FNJ      MO(_FNJ)
//#define TNUM     MO(_TNUM)

// Tap-hold keys
#define JWIN_A  LGUI_T(KC_A)
#define JALT_S  LALT_T(KC_S)
#define JCTL_I  LCTL_T(KC_I)
#define JSFT_N  LSFT_T(KC_N)
#define JSFT_T  LSFT_T(KC_T)
#define JCTL_R  LCTL_T(KC_R)
#define JALT_E  LALT_T(KC_E)
#define JWIN_O  LGUI_T(KC_O)
//#define SFT_TAB  LSFT_T(KC_TAB)
//#define CTL_SPC  LCTL_T(KC_SPC)

// Momentary layer/tap keys
#define FN_CAPS  LT(_FN, KC_CAPS)
#define FN_ESC   LT(_FN, KC_ESC)
#define SPCNUM   LT(_NUM, KC_SPC)
#define ENTPUNC  LT(_PUNC, KC_ENT)
#define TABGUI   LT(_RGBGUI, KC_TAB)
#define DOTFNJ   LT(_FNJ, KC_DOT)
#define UTNUM    LT(_TNUM, KC_U)

// Tap-dance keys
// Currently unused, but I'd like to have left and right alt on the same key (hold for left, tap for one-shot right),
// but I'll have to get more familiar with tap dance before I trust myself to execute this correctly.
#define SFTNUM  TD(LSFT_OSL2)

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
  {{0, 13}, {1, 13}, {2, 13}, {3, 13}, {4, 13}, {5, 13}, {6, 13}},
  // Right half
  {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}},
  {{0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}},
  {{0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}, {6, 2}},
  {{0, 3}, {1, 3}, {2, 3}, {3, 3}, {4, 3}, {5, 3}, {6, 3}},
  {{0, 4}, {1, 4}, {2, 4}, {3, 4}, {4, 4}, {5, 4}, {6, 4}},
  // Right half encoders (last three positions are empty on both sides)
  {{0, 5}, {1, 5}, {2, 5}, {3, 5}, {4, 5}, {5, 5}, {6, 5}},
  // Right half touch encoders (last two positions are empty on both sides)
  {{0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 6}},
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[_COLEJDR] = LAYOUT(
		RESET,   KC_UP,    KC_DOWN, KC_3,    KC_4,    KC_5,    KC_NO,   KC_NO,    KC_6,    KC_7,    KC_8,    KC_LEFT, KC_RIGHT, TCH_TOG,
		OS_ALT,  KC_NUHS,  KC_2,    KC_J,    KC_Y,    KC_C,    KC_Z,    KC_V,     KC_M,    KC_H,    KC_K,    KC_9,    KC_INS,   KC_MUTE,
		KC_NO,   KC_1,     UTNUM,   JALT_S,  JCTL_I,  JSFT_N,  KC_P,    KC_G,     JSFT_T,  JCTL_R,  JALT_E,  KC_B,    KC_0,     KC_NUBS,
		KC_SCLN, TABGUI,   JWIN_A,  KC_W,    KC_COMM, KC_F,    KC_SLSH, KC_MINS,  KC_D,    KC_L,    KC_QUOT, JWIN_O,  DOTFNJ,   KC_NO,
		KC_NO,   KC_NO,    KC_X,    KC_NO,   KC_DEL,  SWP_BCK, SPCNUM,  ENTPUNC,  SFTNUM,  KC_ESC,  KC_NO,   KC_Q,    KC_NO,    ALT_F4,

		_______, _______,  _______, _______,                                                        _______, _______, _______,  _______,
		KC_WH_D, KC_WH_U,  FNJ,     KC_CAPS, KC_NO,                                        KC_DEL,  KC_BSPC, RGBGUI,  KC_NO,    QWERTY
	),

    [_NUM] = LAYOUT(
        _______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  KC_PSCR,
        _______, _______,  _______, KC_VOLD, KC_VOLU, _______, _______, KC_PAST,  KC_P7,   KC_P8,   KC_P9,   KC_BSPC, _______,  KC_NLCK,
		_______, _______,  KC_MUTE, _______, _______, _______, KC_MNXT, KC_PSLS,  KC_P4,   KC_P5,   KC_P6,   KC_EQL,  KC_MINS,  _______,
		_______, KC_LSFT,  _______, KC_LALT, KC_LWIN, KC_MPRV, KC_MPLY, _______,  KC_P1,   KC_P2,   KC_P3,   KC_P0,   KC_PPLS,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  SFTNUM,  _______, _______, KC_PDOT, KC_COMM,  KC_RALT,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
    ),

    [_PUNC] = LAYOUT(
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,
		_______, _______,  _______, _______, KC_EXLM, KC_AT,   _______, _______,  KC_SLSH,  KC_NUHS, _______, _______, _______, _______,
		_______, _______,  _______, KC_LPRN, KC_RPRN, KC_LCBR, KC_RCBR, _______,  KC_NUBS,  KC_AMPR, KC_PAST, _______, _______, _______,
		_______, KC_LSFT,  KC_LBRC, KC_RBRC, _______, _______, _______, _______,  KC_PND,   KC_DLR,  KC_PERC, KC_SCLN, KC_GRV,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, KC_CIRC, _______, _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
    ),

    [_RGBGUI] = LAYOUT(
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  KC_HOME, KC_UP,   KC_END,  _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, RGB_TOG, KC_TAB,   KC_LEFT, KC_DOWN, KC_RIGHT,KC_PGUP, RGB_HUI,  _______,
		_______, _______,  _______, _______, RGB_VAI, RGB_VAD, _______, _______,  RGB_MOD, RGB_RMOD,RGB_SAI, _______, KC_PGDN,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  KC_LALT, _______, _______, RGB_SAD, RGB_HUD,  _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
    ),

	[_FNJ] = LAYOUT(
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,  _______, KC_F3,   KC_F4,   KC_F5,   _______, _______,  KC_F7,   KC_F8,   KC_F9,   _______, _______,  _______,
		_______, _______,  KC_F2,   _______, _______, _______, KC_F6,   _______,  _______, _______, _______, _______, _______,  _______,
		_______, KC_F1,    _______, _______, _______, _______, _______, _______,  KC_F10,  KC_F11,  KC_F12,  _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
	),

	[_TNUM] = LAYOUT(
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  KC_7,    KC_8,    KC_9,    _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  KC_4,    KC_5,    KC_6,    _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  KC_1,    KC_2,    KC_3,    _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, KC_0,    _______, _______, _______,  _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
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
		KC_LCTL, KC_LGUI, KC_LALT, RGB_TOG, ADJUST,  KC_SPC,  KC_DEL,    KC_ENT,    KC_SPC,  KC_LEFT, KC_DOWN, KC_UP,   KC_RIGHT,KC_RCTL,

		_______, _______,  _______, _______,                                                       _______, _______, _______, _______,
		KC_HOME, KC_END,   NUM,   PUNC,    KC_NO,                                        KC_DEL, KC_BSPC, RGBGUI,  KC_CAPS,  COLEJDR
	),

	[_COLEMAK] = LAYOUT(
		_______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
		_______, KC_Q,    KC_W,    KC_F,    KC_P,    KC_G,    _______, _______, KC_J,    KC_L,    KC_U,    KC_Y,    KC_SCLN, _______,
		_______, KC_A,    KC_R,    KC_S,    KC_T,    KC_D,    _______, _______, KC_H,    KC_N,    KC_E,    KC_I,    KC_O,    _______,
		_______, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    _______, _______, KC_K,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, _______,
		_______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
	),

	[_GAME] = LAYOUT(
		_______, _______, _______, _______, _______, _______, KC_F1,   KC_F5,   _______, _______, _______, _______, _______, _______,
		_______, KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_F2,   KC_F6,   KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    _______,
		_______, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_F3,   KC_F7,   KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, _______,
		_______, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_F4,   KC_F8,   KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, _______,
		_______, KC_NO,   _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
	),

	[_FN] = LAYOUT(
		_______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F11,  KC_F12,  KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  _______,
		_______, KC_HOME, KC_UP,   KC_END,  _______, _______, _______, _______, _______, KC_HOME, KC_UP,   KC_END,  KC_PSCR, KC_PGUP,
		_______, KC_LEFT, KC_DOWN, KC_RGHT, _______, _______, _______, _______, _______, KC_LEFT, KC_DOWN, KC_RGHT, KC_INS,  KC_PGDN,
		_______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
		_______, _______, _______, TCH_TOG, _______, _______, _______, _______, _______, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
	),

	[_ADJUST] = LAYOUT(
		_______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F11,  KC_F12,  KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  _______,
		_______, RGB_SAD, RGB_VAI, RGB_SAI, RESET,   _______, _______, _______, _______, KC_P7,   KC_P8,   KC_P9,   _______, _______,
		_______, RGB_HUD, RGB_VAD, RGB_HUI, _______, _______, _______, _______, _______, KC_P4,   KC_P5,   KC_P6,   _______, _______,
		_______, RGB_SPD, _______, RGB_SPI, _______, _______, _______, _______, _______, KC_P1,   KC_P2,   KC_P3,   _______, GAME,
		_______, RGB_RMOD,_______, RGB_MOD, _______, _______, _______, _______, _______, KC_P0,   KC_PDOT, KC_NLCK, QWERTY, COLEMAK,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
	),
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

static tap sfttap_state = {
  .is_press_action = true,
  .state = 0
};

void sft_finished (qk_tap_dance_state_t *state, void *user_data) {
  sfttap_state.state = cur_dance(state);
  switch (sfttap_state.state) {
    case SINGLE_TAP: set_oneshot_layer(_NUM, ONESHOT_START); clear_oneshot_layer_state(ONESHOT_PRESSED); break;
    case SINGLE_HOLD: register_code(KC_LSFT); break;
    case DOUBLE_TAP:
    	if (layer_state_is(_NUM)) {
			// If already set, then switch it off
			layer_off(_NUM);
		} else {
			// If not already set, then switch the layer on
			layer_on(_NUM);
		}
    	break;
    case DOUBLE_HOLD: register_code(KC_LSFT); layer_on(_NUM); break;
    //Last case is for fast typing. Assuming your key is `f`:
    //For example, when typing the word `buffer`, and you want to make sure that you send `ff` and not `Esc`.
    //In order to type `ff` when typing fast, the next character will have to be hit within the `TAPPING_TERM`, which by default is 200ms.
  }
}

void sft_reset (qk_tap_dance_state_t *state, void *user_data) {
  switch (sfttap_state.state) {
    case SINGLE_TAP: break;
    case SINGLE_HOLD: unregister_code(KC_LSFT); break;
    case DOUBLE_TAP: break;
    case DOUBLE_HOLD: layer_off(_NUM); unregister_code(KC_LSFT); break;
  }
  sfttap_state.state = 0;
}

qk_tap_dance_action_t tap_dance_actions[] = {
  [LSFT_OSL2]     = ACTION_TAP_DANCE_FN_ADVANCED(NULL,sft_finished, sft_reset)
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
    switch (keycode) {
        case TCH_TOG:
            touch_encoder_toggle();
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
        case TABGUI:
			print("layer rgbgui\n"); // this isn't currently handled correctly by Python/Arduino,
			// probably because tapping Tab sends two "layer rgbgui"s on one line. Should add logic
			// to check whether the layer has actually been activated before sending.
			return true;  // And process the key normally!
        case DOTFNJ:
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
		  return true;
		case RESET:
		  /* Don't allow reset from oneshot layer state */
		  if (record->event.pressed && is_oneshot_layer_active()){
			clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
			return false;
		  }
		  return true;
		// End tap-dance stuff
        default:
            return true;  // Process all other keycodes normally
    }
};

#if defined(OLED_DRIVER_ENABLE)
static void render_icon(void) {
    static const char PROGMEM font_icon[] = {
        0x9b,0x9c,0x9d,0x9e,0x9f,
        0xbb,0xbc,0xbd,0xbe,0xbf,
        0xdb,0xdc,0xdd,0xde,0xdf,0
    };
    oled_write_P(font_icon, false);
}

static void render_rgb_menu(void) {
    static char buffer[53] = {0};
    snprintf(buffer, sizeof(buffer), "Hue   %3d Satur %3d Value %3d Speed %3d Mode  %3d ",
    rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v, rgb_matrix_config.speed, rgb_matrix_config.mode);
    oled_write(buffer, false);
}

static void render_layer(void) {
    // Host Keyboard Layer Status
    oled_write_P(PSTR("Layer"), false);
    switch (get_highest_layer(layer_state)) {
        case _QWERTY:
            oled_write_ln_P(PSTR("QWERT"), false);
            break;
        case _COLEMAK:
            oled_write_ln_P(PSTR("Clmk "), false);
            break;
        case _GAME:
            oled_write_ln_P(PSTR("GAME"), false);
            break;
        case _FN:
            oled_write_ln_P(PSTR("FN   "), false);
            break;
        case _ADJUST:
            oled_write_ln_P(PSTR("ADJ  "), false);
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
    if (is_keyboard_master()) {
        render_layer();
        oled_write_P(PSTR("     "), false);
        render_leds();
        oled_write_P(PSTR("     "), false);
        render_touch();
        oled_set_cursor(0, 12);
        render_icon();
    }
    else {
        render_rgb_menu();
        oled_set_cursor(0, 12);
        render_icon();
    }
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_270;
}
#endif
