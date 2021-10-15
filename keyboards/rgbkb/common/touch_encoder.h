/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <https://github.com/XScorpion2> wrote this file.  As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day, and
 * you think this stuff is worth it, you can buy me a beer in return. Ryan Caltabiano
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifndef TOUCH_TERM
#    define TOUCH_TERM 350
#endif

#ifndef TOUCH_SEGMENTS
#    define TOUCH_SEGMENTS 3
#elif TOUCH_SEGMENTS < 1 || TOUCH_SEGMENTS > 5
#    error TOUCH_SEGMENTS must be between 1 and 5.
#endif

#ifndef TOUCH_DEADZONE
#    define TOUCH_DEADZONE 50
#endif

#ifndef TOUCH_RESOLUTION
#    define TOUCH_RESOLUTION 25
#endif

void touch_encoder_init(void);
void touch_encoder_update(int8_t transaction_id);//used to be void argument

void touch_encoder_calibrate(void);
bool touch_encoder_calibrating(void);

void touch_encoder_toggle(void);
bool touch_encoder_toggled(void);

// Called when touch encoder is tapped, weak function overridable by the kb
bool touch_encoder_tapped_kb(uint8_t index, uint8_t section); // used to return void

#ifdef TOUCHBAR_HOLD_ENABLE
// Called when touch encoder is held, weak function overridable by the kb
	void touch_encoder_holding_kb(uint8_t index, uint8_t section);
	void touch_encoder_released_kb(uint8_t index, uint8_t section);
#endif

// Called when touch encoder is slid, weak function overridable by the kb
bool touch_encoder_update_kb(uint8_t index, bool clockwise);// used to return void

// Called when touch encoder is tapped, weak function overridable by the user
bool touch_encoder_tapped_user(uint8_t index, uint8_t section);//used to return void


#ifdef TOUCHBAR_HOLD_ENABLE
	// Called when touch encoder is held, weak function overridable by the user
	void touch_encoder_holding_user(uint8_t index, uint8_t section);
	void touch_encoder_released_user(uint8_t index, uint8_t section);
#endif


// Called when touch encoder is slid, weak function overridable by the user
bool touch_encoder_update_user(uint8_t index, bool clockwise);//used to return void


//Used to be a struct and a couple of (get/set_raw) functions here for transport, now removed.

void touch_encoder_slave_sync(uint8_t initiator2target_buffer_size, const void* initiator2target_buffer, uint8_t target2initiator_buffer_size, void* target2initiator_buffer);
