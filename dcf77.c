/*
 * dcf77.c
 *
 *  Created on: 2019. okt. 20.
 *  Author: Skriba D�niel
 *
 */


#include "dcf77.h"
#include <stdio.h>

#define G_DCF77_WEIGHTMASK 1
#define G_DCF77_WEIGHTLENGTH 8
#define G_DCF77_WEEKDAYMAS 0b111
#define G_DCF77_MONTHMASK 0b11111
#define G_DCF77_YEARMASK 0b11111111

#define G_DCF77_MINUTEOFFSET 6
#define G_DCF77_HOUROFFSET 5
#define G_DCF77_DAYOFFSET 5
#define G_DCF77_DAYOFWEEKOFFSET 2
#define G_DCF77_MONTHOFFSET 4
#define G_DCF77_YEAROFFSET 7

DCF77_decode_input_buffer_struct g_DCF77_decode_buffer; 	/**< Global variable for input buffer */

uint8_t calculateDecimalValue(uint32_t tmp_buffer, int8_t offset) {

	uint8_t decimalvalue = 0;
	const uint8_t weight[8] = {1, 2, 4, 8, 10, 20, 40, 80};

	for(uint8_t i = 0; offset >= 0; ++i) {
		decimalvalue += ((tmp_buffer >> offset) &
						G_DCF77_WEIGHTMASK) * weight[i];
		--offset;
	}
	//TODO: To increase efficiency unroll loop or use vector instructions

	return decimalvalue; //Returning the decimal value
}

void resetTmpTimedateFlags() {
	g_DCF77_tmp_timedate->minutevalidflag = 0;
	g_DCF77_tmp_timedate->hourvalidflag = 0;
	g_DCF77_tmp_timedate->dayvalidflag = 0;
	g_DCF77_tmp_timedate->dayofweekvalidflag = 0;
	g_DCF77_tmp_timedate->monthvalidflag = 0;
	g_DCF77_tmp_timedate->yearvalidflag = 0;
	g_DCF77_tmp_timedate->validframe = 0;
}

void setIfAllValid() {
	//If all timedate flag are valid then set the global valid flag
	if(g_DCF77_tmp_timedate->minutevalidflag &
			g_DCF77_tmp_timedate->hourvalidflag &
			g_DCF77_tmp_timedate->dayvalidflag &
			g_DCF77_tmp_timedate->dayofweekvalidflag &
			g_DCF77_tmp_timedate->monthvalidflag &
			g_DCF77_tmp_timedate->yearvalidflag)
	{
		g_DCF77_tmp_timedate->validframe = 1;
	}
	else {
		g_DCF77_tmp_timedate->validframe = 0;
	}
}

void emptyBuffer() {
	//TODO
	g_DCF77_decode_buffer.buffer = 0;
	g_DCF77_decode_buffer.parity = 0;
}

void putinbuffer(int8_t bit) {
	g_DCF77_decode_buffer.buffer <<= 1;			//Shifting bufer left by one
	g_DCF77_decode_buffer.buffer |= (uint32_t)bit;	//Puting new bit in the LSB position
	g_DCF77_decode_buffer.parity ^= bit;
}

void minuteframestart(int8_t bit) {
	emptyBuffer();
	resetTmpTimedateFlags();	//Resetting all flags to 0
}

void timedateframestart(int8_t bit) {
	g_DCF77_tmp_timedate->spacialflags = g_DCF77_decode_buffer.buffer;
	emptyBuffer();
}

void paritycheckminuteandset(int8_t bit) {

	if(g_DCF77_decode_buffer.parity == bit) {
		uint8_t temp = 0;
		temp = calculateDecimalValue(g_DCF77_decode_buffer.buffer, G_DCF77_MINUTEOFFSET);
		if(temp < 60) {
			g_DCF77_tmp_timedate->minute = temp;
			g_DCF77_tmp_timedate->minutevalidflag = 1;
		}
		else {
			g_DCF77_tmp_timedate->minutevalidflag = 0;
		}

	}
	else {
		g_DCF77_tmp_timedate->minutevalidflag = 0;
	}
	emptyBuffer();
}

void paritycheckhourandset(int8_t bit) {

	if(g_DCF77_decode_buffer.parity == bit) {
		uint8_t temp = calculateDecimalValue(g_DCF77_decode_buffer.buffer, G_DCF77_HOUROFFSET);
		if(temp < 60) {
			g_DCF77_tmp_timedate->hour = temp;
			g_DCF77_tmp_timedate->hourvalidflag = 1;
		}
		else {
			g_DCF77_tmp_timedate->hourvalidflag = 0;
		}
	}
	else {
		g_DCF77_tmp_timedate->hourvalidflag = 0;
	}
	emptyBuffer();
}

