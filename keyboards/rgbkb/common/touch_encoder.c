/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <https://github.com/XScorpion2> wrote this file.  As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day, and
 * you think this stuff is worth it, you can buy me a beer in return. Ryan Caltabiano
 * ----------------------------------------------------------------------------
 */



// Current issues - Need to get keymap.c to be aware of raw position (possibly using calls to
// touch_encoder_raw_position at appropriate places? Not 100 % sure if touch_encoder.c is
// "aware" that this function exists; may need to use a different approach); and handle the
// slave_state passthrough/logic of holding (and release!); not sure exactly how to transfer
// the logic from 'taps' and the 'mask' thereof :/


// I wonder if I can refactor the code so that the bytes are updated using the usual logic,
// but all the actual execution of the master half is done in parallel with the slave half later?


// May need to refactor the code to not use bits in certain places, to enforce true/false?

// And code could almost certainly be refactored to take some stuff into functions (potentially ones which return true or false
// depending on whether the calling function should also be escaped with 'return'?

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

#ifndef TOUCH_UPDATE_INTERVAL
#   define TOUCH_UPDATE_INTERVAL 33
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


// JDR tweaks (touchbar)
//bool     is_swiping[2] = {0};  // tracks whether the user is currently swiping - 0 is a byte (00000000), and will be used with masks
//bool     is_holding[6] = {0};  // tracks whether the user is currently holding - 0 is a byte (00000000), and will be used with masks
// Now handled better by the slave_touch_status_t below (maybe?? Need to think)
// JDR tweaks (touchbar) end


uint8_t  touch_handness = 0;
// touch_raw & touch_processed store the Detection Status, Key Status (x2), and Slider Position values
uint8_t  touch_raw[4]       = { 0 };
uint8_t  touch_processed[4] = { 0 };

uint16_t touch_timer        = 0;
uint16_t touch_update_timer = 0;

// For split transport only
typedef struct {
    uint8_t position;
    uint8_t taps;
} slave_touch_status_t;


// JDR tweak
typedef struct {
	uint8_t position;
	uint8_t taps; // Duplicating these attributes here to facilitate code refactoring later
	uint8_t swipes;
	uint8_t holds;
} half_touch_status_t;

bool touch_slave_init = false;
slave_touch_status_t touch_slave_state = { 0, 0 }
half_touch_status_t touch_slave_state_j = { 0, 0, 0, 0 }; // JDR tweak
half_touch_status_t touch_master_state_j = { 0, 0, 0, 0 }; // JDR tweak

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


__attribute__((weak)) bool touch_encoder_tapped_kb(uint8_t index, uint8_t section) { return touch_encoder_tapped_user(index, section); }
__attribute__((weak)) bool touch_encoder_update_kb(uint8_t index, bool clockwise) { return touch_encoder_update_user(index, clockwise); }
__attribute__((weak)) bool touch_encoder_tapped_user(uint8_t index, uint8_t section) { return true; }
__attribute__((weak)) bool touch_encoder_update_user(uint8_t index, bool clockwise) { return true; }


// JDR tweaks (touchbar)
__attribute__((weak)) bool touch_encoder_holding_kb(uint8_t index, uint8_t section) {return touch_encoder_holding_user(index, section); }
__attribute__((weak)) bool touch_encoder_released_kb(uint8_t index, uint8_t section) {return touch_encoder_released_user(index, section); }
__attribute__((weak)) bool touch_encoder_holding_user(uint8_t index, uint8_t section) { return true; }
__attribute__((weak)) bool touch_encoder_released_user(uint8_t index, uint8_t section) { return true; }


// These two functions are nowhere...
static void touch_encoder_update_holding(void) {
	uint8_t section = touch_processed[3] / (UINT8_MAX / TOUCH_SEGMENTS + 1); // TOUCH_SEGMENTS per half (e.g. 3) - defined in touch_encoder.h
	xprintf("holding %d %d\n", touch_handness, section);
	if (is_keyboard_master()) {
		if (!touch_disabled) {
			touch_encoder_holding_kb(touch_handness, section);
			touch_encoder_raw_position(*position);
		}
	}
	else {
		touch_slave_state.holds ^= (1 << section);// Just passes the problem onto slave_state function(s)
	}
}

