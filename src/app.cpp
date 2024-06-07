//============================================================================
// Name        : rpi.cpp
// Author      : z
// Version     :
// Copyright   :
// Description : Hello World in C++, Ansi-style
//============================================================================


// zajednicki
#include "app.h"

// hardware driver
#include "UartMux.h"
#include <wiringPi.h>

// pomocnici
#include "Blinkovi.h"
#include "GasSensor.h"


using namespace std;

int main() {
	cout << "Hey hey" << endl; // prints Hey hey

//	wiringPiSetupGpio();



	while (1) {

		Blinkovi *b = new Blinkovi();
		UartMux *mux = new UartMux();
		GasSensor *co = new GasSensor(adr_CO);
		GasSensor *h2s = new GasSensor(adr_H2S);

		HAL_Delay(2000);
		b->trep(50, 200);
		b->trep(50, 200);
		b->trep(50, 200);


		int blok = 1;
		switch (blok) {

			////////////////////
			// TALK TO SENSOR(s)
			////////////////////
			case 1: {
				b->trepCnt(blok, 5, 250);

				// CO senzor
				mux->setAdr(adr_CO);
				co->setSensorUart(1);
				co->setDebugUart(2);
				co->init(2000);

				for (;;) {
					mux->setAdr(adr_CO);
					for (int i = 0; i < 5; ++i) {
						b->trep(5, 50);
						if (co->getLedStatus()) {
							co->setLedOff();
						} else {
							co->setLedOn();
						}
						HAL_Delay(2000);
					}

					b->trep(5, 50);
					b->trep(5, 50);
					int ppm = co->getGasConcentrationPpm();
					int mg = co->getGasConcentrationMgM3();
					int percOfMax = co->getGasPercentageOfMax();
					float celsius = co->getTemperature();
					float rh = co->getRelativeHumidity();

					HAL_Delay(1000);

					// pretvaramo se da imamo jos neki senzor
					b->trep(5, 50);
					b->trep(5, 50);
					b->trep(5, 50);
					mux->setAdr(adr_H2S);
					HAL_Delay(3000);

				}
				break;
			}




			/////////////////////
			// TALK TO DEBUG UART
			/////////////////////
			case 2: {
				for (;;) {
					b->trepCnt(blok, 5, 250);

					co->setSensorUart(2);
					co->setDebugUart(2);
					uint8_t s[] = {'z', 'e', 'c'};
	//				uint8_t s[] = { 0xFF,       0x01,       0x78,            0x40,       0x00,    0x00,     0x00,    0x00,    0x47};

					co->sendRawCommand(s, sizeof(s));
				}
				break;
			}




			///////////////
			// TEST UARTMUX
			///////////////
			case 3: {
				UartMux *mux = new UartMux();
				for ( ; ; ) {
					b->trepCnt(blok, 5, 250);

					mux->setAdr(adr_CO);
					HAL_Delay(1000);

					mux->setAdr(adr_H2S);
					HAL_Delay(1000);

					mux->setAdr(adr_O2);
					HAL_Delay(1000);

					mux->setAdr(adr_itd);
					HAL_Delay(1000);
				}
				break;
			}

			default: {
				break;
			}

		}


		for (; ; ) {
			b->trep(5, 50);
		}


	}
	return 0;
}
