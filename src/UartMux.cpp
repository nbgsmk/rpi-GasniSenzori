/*
 * UartMux.cpp
 *
 *  Created on: May 23, 2024
 *      Author: peca
 */


// hardware driver
#include "UartMux.h"

UartMux::UartMux() {

}

UartMux::~UartMux() {

}


/**
 * Aktiviram UartMux port po zelji [1..8]  (tupsoni! na plocici pise [1..8])
k *
 */
void UartMux::setAdr(MuxAdr_t muxAdresa){
	muxAdresa--;	// oduzmem 1 jer na stampanoj plocici pise [1..8] a hardverski je [0..7]
	muxAdresa = muxAdresa % 8;
	currentAddress = muxAdresa;
	uint16_t rez = 0;
	rez = (muxAdresa & 0b001) ? (rez | UartMuX_s1_Pin) : rez;
	rez = (muxAdresa & 0b010) ? (rez | UartMuX_s2_Pin) : rez;
	rez = (muxAdresa & 0b100) ? (rez | UartMuX_s3_Pin) : rez;

	// stm32 specific
	HAL_GPIO_WritePin(GPIOB, UartMuX_s1_Pin|UartMuX_s2_Pin|UartMuX_s3_Pin, GPIO_PIN_RESET);	// write zero
	HAL_GPIO_WritePin(GPIOB, rez, GPIO_PIN_SET);				// set only necessary bits

}


MuxAdr_t UartMux::getAdr() {
	return currentAddress;
}

