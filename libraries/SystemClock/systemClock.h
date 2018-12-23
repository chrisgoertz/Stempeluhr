/**
 * basic class for clock functions
 *
 * This software is writen under GPL v3
 * https://www.gnu.org/licenses/gpl-3.0.en.thml
 * Christian Görtz 2018
 */

#ifndef __SYSTEM_CLOCK_H__
#define __SYSTEM_CLOCK_H__
#include <Arduino.h>

class systemClock{
	byte seconds = 0;
	byte minutes = 0;
	byte hours = 0;


public:
	//constructor
	systemClock(byte h, byte m, byte s);
	//constructor accepts timestring from db "hh:mm:ss"
	systemClock(String s);
	//todo constructor for char array
	//###################################################
	//secondTick() has to be called every second
	//this method will do all the clock stuff
	void secondTick();
	//adjust seconds
	void setSeconds(byte s);
	//adjust minuts
	void setMinutes(byte m);
	//adjust hours
	void setHours(byte h);
	//set time to "hh:mm:ss"
	void setTime(String s);
	byte getSeconds();
	byte getMinutes();
	byte getHours();
	//get time as string "hh:mm:ss"
	String getTimeString();
	//put the time into *char "hh:mm:ss\0"
	void getTimeChars(char *arr);
};

#endif