static void touch_encoder_update_release(void) {
	//uint8_t section = touch_processed[3] / (UINT8_MAX / TOUCH_SEGMENTS + 1); // Not needed anymore, as it's always calculated before this function is called
	if (is_keyboard_master()) {
		xprintf("released %d %d\n", touch_handness, section);
		if (!touch_disabled) {
			touch_encoder_released_kb(touch_handness, section);
			// Pretty sure there's no need to update the position, as the key being released should stop e.g. mouse cursor movement by itself.
			touch_master_state.holds ^= (1 << section); // Flip this section's bit on master_state. (to 0)
		}
	}
	else {
		xprintf("released %d %d\n", !touch_handness, section);
		touch_slave_state.holds ^= (1 << section); // (flip, to 0)
	}
	// This function is only ever called when holding has ended
}
// ...near updated for the new (struct) way of doing things.


// Personal tweaks (touchbar) end



static void touch_encoder_update_tapped(void) { // Called whenever the touch bar is touched after zero touching, or completely released.
	// Note that doing it this way makes it completely impossible for two touches or holds to occur simultaneously on one touchbar!
    // Started touching, being counter for TOUCH_TERM
    if (touch_processed[0] & SLIDER_BIT) { // Only returns 1 ("true") if both are 1, i.e. bar is currently being touched
    	touch_timer = timer_read() + TOUCH_TERM; // maybe touch-timer also needs to be per-section enum/struct thing?
        return; // Below code can only be reached if this function is called when touch ENDS
    }

    uint8_t section = touch_processed[3] / (UINT8_MAX / TOUCH_SEGMENTS + 1);
    // Touch held too long, bail
    if (timer_expired(timer_read(), touch_timer)) {
#ifdef TOUCHBAR_HOLD_ENABLE// This is toggled in rules.mk (I THINK!??)
		if (is_keyboard_master()) {
			if (!(touch_master_state_j.swipes & (1 << 0))) {//then the user was holding (not swiping) the master touchbar
				xprintf("released %d %d\n", touch_handness, section);
				if (!touch_disabled) {
					touch_encoder_released_kb(touch_handness, section);
					// Pretty sure there's no need to update the position, as the key being released should stop e.g. mouse cursor movement by itself.
				}
				touch_master_state_j.holds &= ~(1 << section); // Flip this section's bit on master_state. (to 0), even if touch is disabled
				return;
			}
			// Otherwise, they were SWIPING the master touchbar, but have now stopped. There's nothing to execute in this case, since the swiping code only does anything during movement.
			touch_master_state_j.swipes &= ~(1 << 0); // Flip the swiping bit (to FALSE/0) for the master touchbar
			return;
    		}
		else {
			if (!(touch_slave_state_j.swipes & (1 << 0))) {//then the user was holding (not swiping) the slave
				xprintf("released %d %d\n", !touch_handness, section); // Handle this here, or in slave_update instead?
				if (!touch_disabled) {
					//touch_encoder_released_kb(touch_handness, section); // Why wouldn't you just handle this here rather than during slave update?
					// Pretty sure there's no need to update the position, as the key being released should stop e.g. mouse cursor movement by itself.
				}
				touch_slave_state_j.holds &= ~(1 << section); // Flip this section's bit on master_state. (to 0), even if touch is disabled
				return;
			}
			// Otherwise, they were SWIPING the slave touchbar, but have now stopped. There's nothing to execute in this case, since the swiping code only does anything during movement.
			touch_slave_state_j.swipes &= ~(1 << 0); // Flip the swiping bit (to FALSE/0) for the slave touchbar
			return;
	   }
#endif
    // If this code is reached, then a short tap has ended.
    if (is_keyboard_master()) {
        xprintf("tap %d %d\n", touch_handness, section);
    	touch_master_state_j.taps ^= (1 << section); // Flip the tap bit for this section on master half's touchbar
        if (!touch_disabled) {
            touch_encoder_tapped_kb(touch_handness, section); // And send the keypress
        }
    }
    else {
        touch_slave_state.taps ^= (1 << section); // Flip the tap bit for this section on slave half's touchbar.
        // This bit's value is meaningless (this command will flip it to 0 or 1 in an alternating way),
        // except in its comparison to the previously got slave_state (if it has flipped, then a tap has occurred.
        touch_slave_state_j.taps ^= (1 << section);
    }
}

static void touch_encoder_update_position_common(uint8_t* position, uint8_t raw, uint8_t index) {
    int8_t delta = (*position - raw) / TOUCH_RESOLUTION;
    bool clockwise = raw > *position;
    if (delta == 0) return;

    // Don't store raw directly, as we want to ensure any remainder is kept and used next time this is called
    *position -= delta * TOUCH_RESOLUTION;
    xprintf("pos %d %d\n", index, raw);
    //uint8_t u_delta   = delta < 0 ? -delta : delta;
    if (!touch_disabled) {
        //for (uint8_t i = 0; i < u_delta; i++)
            touch_encoder_update_kb(index, clockwise);
            touch_encoder_raw_position(*position); // I think position is probably better than raw, but I'm not 100 % sure tbh.
    }
}

static void touch_encoder_update_position(void) {
    // If the user touchs and moves enough, expire touch_timer faster and do encoder position logic instead
    if (!timer_expired(timer_read(), touch_timer)) {
        if ((uint8_t)(touch_raw[3] - touch_processed[3]) <= TOUCH_DEADZONE) return; // bail if not held long enough AND not moved far enough
        if (is_keyboard_master()) { // If you reach this code, the user has just started swiping
        	//(IS THIS TRUE? Also possible the user is continuing to swipe, no? In which case this flipping makes no sense. But forcing true would, if that can be done?)
        	touch_master_state_j.swipes |= (1 << 0); // Flip bit from false (0) to true (1)
        }
        else {
        	touch_slave_state_j.swipes |= (1 << 0); // Flip bit from false (0) to true (1)
        }
        touch_timer = timer_read();
    }

    // If the user has been holding one position (moving less than the deadzone) for longer
           // than the TOUCH_TERM, then process as a hold (if this behaviour is enabled)
#ifdef TOUCHBAR_HOLD_ENABLE
   if (is_keyboard_master()) {
	   if (!(touch_master_state_j.swipes & (1 << 0))) {//then user did not start swiping (the master touchbar) in the first TOUCH_TERM, but they MIGHT start later
		   if ((uint8_t)(touch_raw[3] - touch_processed[3]) <= TOUCH_DEADZONE) {// then user is still in the deadzone (i.e. holding)
			   uint8_t mask = (1 << section);
			   if (!(touch_master_state_j.holds) & (1 << section))){//then this is the first pass where the holding has been noticed on this section
				   touch_master_state_j.holds |= (1 << section); // Flip holds from 0 (not holding) to 1 (holding)
				   touch_encoder_update_holding(&touch_processed[3], touch_raw[3], touch_handness); // Not happy with this line yet
				}
			return; // then bail (I THINK this is in the right place, but could compare to earlier versions to be sure.
		   }
	   // otherwise, this looks like it's turned into a swipe, so move on to the code for that (below)
	  // but first, deal with releasing the held key (do ALL that are currently held, because nothing else would make sense if you're also swiping)
	  // NEED TO DEAL WITH THIS IN A SLAVE-FRIENDLY WAY! Also be careful about ensuring touch_processed isn't updated between now and slave sync, or weird things might happen...
		for (uint8_t loop_section = 0; loop_section < TOUCH_SEGMENTS; loop_section++) { // Iterate over all sections
			uint8_t mask = (1 << loop_section);
			if (touch_master_state_j.holds & mask) { // For every bit that's currently on
				xprintf("released %d %d\n", touch_handness, section);
		   		if (!touch_disabled) {
		   			touch_encoder_released_kb(touch_handness, loop_section); // Release the key
		   			// Pretty sure there's no need to update the position, as the key being released should stop e.g. mouse cursor movement by itself.
		   		}
		   		touch_master_state_j.holds &= ~(1 << loop_section); // Flip this section's bit on master_state. (to 0)
			}
	   }
		touch_master_state_j.swipes |= (1 << 0)// Then acknowledge that swiping is now on for the master half

   }
   else {
	   if (!(touch_master_state_j.swipes & (1 << 0))) {//then user did not start swiping (the master touchbar) in the first TOUCH_TERM, but they MIGHT start later
		   if ((uint8_t)(touch_raw[3] - touch_processed[3]) <= TOUCH_DEADZONE) {// then user is still in the deadzone (i.e. holding)
			   uint8_t mask = (1 << section);
			   if (!(touch_master_state_j.holds) & (1 << section))){//then this is the first pass where the holding has been noticed on this section
				   touch_slave_state_j.holds |= (1 << section); // Needs handling in update_slave, below!
				// Not happy with this line yet
			   }
			   return; // then bail (I THINK this is in the right place, but could compare to earlier versions to be sure.
		   }
	   // otherwise, this looks like it's turned into a swipe, so move on to the code for that (below)
		  // but first, deal with releasing the held key (do ALL that are currently held, because nothing else would make sense if you're also swiping)
		  // NEED TO DEAL WITH THIS IN A SLAVE-FRIENDLY WAY! Also be careful about ensuring touch_processed isn't updated between now and slave sync, or weird things might happen...
		   for (uint8_t loop_section = 0; loop_section < TOUCH_SEGMENTS; loop_section++) { // Iterate over all sections
				uint8_t mask = (1 << loop_section);
				if (touch_slave_state_j.holds & mask) { // For every bit that's currently on
					xprintf("released %d %d\n", !touch_handness, section);
					if (!touch_disabled) {
						touch_encoder_released_kb(!touch_handness, loop_section); // Release the key - IS THIS SUPPOSED TO BE HANDLED HERE, or in slave update later??
						// Pretty sure there's no need to update the position, as the key being released should stop e.g. mouse cursor movement by itself.
					}
					touch_slave_state_j.holds &= ~(1 << loop_section); // Flip this section's bit on slave_state. (to 0)
				}
		   }
			touch_slave_state_j.swipes |= (1 << 0)// Then acknowledge that swiping is now on for the slave half
	   }
   }
#endif

    if (is_keyboard_master()) {
        touch_encoder_update_position_common(&touch_processed[3], touch_raw[3], touch_handness);
    }
    else {
        touch_slave_state.position = touch_raw[3];
    }
}

