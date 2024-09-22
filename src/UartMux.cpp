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
	setAddr(adr_PWRON);
	pinMode(UartMuX_pinS1, OUTPUT);
	pinMode(UartMuX_pinS2, OUTPUT);
	pinMode(UartMuX_pinS3, OUTPUT);
}

UartMux::~UartMux() {
	setAddr(adr_PWRON);
}


/**
 * Aktiviram UartMux port po zelji [1..8]
 * Vrednosti izvan tog opsega se ignorisu - nema promene.
 * Tupsoni!! Na plocici pise [1..8]. Ko jos u tehnici ne koristi zero-based??
 * 
 * @return ista vrednost = ok. prethodno stanje = nije izvrsena promena
 */
bool UartMux::setAddr(MuxAdr_t portNum1to8){
	if ( (portNum1to8 >=1) && (portNum1to8 <= 8) ){
		this->hwAddr = portNum1to8 - 1;		// oduzmem 1 jer na stampanoj plocici pise [1..8] a hardverski je [0..7]

		// rpi specific
		// wiringPi specific
		digitalWrite(UartMuX_pinS1, (hwAddr & 0b001) ? HIGH : LOW);
		digitalWrite(UartMuX_pinS2, (hwAddr & 0b010) ? HIGH : LOW);
		digitalWrite(UartMuX_pinS3, (hwAddr & 0b100) ? HIGH : LOW);
		usleep(100 * 1000);		// guard time for pins to settle
		return true;
	} else {
		return false;
	}	
}

/*
* Vraca trenutno aktivan mux port, prema numeraciji na stampanoj plocici P1-P8
*/
MuxAdr_t UartMux::getAddr() {
	return this->hwAddr+1;	// dodajem 1 da bi se printout na ekranu poklapao sa nazivom na plocici muxa P1-P8
}