void paritycheckdateandfinish(int8_t bit) {

	if(g_DCF77_decode_buffer.parity == bit) {
		uint8_t day = calculateDecimalValue(g_DCF77_decode_buffer.buffer >> 16, G_DCF77_DAYOFFSET);
		uint8_t weekday = calculateDecimalValue(
				(g_DCF77_decode_buffer.buffer >> 13) & G_DCF77_WEEKDAYMAS, G_DCF77_DAYOFWEEKOFFSET);
		uint8_t month = calculateDecimalValue(
				(g_DCF77_decode_buffer.buffer >> 8) & G_DCF77_MONTHMASK, G_DCF77_MONTHOFFSET);
		uint8_t year = calculateDecimalValue(
				g_DCF77_decode_buffer.buffer & G_DCF77_YEARMASK, G_DCF77_YEAROFFSET);
		if(day < 32 && day != 0) {
			g_DCF77_tmp_timedate->day = day;
			g_DCF77_tmp_timedate->dayvalidflag = 1;
		}
		else {
			g_DCF77_tmp_timedate->dayvalidflag = 0;
		}
		if(weekday < 8 && day != 0) {
			g_DCF77_tmp_timedate->dayofweek = weekday;
			g_DCF77_tmp_timedate->dayofweekvalidflag = 1;
		}
		else {
			g_DCF77_tmp_timedate->dayofweekvalidflag = 0;
		}
		if(month < 13 && month != 0) {
			g_DCF77_tmp_timedate->month = month;
			g_DCF77_tmp_timedate->monthvalidflag = 1;
		}
		else {
			g_DCF77_tmp_timedate->monthvalidflag = 0;
		}
		if(year < 100) {
			g_DCF77_tmp_timedate->year = year;
			g_DCF77_tmp_timedate->yearvalidflag = 1;
		}
		else {
			g_DCF77_tmp_timedate->yearvalidflag = 0;
		}
	}
	else {
		g_DCF77_tmp_timedate->dayvalidflag = 0;
		g_DCF77_tmp_timedate->dayofweekvalidflag = 0;
		g_DCF77_tmp_timedate->monthvalidflag = 0;
		g_DCF77_tmp_timedate->yearvalidflag = 0;
	}
	setIfAllValid();
	emptyBuffer();
}

void DCF77_decode_error(int8_t bit) {
	resetTmpTimedateFlags();
	emptyBuffer();
	printf("Error: %c", bit);
}

next_pos next_bit(int8_t bit) {
	static uint8_t cntr = 0;
	next_pos p;
	p.b = bit;
	if(bit < 0) {
		p.e = ER;	//Creating an error event
		cntr = 0;	//Starting over from error
	}
	else {
		switch(cntr) {
		case MinuteFrame : (bit == 0) ? (p.e = MF) : (p.e = ER); break;
		case Infoend     : (bit == 1) ? (p.e = DF) : (p.e = ER); break;
		case Minuteend   : p.e = PB; break;
		case Hourend     : p.e = PB; break;
		case Dateend     : p.e = PB; break;
		default          : p.e = DB;
		}
	}

	//If not end of date increment counter else reset counter to zero
	(cntr != Dateend) ? cntr++ : (cntr = 0);

	return p;

}

//Initialize state model
void init_decode(timedate_temp* timedate) {
	DCF77_decode_state i;
	DCF77_decode_event j;

	//TODO
	g_DCF77_decode_buffer.buffer = 0;
	g_DCF77_decode_buffer.parity = 0;

	//TODO
	g_DCF77_tmp_timedate = timedate;
	resetTmpTimedateFlags();	//Resetting all flags to 0

	for(i = F; i < D; ++i) {
		for(j = MF; j < ER; ++j) {
			g_DCF77Control[i][j].new_state = F;
			g_DCF77Control[i][j].task = DCF77_decode_error;
		}
	}

	g_DCF77Control[F][MF].new_state = I; g_DCF77Control[F][MF].task = minuteframestart;
	g_DCF77Control[F][ER].new_state = F; g_DCF77Control[F][ER].task = DCF77_decode_error;

	g_DCF77Control[I][DB].new_state = I; g_DCF77Control[I][DB].task = putinbuffer;
	g_DCF77Control[I][DF].new_state = M; g_DCF77Control[I][DF].task = timedateframestart;
	g_DCF77Control[I][ER].new_state = F; g_DCF77Control[I][ER].task = DCF77_decode_error;

	g_DCF77Control[M][DB].new_state = M; g_DCF77Control[M][DB].task = putinbuffer;
	g_DCF77Control[M][PB].new_state = H; g_DCF77Control[M][PB].task = paritycheckminuteandset;
	g_DCF77Control[M][ER].new_state = F; g_DCF77Control[M][ER].task = DCF77_decode_error;

	g_DCF77Control[H][DB].new_state = H; g_DCF77Control[H][DB].task = putinbuffer;
	g_DCF77Control[H][PB].new_state = D; g_DCF77Control[H][PB].task = paritycheckhourandset;
	g_DCF77Control[H][ER].new_state = F; g_DCF77Control[H][ER].task = DCF77_decode_error;

	g_DCF77Control[D][DB].new_state = D; g_DCF77Control[D][DB].task = putinbuffer;
	g_DCF77Control[D][PB].new_state = F; g_DCF77Control[D][PB].task = paritycheckdateandfinish;
	g_DCF77Control[D][ER].new_state = F; g_DCF77Control[D][ER].task = DCF77_decode_error;

	//TODO I M H D �llapotok �sszevonhat�k

}