void touch_encoder_update_slave(slave_touch_status_t slave_state) {
    if (!touch_slave_init) {
        touch_slave_state = slave_state;
        touch_slave_init = true;
        return;
    }

    if (touch_slave_state.position != slave_state.position) {
        // Did a new slide event start?
        uint8_t mask = (1 << 7);
        if ((touch_slave_state.taps & mask) != (slave_state.taps & mask)) {
            touch_slave_state.position = slave_state.position;
        }
        touch_encoder_update_position_common(&touch_slave_state.position, slave_state.position, !touch_handness);
    }

    if (touch_slave_state.taps != slave_state.taps) { // Then some new touch (or release) has occurred since last check
    	// (7th bit flip it touch_encoder_update)
        if (!touch_disabled) {
            for (uint8_t section = 0; section < TOUCH_SEGMENTS; section++) { // Iterates over all sections (1-3) on slave half
                uint8_t mask = (1 << section);
                if ((touch_slave_state.taps & mask) != (slave_state.taps & mask)) { // Extracts number of taps for each section in turn, and compares them to previous time checked.
                // If was sliding or holding, NONE of these will have changed, as bits 1-6 are only changed after tap is released (in touch_encoder_update_tapped).
                    xprintf("tap %d %d\n", !touch_handness, section);
                    touch_encoder_tapped_kb(!touch_handness, section);
                }
                if ((touch_slave_state.holds & mask) != (slave_state.holds & mask)) { // Extracts current hold state (true/false)for each section in turn, and compares them to previous time checked.
					// If was sliding or holding, NONE of these will have changed, as bits 1-6 are only changed after tap is released (in touch_encoder_update_tapped).
					xprintf("hold %d %d\n", !touch_handness, section);
					touch_encoder_tapped_kb(!touch_handness, section);
					}
            }
        }
        touch_slave_state.taps = slave_state.taps;

    } // This is really complex and I don't think it works yet.
#ifdef TOUCHBAR_HOLD_ENABLE
    if (touch_slave_state.holds != slave_state.holds) {
		if (!touch_disabled) {
			for (uint8_t section = 0; section < TOUCH_SEGMENTS; section++) {
				uint8_t mask = (1 << section);
				if ((touch_slave_state.holds & mask) != (slave_state.holds & mask)) {
					xprintf("hold %d %d\n", !touch_handness, section);
					touch_encoder_holding_kb(!touch_handness, section);
				}
			}
		}
		touch_slave_state.taps = slave_state.taps;
    }
#endif
}

