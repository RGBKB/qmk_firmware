/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <https://github.com/XScorpion2> wrote this file.  As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day, and
 * you think this stuff is worth it, you can buy me a beer in return. Ryan Caltabiano
 * ----------------------------------------------------------------------------
 */


// TO-DO:
// I need to either fundamentally understand how arrays within structs are handled in
// terms of memory allocation and pointers (especially as passed to functions), OR
// refactor those arrays so that they're no longer parts of structs at all (this would
// likely make for more ugly code, but it's simpler and might take less time/skill to
// bugfix.

#include "i2c_master.h"
#include "keyboard.h"
#include "touch_encoder.h"
#include "print.h"
#include "wait.h"
#include "timer.h"

// for memcpy
#include <string.h>
#include <transactions.h>

#define I2C_ADDRESS 0x1C
#define CALIBRATION_BIT 0x80
#define OVERFLOW_BIT 0x40
#define SLIDER_BIT 0x02

#ifndef TOUCH_UPDATE_INTERVAL // how many milliseconds to wait between updating the touchbar
#   define TOUCH_UPDATE_INTERVAL 33
#endif

#ifndef TOUCH_UPDATE_SECTION_TIMEOUT // For how many touchbar_update cycles should a section not have a position recorded
// in it before it is considered not-being-touched (i.e. released, or the start of a swipe). Compromise between
// fast swipe responsiveness (low cycle number) and avoidance of accidental/"false" swiping when more than one
// touchbar section is being pressed simultaneously.
#   define TOUCH_UPDATE_SECTION_TIMEOUT 1
// Defaults to one (which is also the MINIMUM allowable value, or the program will do weird things),
// which disables multi-touch capabilities in favour of optimal fast swipe recognition.
#endif

enum {  // QT2120 registers
    QT_CHIP_ID = 0,
    QT_FIRMWARE_VERSION,
    QT_DETECTION_STATUS,
    QT_KEY_STATUS,
    QT_SLIDER_POSITION = 5,
    QT_CALIBRATE,
    QT_RESET,
    QT_LP,
    QT_TTD,
    QT_ATD,
    QT_DI,
    QT_TRD,
    QT_DHT,
    QT_SLIDER_OPTION,
    QT_CHARDE_TIME,
    QT_KEY0_DTHR,
    QT_KEY1_DTHR,
    QT_KEY2_DTHR,
    QT_KEY3_DTHR,
    QT_KEY4_DTHR,
    QT_KEY5_DTHR,
    QT_KEY6_DTHR,
    QT_KEY7_DTHR,
    QT_KEY8_DTHR,
    QT_KEY9_DTHR,
    QT_KEY10_DTHR,
    QT_KEY11_DTHR,
    QT_KEY0_CTRL,
    QT_KEY1_CTRL,
    QT_KEY2_CTRL,
    QT_KEY3_CTRL,
    QT_KEY4_CTRL,
    QT_KEY5_CTRL,
    QT_KEY6_CTRL,
    QT_KEY7_CTRL,
    QT_KEY8_CTRL,
    QT_KEY9_CTRL,
    QT_KEY10_CTRL,
    QT_KEY11_CTRL,
    QT_KEY0_PULSE_SCALE,
    QT_KEY1_PULSE_SCALE,
    QT_KEY2_PULSE_SCALE,
    QT_KEY3_PULSE_SCALE,
    QT_KEY4_PULSE_SCALE,
    QT_KEY5_PULSE_SCALE,
    QT_KEY6_PULSE_SCALE,
    QT_KEY7_PULSE_SCALE,
    QT_KEY8_PULSE_SCALE,
    QT_KEY9_PULSE_SCALE,
    QT_KEY10_PULSE_SCALE,
    QT_KEY11_PULSE_SCALE,
    QT_KEY0_SIGNAL,
    QT_KEY1_SIGNAL     = 54,
    QT_KEY2_SIGNAL     = 56,
    QT_KEY3_SIGNAL     = 58,
    QT_KEY4_SIGNAL     = 60,
    QT_KEY5_SIGNAL     = 62,
    QT_KEY6_SIGNAL     = 64,
    QT_KEY7_SIGNAL     = 66,
    QT_KEY8_SIGNAL     = 68,
    QT_KEY9_SIGNAL     = 70,
    QT_KEY10_SIGNAL    = 72,
    QT_KEY11_SIGNAL    = 74,
    QT_KEY0_REFERENCE  = 76,
    QT_KEY1_REFERENCE  = 78,
    QT_KEY2_REFERENCE  = 80,
    QT_KEY3_REFERENCE  = 82,
    QT_KEY4_REFERENCE  = 84,
    QT_KEY5_REFERENCE  = 86,
    QT_KEY6_REFERENCE  = 88,
    QT_KEY7_REFERENCE  = 90,
    QT_KEY8_REFERENCE  = 92,
    QT_KEY9_REFERENCE  = 94,
    QT_KEY10_REFERENCE = 96,
    QT_KEY11_REFERENCE = 98,
};

