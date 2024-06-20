/*
 * UartMux.cpp
 *
 *  Created on: May 23, 2024
 *      Author: peca
 */

// C++
#include <cstdint>
#include <unistd.h>

// rpi specific
#include <wiringPi.h>

// hardware driver
#include "UartMux.h"


UartMux::UartMux() {
	this->currentAddress = adr_DONOTUSE_1;
	pinMode(UartMuX_pinS1, OUTPUT);
	pinMode(UartMuX_pinS2, OUTPUT);
	pinMode(UartMuX_pinS3, OUTPUT);
}

UartMux::~UartMux() {
	this->currentAddress = adr_DONOTUSE_1;
}


/**
 * Aktiviram UartMux port po zelji [1..8]  (tupsoni! na plocici pise [1..8])
k *
 */
void UartMux::setAddr(MuxAdr_t muxAdresa){
	muxAdresa--;					// oduzmem 1 jer na stampanoj plocici pise [1..8] a hardverski je [0..7]
	muxAdresa = muxAdresa % 8;		// najveci broj je 7
	this->currentAddress = muxAdresa;

	// rpi specific
	// wiringPi specific
	digitalWrite(UartMuX_pinS1, (muxAdresa & 0b001) ? HIGH : LOW);
	digitalWrite(UartMuX_pinS2, (muxAdresa & 0b010) ? HIGH : LOW);
	digitalWrite(UartMuX_pinS3, (muxAdresa & 0b100) ? HIGH : LOW);
	usleep(100 * 1000);		// guard time for pins to settle
}


MuxAdr_t UartMux::getAddr() {
	return currentAddress+1;	// dodajem 1 da bi se printout na ekranu poklapao sa nazivom na plocici muxa
}

