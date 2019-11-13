/*
 * dcf77.h
 *
 *  Created on: 2019. okt. 20.
 *  Author: Skriba Dániel
 *
 */

#ifndef DCF77_H_
#define DCF77_H_

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
/*
 * The section below contains the code to demodulate the pulse amplitude coded
 * discrete data coming from the DCF77 receiver. The program produces a bit
 * stream consists of zeros and ones.
 */
////////////////////////////////////////////////////////////////////////////////

/**
 * States of the demodulator.
 * The demodulator has two states the IDLE when it does nothing until a correct
 * minute frame isn't received. When minute frame detected it changes to receiving
 * state and stays there until an invalid second is received.
 */
typedef enum {IDLE, RECV} DCF77_demod_state;

/**
 * Possible action of the demodulator
 * MS: Minute Start
 * FS: Frame Sync
 * SP: Second pulse
 * EE: Error
 * The slightly difference between MS and FS is that MS doesn't have to has a
 * period in the intervall of [1990; 2010] only needs to have the sufficient
 * high level duration.
 */
typedef enum {MS, FS, SP, EE} DCF77_demod_event;

/**
 * Function pointer for demodulation actions.
 */
typedef void DCF77_demod_activity(int8_t duration);
typedef DCF77_demod_activity* DCF77_demod_todo;

/**
 * A structure to represent the new state of the state machine and the action to do.
 */
typedef struct {
	DCF77_demod_state new_state; /**< The new state */
	DCF77_demod_todo task;		 /**< The function to be called */
} dcf77_demod_element;

/**
 * The struct for the next event and the parameter of this event.
 */
typedef struct {
	DCF77_demod_event e;		/**< The event that happend in the iteration */
	int8_t d;					/**< The parameter beleongs to the action to do in this event */
} dcf77_demod_next_pos;

/**
 * The struct to represent a second with a high and a low level duration.
 */
typedef struct {
	uint16_t low_duration;		/**< The duration of the low level within the second in milisecond */
	uint16_t high_duration;		/**< The duretion of the high level within the second in miliseconds */
} dcf77_second;

dcf77_demod_element g_demod_DCF77Control[RECV+1][EE+1];

/**
 * Initialising the state machine for demodulations
 * @param sync_action function to call when minute frame is detected
 * @param bitgen pointer to a function to call when a new bit is demodulated
 */
void init_demod();

/**
 * This function decides what is the next action to be done and with what parameter
 * @param second The duration of low and high level within the second
 * @return A structure that contains the action to do and the parameter of this action
 */
dcf77_demod_next_pos dcf77_demod_next_level(dcf77_second second);

////////////////////////////////////////////////////////////////////////////////
/*
 * The section below contains the code to decode the demodulated bit stream.
 */
////////////////////////////////////////////////////////////////////////////////

/**
 * Key positions during decoding the DCF77 bits
 */
typedef enum {MinuteFrame, Infoend = 20, Minuteend = 28,
				Hourend = 35, Dateend = 58} dcf77_position;

/**
 * States to decode the bitstream from the demodulator
 * F:
 * I:
 * M:
 * H:
 * D:
 */
typedef enum {F, I, M, H, D} DCF77_decode_state;

/**
 * Events during the decoding procedure.
 * FD: Frame detection.
 * MF: The first bit of the minute is zero start receiving bits. Minute frame.
 * DF: End of the additional information bits within the minute with 21th bit is 1
 * DB: Data bit
 * PB: Parity bit received
 * ER: Invalid bit or error signal from the demodulator
 */
typedef enum {FD, MF, DF, DB, PB, ER} DCF77_decode_event;

/**
 * Function pointer for decoding actions.
 */
typedef void DCF77_decode_activity(int8_t b);
typedef DCF77_decode_activity* DCF77_decode_todo;

/**
 * A structure to represent the new state of the state machine and the action to do.
 */
typedef struct {
	DCF77_decode_event e; 			/**< The event that happend in the iteration */
	uint8_t b;						/**< The parameter beleongs to the action to do in this event */
} dcf77_decode_next_pos;

/**
 * The struct for the next event and the parameter of this event.
 */
typedef struct {
	DCF77_decode_state new_state;	/**< The event that happend in the iteration */
	DCF77_decode_todo task;			/**< The parameter beleongs to the action to do in this event */
} dcf77_decode_element;

/**
 * Struct for the input buffer and the parity bit
 */
typedef struct {
	uint32_t buffer; 	/**< Variables for the input bits */
	uint8_t parity;		/**< This variable indicates the current status of the parity */
} DCF77_decode_input_buffer_struct;

/**
 * Struct for the input bits and flags for their valid states.
 * During the decoding process every field of the information is stored in a
 * variable with flags that indicates that the decoding was successful and the
 * field contains valid values.
 */
typedef struct {
	unsigned int spacialflags:20;		/**< Additional information bits at the beginning of the minute */
	unsigned int minute:6;				/**< Minutes stored on 6 bits */
	unsigned int minutevalidflag:1;		/**< Flag if the minute seems valid */
	unsigned int hour:6;				/**< Hour stored in 6 bits */
	unsigned int hourvalidflag:1;		/**< Flag  if the hour seems valid*/
	unsigned int day:5;					/**< Day of the month stored on 5 bits */
	unsigned int dayvalidflag:1;		/**< Day of the month is valid */
	unsigned int dayofweek:4;			/**< Day of the week stored in 4 bits */
	unsigned int dayofweekvalidflag:1;	/**< Day of the week is valid */
	unsigned int month:5;				/**< Month is stored in five bits */
	unsigned int monthvalidflag:1;		/**< Month is valid */
	unsigned int year:7;				/**< Year of the century stored in 7 bits */
	unsigned int yearvalidflag:1;		/**< Year of the century is valid */
	unsigned int validframe:1;			/**< Flag indicates that the all input field is valid */
} timedate_temp;


timedate_temp* g_DCF77_tmp_timedate; /**< Global pointer to the struct that stores the decoded time. */

dcf77_decode_element g_decode_DCF77Control[D+1][ER+1];	/**< Matrix of control structure */

//64 bit variable for whole frame analysis (further feature)
uint64_t g_DCF77_decode_frame;

/**
 * Initialising the state machine for decoding
 * @param timedate Pointer to the struct where the program will store the decoded datetime in this timedate_temp
 */
void init_decode(timedate_temp* timedate);

/**
 * This function decides what is the next action to be done and with what parameter.
 * @param bit The next decoded bit
 * @return A structure that contains the action to do and the parameter of this action
 */
dcf77_decode_next_pos dcf77_decode_next_bit(int8_t bit);


#endif /* DCF77_H_ */