bool     touch_initialized  = false;
bool     touch_disabled = false;


uint8_t  touch_handness = 0;
// touch_raw & touch_processed store the Detection Status, Key Status (x2), and Slider Position values
uint8_t  touch_raw[4]       = { 0 };
uint8_t  touch_processed[4] = { 0 };


// Tracks how many update cycles since each section (per touchbar) last registered a touch,
// for tap/hold timeout purposes. Initialise to maximum so all are considered "fully timed out"
// (i.e. already released/handled)
// Probably now superseded by half_touch_status_t, below
//uint8_t  cycles_since_section_last_touched_master[TOUCH_SEGMENTS]       = { TOUCH_UPDATE_SECTION_TIMEOUT };
//uint8_t  cycles_since_section_last_touched_slave[TOUCH_SEGMENTS]       = { TOUCH_UPDATE_SECTION_TIMEOUT };

// Keeps a record of when the touch of this section first started, for tap-hold recognition logic.
// NOTE - can I skip this logic entirely, and just call the keydown and key-up functions at appropriate times to let QMK sort
// it out???!!!!
//uint16_t  time_section_first_touched_master[TOUCH_SEGMENTS]       = { 0 }; // need initialising during touchbar init (to e.g. timer_read())
//uint16_t  time_section_first_touched_slave[TOUCH_SEGMENTS]       = { 0 };

// For tracking when position encoder was last checked (and hence when the more complex code needs running again).
uint16_t touch_update_timer = 0;

typedef struct {
	uint8_t position; // Need to change nearly all current occurrences of this to 'initial_position', and
	// use this one only for slave-master transport (i.e. refactor code so that this replaces touch_raw
	// in a both-halves-friendly way)!!!!
	uint8_t taps; // Duplicating these attributes here to facilitate code refactoring later
	uint8_t was_swiping; // per touchbar, only zeroth bit is ever toggled - records logic from previous relevant update.
	uint8_t was_touching; // one bit per section (i.e. usually first 3 bits are toggled)
	uint8_t swipes; // holds data relevant to current relevant update. Synchronised at certain key moments with was_ attributes
	uint8_t holds;
	// I have a suspicion that I can't use lists/arrays within a struct, and/or that I can't initialise them
	// (below) as simply as '0'; will refactor if necessarys
	uint8_t cycles_since_section_last_touched[TOUCH_SEGMENTS];// = { TOUCH_UPDATE_SECTION_TIMEOUT + 1}; // All sections start at max value, indicating
	// not touched within the two most recent timeout periods. (within the last one most recent timeout period
	// indicates that button has JUST timed out and needs to be handled). Can't initialise to anything other than zero here, so handle below.
	uint16_t time_section_first_touched[TOUCH_SEGMENTS];// = 0 // needs initialising during touchbar init (to e.g. timer_read()) - actually doesn't matter,
	// I'll just combine it with a check for was_touching and/or holds, as appropriate
	uint16_t position_section_first_touched[TOUCH_SEGMENTS];// = 0 // Used to initiate swipes slightly quicker
	// on the basis of touching two points too far apart within one section.
} half_touch_status_t;

// For split transport only
typedef struct {
    uint8_t needs_tapping;
    uint8_t needs_pressing;
    uint8_t needs_releasing;
    uint8_t needs_swiping; // Flags signalling whether the touch encoder has to send a keypress (if master),
    // with the section communicated by which bit is toggled
    int8_t delt; // Signals the delta for slide motions (can be positive or negative??)
    uint8_t posn; // Purely for special-key handling purposes (most users won't need this)
    uint8_t special_press;
	uint8_t special_release; // Two flags deciding when the "special key" (i.e. the key that indicates when
	// the touchbar has started or ended being touched) needs pressing or releasing. I would have liked
	// to use a mask for this (i.e. just assign the key to the number of touchbar sections + 1), but
	// this would require me to refactor the touch_encoder.h/rev1.c code.
} half_touch_flags_t;

bool touch_slave_init = false;
// I have a sneaking suspicion that I could refactor this code to take account of the fact that each half runs
// this code separately - i.e. you only need touch_half_flags/state_j, one per side, plus a half_prev_flags
// and a slave_prev flags for the master side.
//half_touch_flags_t touch_slave_flags_j       = { 0, 0, 0, 0, 0, 0 };
//half_touch_flags_t touch_master_flags_j      = { 0, 0, 0, 0, 0, 0 };
//half_touch_flags_t touch_master_prev_flags_j = { 0, 0, 0, 0, 0, 0 }; // Slightly less memory efficient (could just send
//// the keystrokes at the appropriate point in the master half's code), but I think the code is easier
//// to follow this way.
//half_touch_status_t touch_master_state_j = { 0, 0, 0, 0, 0, 0, 0, 0 }; // JDR tweak
//half_touch_status_t touch_slave_state_j  = { 0, 0, 0, 0, 0, 0, 0, 0 }; // JDR tweak
//half_touch_status_t *half_touch_states_j[] = { &touch_master_state_j, &touch_slave_state_j } // Just an array containing pointers to the objects for easy iteration.
// ^ potentially unnecessary? Never used, I think

