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

typedef enum {IDLE, LOWS, HIGH, TRAN} DCF77_demod_state;
typedef enum {HP, FS, LP, TR, EE} DCF77_demod_event;

typedef void DCF77_demod_activity(uint16_t duration);
typedef DCF77_demod_activity* DCF77_demod_todo;

typedef struct {
	DCF77_demod_state new_state;
	DCF77_demod_todo task;
} dcf77_demod_element;

typedef struct {
	uint8_t level;
	uint16_t duration;
	uint16_t period;
} dcf77_demod_level;

typedef struct {
	DCF77_demod_event e;
	dcf77_demod_level l;
} dcf77_demod_next_pos;

dcf77_demod_element g_demod_DCF77Control[TRAN+1][EE+1];

//TODO
void init_demod();

//TODO
dcf77_demod_next_pos dcf77_demod_next_level(dcf77_demod_level level);

////////////////////////////////////////////////////////////////////////////////
/*
 * The section below contains the code to decode the demodulated bit stream.
 */
////////////////////////////////////////////////////////////////////////////////

typedef enum {MinuteFrame, Infoend = 20, Minuteend = 28,
				Hourend = 35, Dateend = 58} dcf77_position;
typedef enum {F, I, M, H, D} DCF77_decode_state;
typedef enum {MF, DF, DB, PB, ER} DCF77_decode_event;

typedef void DCF77_decode_activity(int8_t b);
typedef DCF77_decode_activity* DCF77_decode_todo;

typedef struct {
	DCF77_decode_event e;
	uint8_t b;
} dcf77_decode_next_pos;

typedef struct {
	DCF77_decode_state new_state;
	DCF77_decode_todo task;
} dcf77_decode_element;

typedef struct {
	uint32_t buffer; 	/**< Global variable for input buffer */
	uint8_t parity;
} DCF77_decode_input_buffer_struct;

typedef struct {
	unsigned int spacialflags:20;
	unsigned int minute:6;
	unsigned int minutevalidflag:1;
	unsigned int hour:6;
	unsigned int hourvalidflag:1;
	unsigned int day:5;
	unsigned int dayvalidflag:1;
	unsigned int dayofweek:4;
	unsigned int dayofweekvalidflag:1;
	unsigned int month:5;
	unsigned int monthvalidflag:1;
	unsigned int year:7;
	unsigned int yearvalidflag:1;
	unsigned int validframe:1;
} timedate_temp;


//Global variables only witten by the state machine
timedate_temp* g_DCF77_tmp_timedate;

dcf77_decode_element g_decode_DCF77Control[D+1][ER+1];	/**< Matrix of control structure */

//64 bit variable for whole frame analysis (further feature)
uint64_t g_DCF77_decode_frame;

//Initialize state model
void init_decode(timedate_temp* timedate);

//TODO
dcf77_decode_next_pos dcf77_decode_next_bit(int8_t bit);


#endif /* DCF77_H_ */
