/*
 * main.c
 *
 *  Created on: 2019. okt. 20.
 *      Author: danis
 */

#include "dcf77.h"

#define LENGTH 59

//Simple main function to test basic operation
int main(int argc, char **argv) {
	uint8_t data[LENGTH] = {0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,0,1,0,0,1,1,0,1,0,
							1,1,0,0,1,0,0,0,1,0,0,1,0,0,1,1,0,0,1,1,0,0,0,0,1,
							1,0,0,1,1,0,0,0,1}; //Sa, 19.10.19 11:35:00, SZ
	dcf77_decode_element contrl;
	dcf77_decode_next_pos pos;
	timedate_temp time;
	DCF77_decode_state state = F;
	init_decode(&time);
	for(uint8_t i = 0; i < LENGTH; ++i) {
		pos = dcf77_decode_next_bit(data[i]);
		contrl = g_decode_DCF77Control[state][pos.e];
		contrl.task(pos.b);
		state = contrl.new_state;
	}
	return 0;
}


