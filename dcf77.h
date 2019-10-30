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
} next_pos;

typedef struct {
	DCF77_decode_state new_state;
	DCF77_decode_todo task;
} element;

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


//Global variables
timedate_temp* g_DCF77_tmp_timedate;

element g_DCF77Control[D+1][ER+1];	/**< Matrix of control structure */

//TODO: Creat a 64 bit buffer for whole frame for analysis

//Initialize state model
void init_decode(timedate_temp* timedate);

//TODO
next_pos next_bit(int8_t bit);


#endif /* DCF77_H_ */
