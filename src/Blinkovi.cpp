/*
 * Blinkovi.cpp
 *
 *  Created on: May 23, 2024
 *      Author: peca
 */

#include "app.h"
#include "Blinkovi.h"

Blinkovi::Blinkovi() {
	ledoff();
}

Blinkovi::~Blinkovi() {
	// TODO Auto-generated destructor stub
}




void Blinkovi::ledon(void){
	HAL_GPIO_WritePin(BOARD_LED_GPIO_Port, BOARD_LED_Pin, GPIO_PIN_RESET);
}

void Blinkovi::ledoff(void){
	HAL_GPIO_WritePin(BOARD_LED_GPIO_Port, BOARD_LED_Pin, GPIO_PIN_SET);
}

void Blinkovi::ledtogl(void) {
	HAL_GPIO_TogglePin(BOARD_LED_GPIO_Port, BOARD_LED_Pin);
}

void Blinkovi::trep(uint32_t ticks_on, uint32_t ticks_off){
	ledon();
	HAL_Delay(ticks_on);
	ledoff();
	HAL_Delay(ticks_off);
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
	HAL_Delay(ostatak);

}