// Declare and initialise structs
//uint8_t default_cycles_since_section_last_touched[TOUCH_SEGMENTS] = { 0 };
//__attribute__((constructor))
//void initialise
//for (uint8_t i = 0; i < TOUCH_SEGMENTS; i++) {
//	default_cycles_since_section_last_touched[i] = TOUCH_UPDATE_SECTION_TIMEOUT + 1;
//}
//uint8_t default_cycles_since_section_last_touched[TOUCH_SEGMENTS] = {TOUCH_UPDATE_SECTION_TIMEOUT + 1 };
uint8_t default_time_position_section_first_touched[TOUCH_SEGMENTS] = { 0 };
half_touch_status_t touch_half_state_j;//         = { // try not initialising yet, should all be zero by default anyway
//		.position = 0, .taps = 0, .was_swiping = 0, .was_touching = 0, .swipes = 0, .holds = 0,
//		.cycles_since_section_last_touched = default_cycles_since_section_last_touched,
//		->time_position_section_first_touched = default_time_position_section_first_touched,
//		->time_position_section_first_touched = default_time_position_section_first_touched }; // Used on both halves (separately)
static bool structs_initialised = false; // So that structs can be initialised before the first time
// they're needed, but using a for loop (so I can't do it here - needs to be inside a function :/)
half_touch_flags_t touch_half_flags_j          = { 0, 0, 0, 0, 0, 0, 0, 0 }; // Used on both halves (separately)
half_touch_flags_t touch_master_prev_flags_j   = { 0, 0, 0, 0, 0, 0, 0, 0 }; // Unused on slave side
half_touch_flags_t touch_slave_prev_flags_j    = { 0, 0, 0, 0, 0, 0, 0, 0 }; // Unused on slave side

//memset(touch_half_state_j->cycles_since_section_last_touched, TOUCH_UPDATE_SECTION_TIMEOUT + 1, sizeof *touch_half_state_j->cycles_since_section_last_touched);


static bool write_register8(uint8_t address, uint8_t data) {
    i2c_status_t status = i2c_writeReg((I2C_ADDRESS << 1), address, &data, sizeof(data), I2C_TIMEOUT);
    if (status != I2C_STATUS_SUCCESS) {
        xprintf("write_register8 %d failed %d\n", address, status);
    }
    return status == I2C_STATUS_SUCCESS;
}

static bool read_register(uint8_t address, uint8_t* data, uint16_t length) {
    i2c_status_t status = i2c_readReg((I2C_ADDRESS << 1), address, data, length, I2C_TIMEOUT);
    if (status != I2C_STATUS_SUCCESS) {
        xprintf("read_register %d failed %d\n", address, status);
        return false;
    }
    return true;
}

void touch_encoder_init(void) {
    i2c_init();

    touch_handness = is_keyboard_left() ? 0 : 1;

    // Set QT to slider mode
    touch_initialized = write_register8(QT_SLIDER_OPTION, 0x80);
    touch_initialized &= write_register8(QT_TTD, 4);  // Toward Drift - 20 @ 3.2s
    touch_initialized &= write_register8(QT_ATD, 1);  // Away Drift - 5 @ 0.8s
    touch_initialized &= write_register8(QT_DI, 4);   // Detection Integrator - 4
    touch_initialized &= write_register8(QT_TRD, 0);  // Touch Recall - 48
    touch_encoder_calibrate();
}


// These can only be used by the master half, as the slave half cannot send keypresses
__attribute__((weak)) bool touch_encoder_tapped_kb(uint8_t index, uint8_t section) { return touch_encoder_tapped_user(index, section); }
__attribute__((weak)) bool touch_encoder_update_kb(uint8_t index, bool clockwise, int8_t delta) { return touch_encoder_update_user(index, clockwise, delta); }
__attribute__((weak)) bool touch_encoder_tapped_user(uint8_t index, uint8_t section) { return true; }
__attribute__((weak)) bool touch_encoder_update_user(uint8_t index, bool clockwise, int8_t delta) { return true; }


// JDR tweaks (touchbar)
__attribute__((weak)) bool touch_encoder_holding_kb(uint8_t index, uint8_t section) {return touch_encoder_holding_user(index, section); }
__attribute__((weak)) bool touch_encoder_released_kb(uint8_t index, uint8_t section) {return touch_encoder_released_user(index, section); }
__attribute__((weak)) bool touch_encoder_holding_user(uint8_t index, uint8_t section) { return true; }
__attribute__((weak)) bool touch_encoder_released_user(uint8_t index, uint8_t section) { return true; }


