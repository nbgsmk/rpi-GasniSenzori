//============================================================================
// Name        : rpi.cpp
// Author      : z
// Description : Demo applikacija pokazuje kako se koristi gasni senzor. I jos par testova.
//============================================================================


// C++
#include "app.h"
#include <cstdint>
#include <errno.h>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

// hardware driver
#include <wiringPi.h>
#include <wiringSerial.h>
#include "UartMux.h"
#include "GasSensor.h"

// pomocnici
#include "Blinkovi.h"


using namespace std;

int main() {
	cout << "Hey hey" << endl;

	if (  (uartFileDescriptor=serialOpen (hwUart_ttyS0, 9600)) >= 0  ) {
		// cout << "uart=" << hwUart_ttyS0 << endl;

	} else if ( (uartFileDescriptor=serialOpen (hwUart_serial0, 9600)) >= 0 ) {
		// cout << "uart=" << hwUart_serial0 << endl;

	} else {
		cerr << "Could not open " << hwUart_ttyS0 << " or " << hwUart_serial0 << ". We should be running _only_ on rpi zero w, rpi 3 or rpi 4." << endl;
		std::throw_with_nested(std::runtime_error("Did you forget to run raspi-config to enable serial peripheral?"));
	}

	int we = wiringPiSetupGpio(); // Initializes wiringPi using the Broadcom GPIO pin numbers
	if (we == -1) {
		cerr << "Unable to start wiringPi! Error " << errno << endl;
		std::throw_with_nested(std::runtime_error("Unable to start wiringPi: %s\n"));
	}

	pinMode(UartMuX_pinS1, OUTPUT);
	pinMode(UartMuX_pinS2, OUTPUT);
	pinMode(UartMuX_pinS3, OUTPUT);

	while (1) {

		Blinkovi *b = new Blinkovi();
		GasSensor *co = new GasSensor(adr_CO, uartFileDescriptor);
		GasSensor *h2s = new GasSensor(adr_H2S, uartFileDescriptor);

		b->trep(1000, 1000);
		b->trep(1000, 1000);

		int blok = 1;
		switch (blok) {

			////////////////////
			// TALK TO SENSOR(s)
			////////////////////
			case 1: {
				b->trepCnt(blok, 5, 250);

				// CO senzor
				cout << "mux " << adr_CO << endl;
				cout << "CO init " << endl;
				co->init(2000);
				cout << "ok" << endl;

				for (;;) {
					cout << "----- talk to CO -----" << endl;
					cout << "toggle running led a few times, just to know we are here" << endl;
					for (int i = 0; i < 4; ++i) {
						b->trep(5, 50);
						if (co->getLedStatus()) {
							co->setLedOff();
						} else {
							co->setLedOn();
						}
						usleep(2000 * 1000);
					}

					b->trep(5, 50);
					b->trep(5, 50);
					int ppm = co->getGasConcentrationPpm();
					int mg = co->getGasConcentrationMgM3();
					int percOfMax = co->getGasPercentageOfMax();
					float celsius = co->getTemperature();
					float rh = co->getRelativeHumidity();

					cout << "---- measure ----" << endl;
					cout << "gas ppm " << ppm << "\n";
					cout << "gas mg/m3 " << mg << "\n";
					cout << "gas percentage of max scale " << percOfMax << "\n";
					cout << "temperature " << celsius << " C \n";
					cout << "humidity " << rh << " % \n";
					cout << "---- done ----\n" << endl;


					usleep(2000 * 1000);

					// pretvaramo se da imamo jos neki senzor
					b->trep(5, 50);
					b->trep(5, 50);
					b->trep(5, 50);
					cout << "---- talk to H2S ----" << endl;
					cout << "---- fejk, ne radim nista, samo malo cekam ----" << endl;
					h2s->sendRawCommand("proba", sizeof("proba"));
					usleep(3000 * 1000);
					cout << "---- done ----\n" << endl;

				}
				break;
			}




			/////////////////////
			// TALK TO DEBUG UART
			/////////////////////
			case 2: {
				for ( ; ; ) {
					b->trepCnt(blok, 5, 250);

					const char s[] = {'l', 'e', 'v', 'o', ' ', 'd', 'e', 's', 'n', 'o', '\n'};
//					char s[] = { 0xFF,       0x01,       0x78,            0x40,       0x00,    0x00,     0x00,    0x00,    0x47};

					co->sendRawCommand(s, sizeof(s));
					usleep(1000 * 1000);
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

					mux->setAddr(2);		// raw num
					usleep(1000 * 1000);

					mux->setAddr(adr_CO);	// logical address
					usleep(1000 * 1000);

					mux->setAddr(adr_O2);
					usleep(1000 * 1000);

					mux->setAddr(adr_itd);
					usleep(1000 * 1000);
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
