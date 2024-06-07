/*
 * Blinkovi.cpp
 *
 *  Created on: May 23, 2024
 *      Author: peca
 */


// pomocnici
#include "Blinkovi.h"



Blinkovi::Blinkovi() {
	pinMode(LED_pin, OUTPUT);
	ledoff();
}

Blinkovi::~Blinkovi() {

}




void Blinkovi::ledon(void){
	digitalWrite(LED_pin, LOW);
}

void Blinkovi::ledoff(void){
	digitalWrite(LED_pin, HIGH);
}

void Blinkovi::ledtogl(void) {
	int p = digitalRead(LED_pin)==HIGH;
	digitalWrite(LED_pin, (p ? LOW : HIGH));
}


void Blinkovi::trep(uint32_t ticks_on, uint32_t ticks_off){
	ledon();
	usleep(ticks_on * 1000);
	ledoff();
	usleep(ticks_off * 1000);
}

void Blinkovi::trepCnt(uint32_t count, uint32_t ticks_on, uint32_t ticks_off){
	for(uint32_t i = 0; i < count; i++){
		trep(ticks_on, ticks_off);
	}
}


void Blinkovi::trepCntPer(uint32_t count, uint32_t ticks_on, uint32_t ticks_off, uint32_t period){
	for(uint32_t i = 0; i < count; i++){
		trep(ticks_on, ticks_off);
	}
	uint32_t treptanje = count * (ticks_on + ticks_off);
	uint32_t ostatak;
	if (treptanje >= period) {
		ostatak = 1;
	} else {
		ostatak = period - (count * (ticks_on + ticks_off));
	}
	usleep(ostatak * 1000);
}