void touch_encoder_end_static_touch(uint8_t touch_handedness, half_touch_status_t *half_touch_state, half_touch_flags_t *half_touch_flags, uint8_t sectn) {
	uint8_t mask = (1 << sectn);
	// If it was being held...
	if (half_touch_state->holds & mask) { // Extracts current hold status for each section in turn. Any form of swiping and/or button timeout will toggle this off and handle key release
		xprintf("released %d %d\n", touch_handedness, sectn);
		half_touch_flags->needs_releasing ^= mask; // set flag to release whatever key was being pressed
		half_touch_state->holds &= ~mask; // Toggle bit off
		half_touch_state->was_touching &= ~mask; // And this one (will be unnecessary in the case of multi-touching,
		// but might as well have it and not need it...)
		return; // and bail (to next loop)
	} // else it may have been being tapped (not yet hit the threshold for 'hold').
	// That'd've been recorded in, for this section...
	if (half_touch_state->was_touching & mask) { // Extracts current hold status for each section in turn. Any form of swiping and/or button timeout will toggle this off and handle key release
		xprintf("tapped %d %d\n", touch_handedness, sectn);
		half_touch_flags->needs_tapping ^= mask; // Flip the bit (set flag to tap whatever key was being touched)
		half_touch_state->was_touching &= ~mask; // Toggle bit off to record this section as completed
		return; // and bail (to next loop)
	}
}

void touch_encoder_timeout_increment(uint8_t touch_handedness, half_touch_status_t *half_touch_state, half_touch_flags_t *half_touch_flags) {
	if (half_touch_flags->posn != touch_raw[3]) { // i.e. touching position has moved
		// Set the flag to send the new raw position to the handler function in keymap.c
		half_touch_flags->posn = touch_raw[3]; // to update the position for the special function (not compatible
		// with multi-touching, and it doesn't care whether you're tap/hold/swiping, so just do it every time it
		// changes.
	}
	for (uint8_t section = 0; section < TOUCH_SEGMENTS; section++) { // Iterates over all sections (1-3) on slave half
		if (half_touch_state->cycles_since_section_last_touched[section] <= TOUCH_UPDATE_SECTION_TIMEOUT) {
			if (half_touch_state->cycles_since_section_last_touched[section] == TOUCH_UPDATE_SECTION_TIMEOUT) {// Then it's time to time this one out
				// But it could either be timing out because it's an old tap/hold, or because the swipe has moved on.
				// Check if any previous logic has identified a swipe, in which case it would ALSO have handled key release,
				// (and set all was_touchings to 0)
				// so there's nothing to do
				if (!half_touch_state->swipes) {
					// Then a key has managed to go a full timeout period without a touch being recorded,
					// and without a swipe being recognised on this bar. So...
					touch_encoder_end_static_touch(touch_handedness, &half_touch_state, section);
					// and update the "initial touch position" for swiping logic (it makes no sense to keep
					// it on an expired section - allows users to switch from e.g. dual-hold -> single hold
					// -> swipe without leaving the touchbar.
					// Note that this logic will lead to a delay of SECTION_TIMEOUT between a user releasing
					// one section tap/hold and being able to initiate a swipe from a new key. This will be
					// asymmetric between the first and second keys touched.
					half_touch_state->position = touch_raw[3];
				} // else do nothing (re: timeout handling) until next loop.
			}
			// Increment by one if not already max (i.e. not touched within timeout number of cycles)
			half_touch_state->cycles_since_section_last_touched[section] += 1;
		}
	}
}

