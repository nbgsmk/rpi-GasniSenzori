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
				cout << "init ok" << endl;

				for (;;) {
					cout << "----- talk to CO -----" << endl;
					cout << "toggle sensor running led a few times, just to know we are here" << endl;
					for (int i = 0; i < 3; ++i) {
						b->trep(5, 50);
						if (co->getLedStatus()) {
							cout << "led should be off" << endl;
							co->setLedOff();
						} else {
							cout << "led should be on" << endl;
							co->setLedOn();
						}
						usleep(1000 * 2500);
					}

					b->trep(5, 50);
					b->trep(5, 50);
					cout << "---- measure ----" << endl;

					int dec = co->getDecimals();
					cout << "br decimala " << dec << endl;

					int ppm = co->getGasConcentrationPpm();
					cout << "gas ppm " << ppm << endl;

					int mg = co->getGasConcentrationMgM3();
					cout << "gas mg/m3 " << mg << endl;

					int percOfMax = co->getGasPercentageOfMax();
					cout << "gas percentage of max scale " << percOfMax << endl;

					float celsius = co->getTemperature();
					cout << "temperature " << celsius << " C " << endl;

					float rh = co->getRelativeHumidity();
					cout << "humidity " << rh << " % " << endl;

					cout << "----  done  ----\n" << endl;

					usleep(5000 * 1000);

					// pretvaramo se da imamo jos neki senzor
					b->trep(5, 50);
					b->trep(5, 50);
					b->trep(5, 50);
					cout << "---- talk to H2S ----" << endl;
					cout << "---- fejk, ne radim nista, samo malo cekam ----" << endl;
					h2s->sendRawBytes("proba", sizeof("proba"));
					usleep(3000 * 1000);
					cout << "---- done ----\n" << endl;

				}
				break;
			}

			/////////////////////
			// TALK TO DEBUG UART
			/////////////////////
			case 2: {
				for (;;) {
					b->trepCnt(blok, 5, 250);

					const char s[] = { 'l', 'e', 'v', 'o', ' ', 'd', 'e', 's', 'n', 'o', '\n' };
//					char s[] = { 0xFF,       0x01,       0x78,            0x40,       0x00,    0x00,     0x00,    0x00,    0x47};

					co->sendRawBytes(s, sizeof(s));
					usleep(1000 * 1000);
				}
				break;
			}

			///////////////////////
			// SENSOR RESPONSE TIME
			///////////////////////
			case 3: {
				for (;;) {
					b->trepCnt(blok, 5, 100);

					// LED OFF
					const std::vector<uint8_t> cmd_running_light_off = { 0xFF, 0x01, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77 };
					const CmdStruct_t cmdRunningLightOff = { cmd_running_light_off, 2, false };		// slagacu da nije bitan checksum

					// LED ON
					const std::vector<uint8_t> cmd_running_light_on = { 0xFF,       0x01,    0x89,     0x00,   0x00,    0x00,     0x00,    0x00,    0x76};
					const CmdStruct_t cmdRunningLightOn = { cmd_running_light_on, 2, false };		// slagacu da nije bitan checksum

					// LED STATUS
					const std::vector<uint8_t> cmd_running_light_get_status = { 0xFF,       0x01,    0x8A,     0x00,   0x00,    0x00,     0x00,    0x00,    0x75 };
					const CmdStruct_t cmdRunningLightGetStatus = { cmd_running_light_get_status, 9, true };			// slagacu da nije bitan checksum

					co->send(cmdRunningLightGetStatus);
					usleep(1000 * 1000);
				}
				break;
			}

			///////////////
			// TEST UARTMUX
			///////////////
			case 4: {
				UartMux *mux = new UartMux();
				for (;;) {
					cout << "uartmux test" << endl;
					cout << "------------" << endl;
					b->trepCnt(blok, 5, 250);

					mux->setAddr(2);		// raw num
					cout << "mux addr " << mux->getAddr() << endl;
					usleep(1000 * 2000);

					mux->setAddr(adr_CO);	// logical address
					cout << "mux addr " << mux->getAddr() << endl;
					usleep(1000 * 2000);

					mux->setAddr(adr_O2);
					cout << "mux addr " << mux->getAddr() << endl;
					usleep(1000 * 2000);

					mux->setAddr(adr_itd);
					cout << "mux addr " << mux->getAddr() << endl;
					usleep(1000 * 2000);

					cout << endl;
				}
				break;
			}


		}

		for (;;) {
			b->trep(5, 50);
		}

	}

	return 0;
}
