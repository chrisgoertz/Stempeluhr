#include "systemClock.h"

systemClock::systemClock(byte h, byte m, byte s) {
	this->hours = h;
	this->minutes = m;
	this->seconds = s;
}
systemClock::systemClock(String s){
	this->seconds = s.substring(6,8).toInt();
	this->minutes = s.substring(3,5).toInt();
	this->hours = s.substring(0,2).toInt();

}

void systemClock::secondTick() {
	this->seconds++;
	if (this->seconds > 59) {
		this->seconds = 0;
		this->minutes++;
		if (this->minutes > 59) {
			this->minutes = 0;
			this->hours++;
			if (this->hours > 23) {
				this->hours = 0;
			}
		}
	}
}
void systemClock::setSeconds(byte s){
	this->seconds = s > 0 && s < 59 ? s : 0;
}
void systemClock::setMinutes(byte m){
	this->seconds = m > 0 && m < 59 ? m : 0;
}
void systemClock::setHours(byte h){
	this->hours = h >0 && h <23 ? h : 0;
}
void systemClock::setTime(String s){
	this->seconds = s.substring(6,8).toInt();
	this->minutes = s.substring(3,5).toInt();
	this->hours = s.substring(0,2).toInt();

}
byte systemClock::getSeconds(){
	return this->seconds;
}
byte systemClock::getMinutes(){
	return this->minutes;
}
byte systemClock::getHours(){
	return this->hours;
}
String systemClock::getTimeString(){
	String retVal = "";
	if(this->hours < 10){
		retVal.concat("0");
	}
	retVal.concat(this->hours);
	//-------------------------
	retVal.concat(":");
	if(this->minutes < 10){
		retVal.concat("0");
	}
	//-------------------------
	retVal.concat(this->minutes);
	retVal.concat(":");

	if(this->seconds < 10){
		retVal.concat("0");
	}
	retVal.concat(this->seconds);
	return retVal;
}
void systemClock::getTimeChars(char *arr){



}