void touch_encoder_start_end_touch(uint8_t touch_handedness, half_touch_status_t *half_touch_state, half_touch_flags_t *half_touch_flags) {
	if (touch_raw[0] != touch_processed[0]) { // Then the touchbar has had a change of state (of being touched or not, and at what position)
		uint8_t delta = touch_raw[0] ^ touch_processed[0]; // Exclusive or, bitwise: delta is 1 at each position which is different between _raw and _processed
		touch_processed[0]  = touch_raw[0];
		// When calibrating, normal sensor behavior is supended
		if (delta & CALIBRATION_BIT) {
			xprintf("calibration %d\n", touch_processed[0] >> 7 & 1);
		}
		if (delta & OVERFLOW_BIT) {
			xprintf("overflow %d\n", touch_processed[0] >> 6 & 1);
		}
		if ((!touch_processed[0] & SLIDER_BIT) && (delta & SLIDER_BIT)) { // Only returns 1 ("true") if touch_processed[0] is zero at the SLIDER_BIT, i.e. bar is not currently being touched,
			// AND this is different from last time this was checked, i.e. touch status (i.e. touching or not) has changed  - just saves running this code unnecessarily
			// while the bar isn't being touched for ages.
			// so just check if anything needs releasing.

			// Set flag to release the special key
			half_touch_flags->special_release ^= (1 << 0); // Flip the bit


			// In the simplest possible case, the user was just sliding on this touchbar, so we KNOW nothing else was happening on this touchbar (since swiping overrules all other actions for the duration)
			if (half_touch_state->swipes) {// Then user was swiping at last check, but obviously isn't now (since the bar isn't being touched), so update those flages
				half_touch_state->was_swiping |= (1 << 0); // Toggle bit on (will be toggled off once...er... maybe I should toggle this off now??
				// Depends if it's important for and/or toggled in any later logic)
				half_touch_state->swipes &= ~(1 << 0); // Toggle bit off
				// There's nothing else to do in this case, since swipes are movement-based so don't need "releasing"
				// Just flag all touchbar sections as not being touch-tracking atm (i.e. "All signals are fully resolved")
				//half_touch_state.was_touching = 0; // Sets all bits in this to zero (should already have been done when the swipe was first
				// recognised actually, so don't bother)
				return true;
			}
			// else...
			// if it wasn't being swiped it was obviously being tapped or held. I'd like to leave it up to QMK's tap-hold functionality to decide,
			// but enabling multi-position touching necessitates a timeout period, which would lead to unreliable tap-hold behaviour
			// (while multiple touchbar positions are being held and/or tapped, the shortest possible 'tap' will be equal to
			// the button timeout duration; but in single button mode it will be near-instantaneous (touch_encoder_tapped_kb-determined),
			// at least as seen by QMK) - so instead I'll have to make this logic work as follows:
			// All touches are, by default, upon first being recognised, a 'tap' (recorded as an on-bit for the appropriate section in
			// half_touch_state.was_touching), which will lead to a _tapped event if the touchbar is released completely OR if that
			// section hits a timeout (multi-touch/-hold) without the swipe/"hold" timing threshold (TOUCH_TERM) being hit.
			// However, if TOUCH_TERM is hit, this might not be a hold by the user's definition, but it WILL result in a keydown
			// event if the user has not started swiping in that interval. Users should define a per-key tap-hold duration if they
			// want to use e.g. tap-dance on these keys, where a hold is registered by QMK exactly one TOUCH_TERM sooner than the
			// user actually wants, since the keydown event will be sent this duration later than the touch actually begins.
			// And if it's not being touched at all, then release everything being tapped/held.
			if (!touch_disabled) {
				for (uint8_t section = 0; section < TOUCH_SEGMENTS; section++) { // Iterates over all sections (1-3) on slave half
					touch_encoder_end_static_touch(touch_handedness, &half_touch_state, section);
				}
//				if (half_touch_state.was_touching != 0) { // All bits should have been toggled off if this point is reached
//					xprintf("Some touches weren't released correctly :(");
//				}
			}
			return true; // And those are the only possibilities if the bar has just stopped being touched, so bail
		}
		// else...
		// It could be a new touch, in which case the logic is simple:
		if (delta & SLIDER_BIT) { // I.e. the touch is new (it's changed, and it's not ended (above logic --> return)

			// Set flags to keydown the special key and send position to handler function in keymap.c
			half_touch_flags->special_press ^= (1 << 0); // Flip the bit

			// Calculate what section is being touched
			uint8_t sect = touch_raw[3] / (UINT8_MAX / TOUCH_SEGMENTS + 1); // I'm not completely sure that touch_raw will always be the
			// correct choice here, considering the slave-master transport? This could do weird stuff, might need to refactor at all occurrences.
			half_touch_state->position = touch_raw[3]; // Update the bar's "initial touch" position
			half_touch_state->was_touching |= (1 << sect); // Toggles this section's was_touching bit on
			half_touch_state->cycles_since_section_last_touched[sect] = 0; // And acknowledge the touch for timeout purposes
			half_touch_state->time_section_first_touched[sect] = timer_read(); // And note the time
			half_touch_state->position_section_first_touched[sect] = touch_raw[3]; // and the position
			return true; // No keypresses to send yet (will be sent upon touchbar release (tap), timeout (multi-touch), or transition to swiping).
		}
	}
	return false;
}


static void touch_encoder_update_position(uint8_t* position, uint8_t raw, uint8_t index,
		half_touch_flags_t *half_touch_flags) {
	int8_t delta = (*position - raw) / TOUCH_RESOLUTION;
//	bool clockwise = raw > *position; // Not necessary until later
	if (delta == 0) return; // Then user has not moved more than the touch resolution, so there's nothing to do

	// Don't store raw directly, as we want to ensure any remainder is kept and used next time this is called
	*position -= delta * TOUCH_RESOLUTION;
	xprintf("pos %d %d\n", index, raw);
	//uint8_t u_delta   = delta < 0 ? -delta : delta;
	// Update flags for send_strokes
	half_touch_flags->needs_swiping ^= (1 << 0); // flip the bit
	half_touch_flags->delt = delta; // And flag how many delts to send
}


