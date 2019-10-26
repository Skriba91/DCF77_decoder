/*
 * dcf77.h
 *
 *  Created on: 2019. okt. 20.
 *  Author: Skriba Dániel
 */

#ifndef DCF77_H_
#define DCF77_H_

#include <stdint.h>

typedef enum {MinuteFrame, Infoend = 20, Minuteend = 28,
				Hourend = 35, Dateend = 58} dcf77_position;
typedef enum {F, I, M, H, D, E} DCF77_decode_state;
typedef enum {MF, DF, DB, PB, ER} DCF77_decode_event;

typedef void DCF77_decode_activity(uint8_t b);
typedef DCF77_decode_activity* DCF77_decode_todo;

typedef struct {
	DCF77_decode_event e;
	uint8_t b;
} next_pos;

typedef struct {
	DCF77_decode_state new_state;
	DCF77_decode_todo task;
} element;

element g_DCF77Control[E+1][ER+1];

//Initialise state model
void init_decode();

next_pos next_bit(int8_t bit) {
	static uint8_t cntr = 0;
	next_pos p;
	p.b = bit;
	if(bit < 0) {
		p.e = ER;
	}
	else {
		switch(cntr) {
		case MinuteFrame : p.e = MF; break;
		case Infoend     : p.e = DF; break;
		case Minuteend   : p.e = PB; break;
		case Hourend     : p.e = PB; break;
		case Dateend     : p.e = PB; break;
		default          : p.e = DB;
		}
	}

	//If not end of date increment counter else reset counter to zero
	cntr != Dateend ? cntr++ : cntr = 0;

	return p;

}

//TODO
void putinbuffer(int8_t bit);

//TODO
void framecheckminute(int8_t bit);

//TODO
void framecheckdata(int8_t bit);

//TODO
void paritycheckminute(int8_t bit);

//TODO
void paritycheckhour(int8_t bit);

//TODO
void paritycheckdate(int8_t bit);

//TODO
void DCF77_decode_error(int8_t bit);


#endif /* DCF77_H_ */
