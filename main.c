/*
 * main.c
 *
 *  Created on: 2019. okt. 20.
 *      Author: danis
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "dcf77.h"

#define LENGTH 60

#define COLOR_WHITE 7
#define COLOR_RED 4
#define COLOR_GREEN 2

//A struct to store datetime
typedef struct {
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t dayofweek;
	uint8_t month;
	uint8_t year;
} mytime;

void printTmpTime(timedate_temp time);

void printMyTime(mytime time);

void printSecond(dcf77_second second);

//Function for testing. Codes the input bit to second low-high intervals
dcf77_second simpletoSeconds(uint8_t bit);

//Function for testing. Reads a DCF77 input file witch starts with the
//demodulated bitstream with a terminating coma, than the date separated with
//dots and finally the time. Generaits a bit file with second intervals.
void createIntervalData(char* fileinName, char* fileoutName);

uint8_t isDateEqual(mytime date1, mytime date2);

void validate(mytime date1, mytime date2);

void decode(int8_t bit);

void tmpTimeToMytime(timedate_temp* tmptime, mytime* mtime);

void sinc();

timedate_temp g_tmp_time;
mytime g_mytime;

//Simple main function to test basic operation
int main(int argc, char **argv) {
	//TEST CASE 1: One minute frame
	if(argc == 1) {
		//Test data to test the demodulator and the decoder
		uint8_t data[LENGTH] = {2,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,0,1,0,0,1,1,0,1,0,
								1,1,0,0,1,0,0,0,1,0,0,1,0,0,1,1,0,0,1,1,0,0,0,0,1,
								1,0,0,1,1,0,0,0,1}; //2019.10.19 11:35:00
		dcf77_demod_element contrl;
		dcf77_demod_next_pos pos;
		DCF77_demod_state demodState = IDLE;
		mytime test_time;
		test_time.year = 19;
		test_time.month = 10;
		test_time.day = 19;
		test_time.hour = 11;
		test_time.minute = 35;
		//TODO very BAD only for temporal tests
		init_demod(sinc, decode);
		init_decode(&g_tmp_time);
		for(uint8_t i = 0; i < LENGTH; ++i) {
			pos = dcf77_demod_next_level(simpletoSeconds(data[i]));
			contrl = g_demod_DCF77Control[demodState][pos.e];
			contrl.task(pos.d);
			demodState = contrl.new_state;
		}
		sinc();
		validate(g_mytime, test_time);
	}
	//TEST CASE 2: Reading second intervall data from binary file
	else if(argc == 2){
		FILE *file;
		file = fopen(argv[1], "rb");
		if(!file) {
			printf("Failed to open file: %s\nExiting\n", argv[1]);
			exit(-6);
		}
		dcf77_second second[60];
		mytime time;
		dcf77_demod_element contrl;
		dcf77_demod_next_pos pos;
		DCF77_demod_state demodState = IDLE;
		init_demod(sinc, decode);
		init_decode(&g_tmp_time);
		while(fread(&second, sizeof(dcf77_second), 60, file)) {
			fread(&time, sizeof(mytime), 1, file);
			for(uint8_t i = 0; i < LENGTH; ++i) {
				pos = dcf77_demod_next_level(second[i]);
				contrl = g_demod_DCF77Control[demodState][pos.e];
				contrl.task(pos.d);
				demodState = contrl.new_state;
			}
			//Manual sync required for testing
			sinc();
			validate(g_mytime, time);
		}
		fclose(file);
	}
	//Creating a binary file from DCF77 log input file
	else if(argc == 3) {
		createIntervalData(argv[1], argv[2]);
	}
	return 0;
}

void printTmpTime(timedate_temp time) {
	printf("%4d.%02d.%02d %02d:%02d", time.year+2000, time.month, time.day, time.hour,
								time.minute);
}

void printMyTime(mytime time) {
	printf("%4d.%02d.%02d %02d:%02d", time.year+2000, time.month, time.day, time.hour,
								time.minute);
}

void printSecond(dcf77_second second) {
	printf("H: %d, L: %d\n", second.high_duration, second.low_duration);
}

//Function for testing. Codes the input bit to second low-high intervals
dcf77_second simpletoSeconds(uint8_t bit) {
	dcf77_second second;
	//Code one
	if(bit == 1) {
		second.low_duration = 200;
		second.high_duration = 800;
	}
	//Code minute sync
	else if(bit == 2) {
		second.low_duration = 200;
		second.high_duration = 1800;
	}
	//Code zero
	else {
		second.low_duration = 100;
		second.high_duration = 900;
	}
	return second;
}

//Function for testing. Reads a DCF77 input file witch starts with the
//demodulated bitstream with a terminating coma, than the date separated with
//dots and finally the time. Generaits a bit file with second intervals.
void createIntervalData(char* fileinName, char* fileoutName) {
	FILE* filein = fopen(fileinName, "r");		//Input file only to read
	FILE* fileout = fopen(fileoutName, "wb");	//Binary output file
	//If something wrong abort
	if(!filein) {
		printf("File read error... exiting\n");
		exit(-4);
	}
	else if (!fileout) {
			printf("File write error.. exiting\n");
			fclose(filein);		//Closing opened input file
			exit(-5);
		}
	else {
		mytime time;
		dcf77_second second;
		unsigned int rows = 1;
		//Values for a DCF77 minute frame
		second.low_duration = 200;
		second.high_duration = 1800;
		//Writing out the fist DCF77 minute sync
		fwrite(&second, sizeof(dcf77_second), 1, fileout);
		uint8_t inbit;
		uint8_t timecntr = 0;	//For counting timedate characters
		char c, state;
		//state == 0 means the input is demodulated DCF77 bitstream
		//state == 1 means the input is decoded DCF77 time
		state = 0;		// 0 means the input is demodulated DCF77 bitstream
		//Reads until end of file
		while ((c = (char)fgetc(filein)) != EOF) {
			//Ignoring spaces
			if(c != ' ') {
				//Incrementing state every time c is equal to ','
				if(c == ',') {
					++state;
				}
				//Every line starts with minute frame sync
				else if(c == '\n') {
					++rows;
					state = 0;
					timecntr = 0;
					second = simpletoSeconds(2);	//2 in the argument means frame
					fwrite(&second, sizeof(dcf77_second), 1, fileout);
				}
				//Reading DCF77 demodulated bits
				else if((state == 0) && ((c == '0') || (c == '1'))) {
					inbit = c - 48;						//Convert to integer
					second = simpletoSeconds(inbit);	//Remodulate bit
					fwrite(&second, sizeof(dcf77_second), 1, fileout);
				}
				//Reading decoded DCF77 datetime data
				else if(state == 1) {
					//Ignoring time and date separator
					if((c != '.') && (c != ':')) {
						inbit = c - 48;
						switch(timecntr) {
						case 0 : time.day = 10 * inbit; break;
						case 1 : time.day += inbit; break;
						case 2 : time.month = 10 * inbit; break;
						case 3 : time.month += inbit; break;
						case 4 : time.year = 10 * inbit; break;
						case 5 : time.year += inbit; break;
						case 6 : time.hour = 10 * inbit; break;
						case 7 : time.hour += inbit; break;
						case 8 : time.minute = 10 * inbit; break;
						case 9 : time.minute += inbit; break;
						}
						++timecntr;
					}
				}
				else if(state == 2) {
					++state;
					printMyTime(time);
					printf("\n");
					fwrite(&time, sizeof(mytime), 1, fileout);
				}
			}
		}
		printf("Processed rows: %d\n", rows);
		//Closing files
		fclose(filein);
		fclose(fileout);
	}
}

uint8_t isDateEqual(mytime date1, mytime date2) {
	if((date1.year == date2.year) &&
	   (date1.month == date2.month) &&
	   (date1.day == date2.day) &&
	   (date1.hour == date2.hour) &&
	   (date1.minute == date2.minute))
	{
		return 1;
	}
	else {
		return 0;
	}
}

void validate(mytime date1, mytime date2) {
	HANDLE  hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	printf("Decoded datetime: ");
	printMyTime(date1);
	printf(" ");
	printf("Expected datetime: ");
	printMyTime(date2);
	printf(" Result: ");
	if(isDateEqual(date1, date2)) {
		SetConsoleTextAttribute(hConsole, COLOR_GREEN);
		printf("OK\n");
		SetConsoleTextAttribute(hConsole, COLOR_WHITE);
	}
	else {
		SetConsoleTextAttribute(hConsole, COLOR_RED);
		printf("ERROR\n");
		SetConsoleTextAttribute(hConsole, COLOR_WHITE);
	}
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

void tmpTimeToMytime(timedate_temp* tmptime, mytime* mtime) {
	mtime->year = tmptime->year;
	mtime->month = tmptime->month;
	mtime->day = tmptime->day;
	mtime->hour = tmptime->hour;
	mtime->minute = tmptime->minute;

}

void sinc() {
	tmpTimeToMytime(&g_tmp_time, &g_mytime);
}