void touch_encoder_ongoing_touch(uint8_t touch_handedness, half_touch_status_t *half_touch_state,
		half_touch_flags_t *half_touch_flags) {
	if (touch_processed[0] & SLIDER_BIT) {
		// Then there is some ongoing touch.
		// If the position has changed at all...
		// Then deal with the main touch encoder stuff
		//Check if it's previously been recognised as a swipe.
		if (half_touch_state->swipes) {// Then user was swiping at last check, so must still be swiping now.
			// Just check if a new swipe signal needs to be sent and update position if so
			if ((uint8_t)(touch_raw[3] - half_touch_state.position) <= TOUCH_DEADZONE) return; // No need to do anything if true
			// C/C++ auto-replace '.' with '->' (Window/Preferences/C/C++/Editor/Content Assist)
			// position is only updated in three situations - first touch, contiuing swipe (immediately after a swipe signal is sent),
			// or to remaining touched position once a tap/hold is released
			// else...
			// movement was more than the touch deadzone, so a swipe must be sent and the position updated
			if (!touch_disabled) {
				touch_encoder_update_position(&half_touch_state->position, touch_raw[3], touch_handedness); // Still need to sort this function out
				return; // Then get out
			}
		}
		// else...
		// check if any new taps, holds or swipes need to be started. Need to do this in a multi-touch-compatible way.
		// First, check if the conditions have been met to initiate a new swipe motion...


		// THIS IS A KEY DECISION POINT!!!!!!!!!...
		// No matter what one chooses, moving more than the deadzone within a single section should result
		// in swipe initiation, unambiguously.
		// However, if a user crosses a section boundary, the script has to make a decision whether this is
		// a swipe or two holds (since the touchbar, to my understanding, only signals the most recent touch
		// it has recognised, and is incapable of signalling two (or three or more) touches at a time - otherwise
		// this problem could be 100 % solved). The point of compromise is decided entirely by the
		// TOUCH_UPDATE_SECTION_TIMEOUT:
		// If this is set to zero, multi-touch is completely disabled and swipes will pay no attention to
		// inter-sectional boundaries, maximum responsiveness;
		// If it's set to 1, then unless the touchbar is perfectly calibrated and sends alternating signals
		// of the two (no more!) touch positions, occasionally double holds will be mistakenly interpreted as
		// swipes; and so on for higher numbers, with decreasing responsiveness for inter-sectional swiping
		// (longer timeout
		// required before a tap/hold is 'abandoned' in favour of a swipe) but increasing reliability for
		// holds. It's also possible that the touchbar will ALWAYS pick up the same signal (e.g. the closest to
		// one end of the resistor/capacitor/however this works) when given a choice of two, so multi-touch
		// holding would be impossible and there would be no benefit at all to enabling a non-zero timeout
		// cycle number.

		// Calculate what section is being touched
		uint8_t sect = touch_raw[3] / (UINT8_MAX / TOUCH_SEGMENTS + 1);
		if (!half_touch_state->was_touching) {
			// Then this is a new touch for this section (since at least one timeout period ago)
			half_touch_state->time_section_first_touched[sect] = timer_read(); // so note the time of this first touch
			half_touch_state->position_section_first_touched[sect] = touch_raw[3]; // And note the position for slighty
			// improved swipe recognition when two touches are recognised within one section before SECTION_TIMEOUT
			// has expired.
			half_touch_state->was_touching |= (1 << sect); // Toggles this section's was_touching bit on
		}
		half_touch_state->cycles_since_section_last_touched[sect] = 0; // And acknowledge the touch for timeout purposes
		// Check to see if there has been more movement within one section than the DEADZONE,
		// in which case you can just immediately initiate a swipe
		if ((half_touch_status->cycles_since_section_last_touched[sect] < TOUCH_UPDATE_SECTION_TIMEOUT) &&
				half_touch_state->position_section_first_touched[sect] - touch_raw[3] ) {
			// if the section currently being touched has been touched within the latest TIMEOUT period
			// AND is now being touched too far from where it was first touched, we'll declare this a swipe
			// immediately
			half_touch_state->position = half_touch_state->position_section_first_touched[sect];
			// Update the position for all future touchbar logic (until touching ends) to be this
			// original section touchpoint
			touch_encoder_update_position(&half_touch_state->position, touch_raw[3], touch_handedness);
			half_touch_state->swipes |= (1 << 0); // toggle swiping bit on
			return;
		}

		// else..
		uint8_t not_timed_out = 0;
		for (uint8_t section = 0; section < TOUCH_SEGMENTS; section++) { // Iterates over all sections (1-3) on slave half
			if (half_touch_status->cycles_since_section_last_touched[section] < TOUCH_UPDATE_SECTION_TIMEOUT) {
				not_timed_out += 1; // Add up all the non-timed out sections (This will be one if
				// TOUCH_UPDATE_SECTION_TIMEOUT is 1, regardless of what's pressed (since the current
				// touch being processed will always have just been set to zero (< 1, the
				// minimum TOUCH_UPDATE_SECTION_TIMEOUT value); or more than
				// one if timeout is more than one AND two or more buttons have been pressed in quick
				// succession/are being pressed simultaneously).
			}
		}
		if (not_timed_out = 1) { // Then all recent (i.e. within one timeout period previously)
			// movement has been within one section (or SECTION_TIMEOUT is disabled)
			// Note that this creates a critical dependency of the reliability of initiating swipes on
			// the size of a section (via TOUCH_DEADZONE, e.g. = 256/3  =~ 85 for three sections).
			// Then just check for movement within one section which is larger than the deadzone
			if ((uint8_t)(touch_raw[3] - half_touch_state->position) <= TOUCH_DEADZONE) return; // No need to do anything if true
			// else...
			// movement was more than the touch deadzone WITHIN one section (because all other sections
			// have timed out, so a swipe must be sent
			// and the position updated
			if (!touch_disabled) {
				touch_encoder_update_position(&half_touch_state->position, touch_raw[3], touch_handedness); // Still need to sort this function out
				// Note that this will result in larger swipes being sent when crossing section boundaries c.f. using
				// &half_touch_state.position_section_first_touched[sect] here.
				half_touch_state->swipes |= (1 << 0); // toggle swiping bit on
			}
			return;
		}
		// else...
		// Then movement crossed an inter-section boundary (OR there hasn't been enought movement yet to
		// initiaee a swipe, so do nothing), so some inference/user discretion is required
		// There's more than one section been touched within the most recent timeout period, so the program
		// HAS TO ASSUME that this is not yet a swipe. The time that this section started to be touched
		// will have been recorded above, and the timeouts will increment as normal until all but one
		// section has expired (assuming the user doesn't do something wild to prevent that ever happening),
		// at which point those keys will be released by the timeout function and the swiping maths above
		// will begin. So there's nothing else to do I think!
	}
}


