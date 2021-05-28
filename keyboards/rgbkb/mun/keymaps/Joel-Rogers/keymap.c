#include QMK_KEYBOARD_H
//#include "raw_hid.h" // For sending data to host - doesn't work as Windows steals exclusive access to keyboards
#include "print.h" // For sending custom @!

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
};

enum keymap_keycodes {
    // Disables touch processing
    TCH_TOG = SAFE_RANGE
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
  LYROSTO = 1,
};

int ossft = 0; // for toggling off shift after one-shot layer
int lyrlyr = 0; // for toggling off layer layer after making your selection

int cur_dance (qk_tap_dance_state_t *state);
void lyrhld_finished (qk_tap_dance_state_t *state, void *user_data);
void lyrhld_reset (qk_tap_dance_state_t *state, void *user_data);
void lyrostg_finished (qk_tap_dance_state_t *state, void *user_data);
void lyrostg_reset (qk_tap_dance_state_t *state, void *user_data);


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
#define QPUNC  LT(_PUNC, KC_Q)
//#define UTNUM    LT(_TNUM, KC_U)

// Tap-dance keys
#define SPCHLD     TD(SPCLYRHLD)
#define LYROSTG    TD(LYROSTO)

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
		OS_ALT,  KC_NUHS,  KC_2,    KC_B,    KC_Y,    KC_C,    KC_Z,    KC_V,     KC_M,    KC_H,    KC_J,    KC_9,    KC_INS,   KC_MUTE,
		KC_NO,   KC_X,     KC_U,    JALT_S,  JCTL_I,  JSFT_N,  KC_P,    KC_G,     JSFT_T,  JCTL_R,  JALT_E,  KC_K,    QPUNC,    KC_NUBS,
		KC_SCLN, TABFNJ,   JWIN_A,  KC_W,    KC_COMM, KC_F,    KC_SLSH, KC_MINS,  KC_D,    KC_L,    KC_QUOT, JWIN_O,  DOTGUI,   KC_NO,
		KC_NO,   KC_NO,    KC_1,    KC_SPC,  KC_DEL,  SWP_BCK, SPCHLD,  KC_SFTENT,LYROSTG, KC_ESC,  KC_NO,   KC_0,    KC_NO,    ALT_F4,

		_______, _______,  _______, _______,                                                        _______, _______, _______,  _______,
		KC_WH_D, KC_WH_U,  KC_RIGHT,KC_LEFT, KC_NO,                                        KC_DEL,  KC_BSPC, KC_CAPS,  KC_NO,    QWERTY
	),

    [_NUM] = LAYOUT(
        _______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  KC_PSCR,
        _______, _______,  _______, KC_VOLD, KC_VOLU, _______, _______, KC_PAST,  KC_P7,   KC_P8,   KC_P9,   _______, _______,  KC_NLCK,
		_______, _______,  KC_MUTE, _______, _______, _______, KC_MPLY, KC_PSLS,  KC_P4,   KC_P5,   KC_P6,   KC_EQL,  KC_MINS,  _______,
		_______, _______,  _______, _______, _______, KC_MPRV, KC_MNXT, KC_SCLN,  KC_P1,   KC_P2,   KC_P3,   KC_P0,   KC_BSPC,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, KC_PDOT, _______, KC_PPLS, KC_COMM,  KC_RALT,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
    ),

    [_PUNC] = LAYOUT(
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,
		_______, _______,  _______, KC_CIRC, KC_EXLM, KC_AT,   _______, _______,  KC_SLSH,  KC_AMPR, _______, _______, _______, _______,
		_______, _______,  KC_GRV,  JALT0,   JCTLLBRC,JSFTRBRC,JWINSCLN, _______, JSFTNUBS, JCTLNUHS,JALTPAST,_______,  _______, _______,
		_______, _______,  JWIN9,   _______, _______, _______, _______, _______,  KC_PND,   KC_DLR,  KC_PERC, _______, _______, _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______, _______, _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
    ),

    [_RGBGUI] = LAYOUT(
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,  _______, RGB_SAI, RGB_HUD, RGB_HUI, _______, _______,  KC_HOME, KC_UP,   KC_END,  _______, _______,  _______,
		_______, _______,  RGB_SAD, _______, _______, _______, RGB_TOG, _______,  KC_LEFT, KC_DOWN, KC_RIGHT,KC_PGUP, _______,  _______,
		_______, _______,  _______, RGB_MOD, RGB_VAI, RGB_VAD, _______, _______,  KC_BSPC, KC_DEL,  _______, KC_TAB,  KC_PGDN,  _______,
		_______, _______,  RGB_RMOD,_______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
    ),

	[_FNJ] = LAYOUT(
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,  _______, JALTF3,  JCTLF4,  JSFTF5,  KC_F6,   KC_F7,    JSFTF8,  JCTLF9,  JALTF10, _______, _______,  _______,
		_______, KC_F1,    JWINF2,  _______, _______, _______, _______, _______,  _______, _______, _______, JWINF11, KC_F12,   _______,
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

	[_ADJUST] = LAYOUT(
		_______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F11,  KC_F12,  KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  _______,
		_______, RGB_SAD, RGB_VAI, RGB_SAI, RESET,   _______, _______, _______, _______, KC_P7,   KC_P8,   KC_P9,   _______, _______,
		_______, RGB_HUD, RGB_VAD, RGB_HUI, _______, _______, _______, _______, _______, KC_P4,   KC_P5,   KC_P6,   _______, _______,
		_______, RGB_SPD, _______, RGB_SPI, _______, _______, _______, _______, _______, KC_P1,   KC_P2,   KC_P3,   _______, GAME,
		_______, RGB_RMOD,_______, RGB_MOD, _______, _______, _______, _______, _______, KC_P0,   KC_PDOT, KC_NLCK, QWERTY, COLEMAK,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
	),

	[_LYRHLD] = LAYOUT(
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,_______,TG(_RGBGUI),TG(_PUNC),TG(_NUM),_______, _______,TG(_FNJ),TG(_TNUM),_______, _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,

		_______, _______, _______, _______,                                                       _______, _______, _______, _______,
		_______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______
	),

	[_LYROS] = LAYOUT(
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,_______,OSL(_RGBGUI),OSL(_PUNC),OSL(_NUM),_______, _______,OSL(_FNJ),OSL(_TNUM),_______, _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,
		_______, _______,  _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______,  _______,

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

static tap lyrhld_state = {
  .is_press_action = true,
  .state = 0
};

static tap lyrostg_state = {
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
  }
}

void lyrhld_reset (qk_tap_dance_state_t *state, void *user_data) {
  switch (lyrhld_state.state) {
    case SINGLE_TAP:
    	break;
    case SINGLE_HOLD:
    	layer_clear();
    	break;//clears all layers on release (except base layer)
  }
  lyrhld_state.state = 0;
}

void lyrostg_finished (qk_tap_dance_state_t *state, void *user_data) {
  lyrostg_state.state = cur_dance(state);
  //layer_clear();  // This needs doing as long as the layer layer is below the layers being switched to,
  // otherwise you get trapped when using the toggle function. It does mean you can't stack layers though.
  switch (lyrostg_state.state) {
    case SINGLE_TAP:
    	set_oneshot_layer(_LYROS, ONESHOT_START);
    	clear_oneshot_layer_state(ONESHOT_PRESSED);
    	break;
    case SINGLE_HOLD:
    	layer_on(_LYRHLD);
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

void lyrostg_reset (qk_tap_dance_state_t *state, void *user_data) {
  switch (lyrostg_state.state) {
    case SINGLE_TAP:
    	break;
    case SINGLE_HOLD:
    	layer_off(_LYRHLD);
    	break;
    case DOUBLE_TAP:
        break;
  }
  lyrostg_state.state = 0;
}

qk_tap_dance_action_t tap_dance_actions[] = {
  [SPCLYRHLD]    = ACTION_TAP_DANCE_FN_ADVANCED(NULL,lyrhld_finished, lyrhld_reset),
  [LYROSTO]     = ACTION_TAP_DANCE_FN_ADVANCED(NULL,lyrostg_finished, lyrostg_reset),
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