void touch_encoder_update(int8_t transaction_id) {//This used to take void argument - what's the change?
    if (!touch_initialized) return;

#if TOUCH_UPDATE_INTERVAL > 0
    if (!timer_expired(timer_read(), touch_update_timer)) return;
    touch_update_timer = timer_read() + TOUCH_UPDATE_INTERVAL;
#endif

    read_register(QT_DETECTION_STATUS, &touch_raw[0], sizeof(touch_raw));
    touch_processed[1] = touch_raw[1];
    touch_processed[2] = touch_raw[2];

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
        if (delta & SLIDER_BIT) { // i.e. touch status (i.e. touching or not) has changed - Note that this specific logic check means that _tapped will never
        	// be called if one section of a touchbar is being held when another section of the same touchbar begins a tap or hold. Could do with a different
        	// check here (e.g. a per-section enum/struct thing), extracting the position and checking if that section was previously being tapped/held!!!
            touch_processed[3] = touch_raw[3]; // Extracts the position of touching
            if (is_keyboard_master()) { // If master...
                touch_master_state_j.position = touch_raw[3]; // Then tell master functions that they need to process something (and give them the position)
                touch_master_state_j.taps ^= (1 << 7); // Inverts the bit at position 7 in the byte to flag that a new touch has occurred. Doesn't actually get
                // used for master half with code factored as-is, but there to ease refactoring if that's done at any point.
                //touch_master_state.holds ^= (1 << 7); // Same for hold (necessary??)
            }
            else { // if slave...
            	touch_slave_state.position = touch_raw[3]; // Then tell slave functions that they need to process something (and give them the position)
				touch_slave_state.taps ^= (1 << 7); // Inverts the bit at position 7 in the byte to flag that a new touch has occurred
				touch_slave_state_j.position = touch_raw[3]; // And JDR struct attributes as well
				touch_slave_state_j.taps ^= (1 << 7); // Necessary?
				//touch_slave_state.holds ^= (1 << 7); // Same for hold (necessary??)
            }
            touch_encoder_update_tapped(); // Then runs the usual checks to determine if a tap or hold has just started or ended (but NOT
            // "turned into" a swipe - this is handled in update_position, below
        }
    }

    //if ((touch_raw[0] & SLIDER_BIT) && touch_processed[3] != touch_raw[3]) {
    if (touch_raw[0] & SLIDER_BIT){// I removed the extra check here so I can hijack this function for holding.
	   // Losing it doesn't seem to cause any problems or behaviour changes. Keeping it means that holding users need to wiggle very slightly before
    	// touch_encoder_update_tapped (and hence touch_encoder_holding after some logic) will be reached/called the second (necessary) time
        touch_encoder_update_position();
    }

    if (is_keyboard_master()) {
        slave_touch_status_t slave_state;
        if (transaction_rpc_exec(transaction_id, sizeof(bool), &touch_disabled, sizeof(slave_touch_status_t), &slave_state)) {
            if (memcmp(&touch_slave_state, &slave_state, sizeof(slave_touch_status_t)))
                touch_encoder_update_slave(slave_state);
        }
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

void touch_encoder_slave_sync(uint8_t initiator2target_buffer_size, const void* initiator2target_buffer, uint8_t target2initiator_buffer_size, void* target2initiator_buffer) {
    touch_disabled = *(bool*)initiator2target_buffer;
    memcpy(target2initiator_buffer, &touch_slave_state, sizeof(slave_touch_status_t));
}

// The following was how transport USED to be handled, but I'm pretty sure I didn't make any tweaks to it and this can be deleted
// (superseded by touch_encoder_slave_sync, above)
//void touch_encoder_get_raw(slave_touch_status_t* slave_state) {
//    memcpy(slave_state, &touch_slave_state, sizeof(slave_touch_status_t));
//}
//
//void touch_encoder_set_raw(slave_touch_status_t slave_state) {
//    if (!touch_slave_init) {
//        touch_slave_state = slave_state;
//        touch_slave_init = true;
//        return;
//    }
//
//    if (touch_slave_state.position != slave_state.position) {
//        // Did a new slide event start?
//        uint8_t mask = (1 << 7);
//        if ((touch_slave_state.taps & mask) != (slave_state.taps & mask)) {
//            touch_slave_state.position = slave_state.position;
//        }
//        touch_encoder_update_position_common(&touch_slave_state.position, slave_state.position, !touch_handness);
//    }
//
//    if (touch_slave_state.taps != slave_state.taps) {
//        if (!touch_disabled) {
//            for (uint8_t section = 0; section < TOUCH_SEGMENTS; section++) {
//                uint8_t mask = (1 << section);
//                if ((touch_slave_state.taps & mask) != (slave_state.taps & mask)) {
//                    xprintf("tap %d %d\n", !touch_handness, section);
//                    touch_encoder_tapped_kb(!touch_handness, section);
//                }
//            }
//        }
//        touch_slave_state.taps = slave_state.taps;
//    }
//}