void touch_encoder_check_states(uint8_t touch_handedness, half_touch_status_t *half_touch_state, half_touch_flags_t *half_touch_flags) {
	// I can achieve my desired mousekey/raw encoder value behaviour by adding an extra, 'special' key in the matrix
	// which gets keyed-down literally any time the encoder is touched, and keyed up upon release. This code
	// should also send the raw position to a handler function in keymap.c every time it changes.
	touch_encoder_timeout_increment(touch_handedness, &half_touch_state, &half_touch_flags);
    if (touch_encoder_start_end_touch(touch_handedness, &half_touch_state, &half_touch_flags)) {
    	return;
    }
    // Else the touch is not new, nor newly ended. Could be ongoing, or no touches at all.
    touch_encoder_ongoing_touch(touch_handedness, &half_touch_state, &half_touch_flags);
}


// Need to do/finish needs_press and needs_swiping!
// Handles actually sending the keystrokes after all/most of the variables/flags have been updated (using the logic in check_states().
void touch_encoder_send_strokes(uint8_t touch_handedness, half_touch_flags_t *half_touch_prev_flags, half_touch_flags_t *half_touch_flags){
	if (!touch_disabled) {// Don't do any of this if the touchbar is disabled
		if (half_touch_prev_flags->needs_tapping != half_touch_flags->needs_tapping) {// Then at least one section needs releasing
			for (uint8_t section = 0; section < TOUCH_SEGMENTS; section++) { // Iterates over all sections (1-3) on slave half
				uint8_t mask = (1 << section);
				if ((half_touch_prev_flags->needs_tapping & mask) != (half_touch_flags->needs_tapping & mask)) { // If this bit has been flipped since last check
					touch_encoder_tapped_kb(touch_handedness, section); // tap whatever key was being touched
					half_touch_prev_flags->needs_tapping &= mask; // And update the record (flip the bit in prev_)
				}
			}
		}
		if (half_touch_prev_flags->needs_press != half_touch_flags->needs_releasing) {// Then at least one section needs releasing
			for (uint8_t section = 0; section < TOUCH_SEGMENTS; section++) { // Iterates over all sections (1-3) on slave half
				uint8_t mask = (1 << section);
				if ((half_touch_prev_flags->needs_releasing & mask) != (half_touch_flags->needs_releasing & mask)) { // If this bit has been flipped since last check
					touch_encoder_released_kb(touch_handedness, section); // release whatever key was being pressed
					half_touch_prev_flags->needs_releasing &= mask; // And update the record (flip the bit in prev_)
				}
			}
		}
		if (half_touch_prev_flags->needs_releasing != half_touch_flags->needs_releasing) {// Then at least one section needs releasing
			for (uint8_t section = 0; section < TOUCH_SEGMENTS; section++) { // Iterates over all sections (1-3) on slave half
				uint8_t mask = (1 << section);
				if ((half_touch_prev_flags->needs_releasing & mask) != (half_touch_flags->needs_releasing & mask)) { // If this bit has been flipped since last check
					touch_encoder_released_kb(touch_handedness, section); // release whatever key was being pressed
					half_touch_prev_flags->needs_releasing &= mask; // And update the record (flip the bit in prev_)
				}
			}
		}
		if (half_touch_prev_flags->needs_swiping != half_touch_flags->needs_swiping) {
			bool clockwise = (half_touch_flags->delt > 0); // This logic could be the wrong way round, but meh
			touch_encoder_update_kb(touch_handedness, clockwise, abs(half_touch_flags->delt)); // hmm. This code originally ignored how many deltas
			// should be sent. I'll include them in my version. Sends the absolute (positive) magnitude of delta.
			half_touch_prev_flags->needs_swiping ^= (1 << 0); // Flip the bit
		}
		if (half_touch_prev_flags->special_press != half_touch_flags->special_press) {
			// Keydown the special key
			touch_encoder_holding_kb(touch_handedness, -1);
			half_touch_prev_flags->special_press ^= (1 << 0); // Flip the bit
		}
		if (half_touch_prev_flags->special_release != half_touch_flags->special_release) {
			touch_encoder_released_kb(touch_handedness, -1);
			half_touch_prev_flags->special_release ^= (1 << 0); // Flip the bit
		}
		if (half_touch_prev_flags->posn != half_touch_flags->posn) {
			// Send position to handler function in keymap.c
			touch_encoder_raw_position(touch_handedness, half_touch_flags->posn);
			half_touch_prev_flags->posn = half_touch_flags->posn; // And update the position
		}
}



void touch_encoder_update(int8_t transaction_id) {//This used to take void argument - changed for the transport, which is now 'less hacky'.
    if (!touch_initialized) return;

#if TOUCH_UPDATE_INTERVAL > 0
    if (!timer_expired(timer_read(), touch_update_timer)) return;
    touch_update_timer = timer_read() + TOUCH_UPDATE_INTERVAL;
#endif

    read_register(QT_DETECTION_STATUS, &touch_raw[0], sizeof(touch_raw));
    touch_processed[1] = touch_raw[1];
    touch_processed[2] = touch_raw[2];

    if (!structs_initialised) { // then this is the first run, so initialise/construct them
    	for (uint8_t i = 0; i < TOUCH_SEGMENTS; i++) {
    		touch_half_state_j->cycles_since_section_last_touched[i] = TOUCH_UPDATE_SECTION_TIMEOUT + 1;
    	}
    }
    // There are two points in the always-executed (no matter if master or slave) code which I can see that update values relevant
    // to the update_slave function - touch_slave_state.taps and .position.
    // I want to unpack that logic into the 'else' below, and make a perfectly analogous bit of code in the if (is_keyboard_master),
    // with the hope that I can ultimately merge both into one function, called twice.
    if (is_keyboard_master()) { // If master...
    	touch_encoder_check_states(touch_handness, &touch_half_state_j, &touch_half_flags_j); // Not 100 % sure this should be a pointer tbh :/
    	touch_encoder_send_strokes(touch_handness, &touch_master_prev_flags_j, &touch_half_flags_j);
		half_touch_flags_t slave_flags;
		if (transaction_rpc_exec(transaction_id, sizeof(bool), &touch_disabled, sizeof(half_touch_flags_t), &slave_flags)) {
		// I THINK this updates slave_state with the position and taps from the slave half
			if (memcmp(&touch_slave_prev_flags_j, &slave_flags, sizeof(half_touch_flags_t))) {
			// I THINK this compares the current slave_state to the last time touch_slave_state was updated,
				if (!touch_slave_init) { // Not really sure if/why this is necessary. Shouldn't both
					// initially be the same anyway? Will the first touch always be missed if this code
					// is included?
					touch_slave_prev_flags_j = slave_flags;
					touch_slave_init = true;
					return;
				}
				touch_encoder_send_strokes(!touch_handness, &touch_slave_prev_flags_j, &slave_flags);
			}
		}
    }
    else { // Then the touch encoder being updated is on the slave side, so do the logic on that ALONE
    	// (only the master half can send keystrokes to the PC!)
    	touch_encoder_check_states(!touch_handness, &touch_half_state_j, &touch_half_flags_j);
    	// All flags will be set here on the slave's side by the encoder, sync'd with the master at touch_encoder_slave_sync,
    	// then transported across by transaction_rpc_exec next time the master is polled
    }




}

void touch_encoder_calibrate(void) {
    if (!touch_initialized) return;
    write_register8(QT_CALIBRATE, 0x01);
}

bool touch_encoder_calibrating(void) {
    return touch_raw[0] & CALIBRATION_BIT;
}

void touch_encoder_toggle(void) {
    touch_disabled = !touch_disabled;
}

bool touch_encoder_toggled(void) {
    return touch_disabled;
}

// I don't know where this function is called from, or whether it accurately copies the slave side's flags into
// &touch_slave_prev_flags_j.
void touch_encoder_slave_sync(uint8_t initiator2target_buffer_size, const void* initiator2target_buffer, uint8_t target2initiator_buffer_size, void* target2initiator_buffer) {
    touch_disabled = *(bool*)initiator2target_buffer;
    memcpy(target2initiator_buffer, &touch_slave_prev_flags_j, sizeof(half_touch_flags_t)); // I think
    // this is probably run by slave, and copies the touch_slave_prev_flags_j to the initiator
    // buffer for reading during master's nezt transaction_rpc_exec run (which copies this data
    // to the target (slave?) to initiator (master?) buffer. I have a feeling that, if that's
    // true, then the "prev" flags are the wrong ones to copy! Expect to newer register a change
    // of state on targer touchbar if this is the case!
}
