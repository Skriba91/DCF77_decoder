/*
 * main.c
 *
 *  Created on: 2019. okt. 20.
 *      Author: danis
 */

#include <stdio.h>
#include <stdlib.h>

#include "dcf77.h"

#define LENGTH 60

void printTime(timedate_temp time) {
	printf("%d.%d.%d %d:%d", time.year+2000, time.month, time.day, time.hour,
								time.minute);
}

dcf77_second simpletoSeconds(uint8_t bit) {
	dcf77_second second;
	//Code one
	if(bit == 1) {
		second.low_duration = 100;
		second.high_duration = 900;
	}
	//Code minute sync
	else if(bit == 2) {
		second.low_duration = 2000;
		second.high_duration = 1800;
	}
	//Code zero
	else {
		second.low_duration = 200;
		second.high_duration = 800;
	}
	return second;
}

void decode(int8_t bit) {
	static dcf77_decode_element contrl;
	static dcf77_decode_next_pos pos;
	static DCF77_decode_state decodeState = F;

	pos = dcf77_decode_next_bit(bit);
	contrl = g_decode_DCF77Control[decodeState][pos.e];
	contrl.task(pos.b);
	decodeState = contrl.new_state;

}

void sinc() {

}

//Simple main function to test basic operation
int main(int argc, char **argv) {
	uint8_t data[LENGTH] = {2,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,0,1,0,0,1,1,0,1,0,
							1,1,0,0,1,0,0,0,1,0,0,1,0,0,1,1,0,0,1,1,0,0,0,0,1,
							1,0,0,1,1,0,0,0,1}; //Sa, 19.10.19 11:35:00, SZ
	timedate_temp time;
	dcf77_demod_element contrl;
	dcf77_demod_next_pos pos;
	DCF77_demod_state demodState = IDLE;
	//TODO very BAD only for temporal tests
	init_demod(sinc, decode);
	init_decode(&time);
	for(uint8_t i = 0; i < LENGTH; ++i) {
		pos = dcf77_demod_next_level(simpletoSeconds(data[i]));
		contrl = g_demod_DCF77Control[demodState][pos.e];
		contrl.task(pos.d);
		demodState = contrl.new_state;
	}
	printTime(time);
	return 0;
}


