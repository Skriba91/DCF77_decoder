/*
 * dcf77.c
 *
 *  Created on: 2019. okt. 20.
 *  Author: Skriba Dániel
 *
 */


#include "dcf77.h"

//Initialise state model
void init_decode() {
	DCF77_decode_state i;
	DCF77_decode_event j;

	for(i = F; i < E; ++i) {
		for(j = MF; j < ER; ++j) {
			g_DCF77Control[i][j].new_state = E;
			g_DCF77Control[i][j].task = DCF77_decode_error;
		}
	}

}

