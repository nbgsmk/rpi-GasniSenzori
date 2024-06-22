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
		GasSensor *o2 = new GasSensor(adr_O2, uartFileDescriptor);

		b->trep(1000, 1000);
		b->trep(1000, 1000);

		/**
		 * blok 1 = talk to sensors normal
		 * blok 2 = nekakav debug uart test
		 * blok 3 = sensor response time (ispada da je oko 30-4mS)
		 * blok 4 = UartMux test
		 * blok 5 = svaki senzor bi morao da zna svoju adresu
		 */

		int blok = 1;
		cout << "test blok " << blok << endl;

		switch (blok) {

			////////////////////
			// TALK TO SENSOR(s)
			////////////////////
			case 1: {
				b->trepCnt(blok, 5, 250);
				for (;;) {
					///////////////////////////
					///////////////////////////
					// -------- CO -------- //
					///////////////////////////
					///////////////////////////
					cout << "----- talk to CO -----" << endl;
					cout << "mux will be " << adr_CO << endl;
					cout << "init " << endl;
					co->init(2000);
					cout << "init ok" << endl;
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

					cout << "---- sensor tip ----" << endl;
					int ht = co->getSensorTypeHex();
					cout << "hex=" << std::hex << ht << std::dec << endl;

					std::string st = co->getSensorTypeStr();
					cout << "str=" << st << endl;


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
					cout << endl;
					usleep(5000 * 1000);


					///////////////////////////
					///////////////////////////
					// -------- H2S -------- //
					///////////////////////////
					///////////////////////////
					cout << "----- talk to H2S -----" << endl;
					cout << "mux will be " << adr_H2S << endl;
					cout << "init " << endl;
					h2s->init(2000);
					cout << "init ok" << endl;
					cout << "toggle sensor running led a few times, just to know we are here" << endl;
					for (int i = 0; i < 3; ++i) {
						b->trep(5, 50);
						if (h2s->getLedStatus()) {
							cout << "led should be off" << endl;
							h2s->setLedOff();
						} else {
							cout << "led should be on" << endl;
							h2s->setLedOn();
						}
						usleep(1000 * 2000);
					}

					cout << "---- sensor tip ----" << endl;
					ht = h2s->getSensorTypeHex();
					cout << "hex=" << std::hex << ht << std::dec << endl;

					st = h2s->getSensorTypeStr();
					cout << "str=" << st << endl;


					b->trep(5, 50);
					b->trep(5, 50);
					cout << "---- measure ----" << endl;

					dec = h2s->getDecimals();
					cout << "br decimala " << dec << endl;

					ppm = h2s->getGasConcentrationPpm();
					cout << "gas ppm " << ppm << endl;

					mg = h2s->getGasConcentrationMgM3();
					cout << "gas mg/m3 " << mg << endl;

					percOfMax = h2s->getGasPercentageOfMax();
					cout << "gas percentage of max scale " << percOfMax << endl;

					celsius = h2s->getTemperature();
					cout << "temperature " << celsius << " C " << endl;

					rh = h2s->getRelativeHumidity();
					cout << "humidity " << rh << " % " << endl;

					cout << "----  done  ----\n" << endl;
					cout << endl;
					usleep(5000 * 1000);



					///////////////////////////
					///////////////////////////
					// -------- O2 -------- //
					///////////////////////////
					///////////////////////////
					cout << "----- talk to O2 -----" << endl;
					cout << "mux will be " << adr_O2 << endl;
					cout << "init " << endl;
					o2->init(2000);
					cout << "init ok" << endl;
					cout << "toggle sensor running led a few times, just to know we are here" << endl;
					for (int i = 0; i < 3; ++i) {
						b->trep(5, 50);
						if (o2->getLedStatus()) {
							cout << "led should be off" << endl;
							o2->setLedOff();
						} else {
							cout << "led should be on" << endl;
							o2->setLedOn();
						}
						usleep(1000 * 2000);
					}

					cout << "---- sensor tip ----" << endl;
					ht = o2->getSensorTypeHex();
					cout << "hex=" << std::hex << ht << std::dec << endl;

					st = o2->getSensorTypeStr();
					cout << "str=" << st << endl;


					b->trep(5, 50);
					b->trep(5, 50);
					cout << "---- measure ----" << endl;

					dec = o2->getDecimals();
					cout << "br decimala " << dec << endl;

					ppm = o2->getGasConcentrationPpm();
					cout << "gas ppm " << ppm << endl;

					mg = o2->getGasConcentrationMgM3();
					cout << "gas mg/m3 " << mg << endl;

					percOfMax = o2->getGasPercentageOfMax();
					cout << "gas percentage of max scale " << percOfMax << endl;

					celsius = o2->getTemperature();
					cout << "temperature " << celsius << " C " << endl;

					rh = o2->getRelativeHumidity();
					cout << "humidity " << rh << " % " << endl;

					cout << "----  done  ----\n" << endl;
					cout << endl;
					usleep(5000 * 1000);

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

					mux->setAddr(adr_H2S);
					cout << "mux addr " << mux->getAddr() << endl;
					usleep(1000 * 2000);

					mux->setAddr(adr_itd);
					cout << "mux addr " << mux->getAddr() << endl;
					usleep(1000 * 2000);

					for (int aa = 8; aa > 0; --aa) {
						mux->setAddr(aa);
						cout << "mux addr " << mux->getAddr() << endl;
						usleep(1000 * 100);
					}
					cout << endl;
				}
				break;
			}



			////////////////////////////
			// Svaki senzor zna sa sebe
			///////////////////////////
			case 5: {
				GasSensor *sa = new GasSensor(4, uartFileDescriptor);
				GasSensor *sb = new GasSensor(5, uartFileDescriptor);
				GasSensor *sc = new GasSensor(6, uartFileDescriptor);
				sa->init(2000);
				sb->init(2000);
				sc->init(2000);

				cout << "-------------- self-aware -------------" << endl;
				cout << "--- ko je na ovoj adresi i sta meri ---" << endl;
				cout << "---------------------------------------" << endl;
				cout << "Ja sam " << sa->getSensorTypeStr() << ", na adresi " << sa->getMuxAddress() << ", merim " << sa->getGasConcentrationPpm() << " ppm, temperatura je " << sa->getTemperature() << endl;
				cout << "Ja sam " << sb->getSensorTypeStr() << ", na adresi " << sb->getMuxAddress() << ", merim " << sb->getGasConcentrationPpm() << " ppm, temperatura je " << sb->getTemperature() << endl;
				cout << "Ja sam " << sc->getSensorTypeStr() << ", na adresi " << sc->getMuxAddress() << ", merim " << sc->getGasConcentrationPpm() << " ppm, temperatura je " << sc->getTemperature() << endl;
				cout << endl;
				cout << "-------- ukratko --------" << endl;
				cout << "senzor " << sb->getSensorTypeStr() << ", ppm=" << sb->getGasConcentrationPpm() << endl;
				cout << "senzor " << sc->getSensorTypeStr() << ", ppm=" << sc->getGasConcentrationPpm() << endl;
				cout << endl;
				cout << "------- par nasumicnih komandi za kraj -------" << endl;
				cout << "senzor " << sa->getSensorTypeStr() << ", senzor " << sc->getSensorTypeStr() << endl;
				cout << "senzor " << sb->getSensorTypeStr() << " prisutan" << endl;

				cout << "senzor na nepostojecoj adresi 2 ?" << endl;
				GasSensor *gx = new GasSensor(2, uartFileDescriptor);
				gx->init(1000);
				cout << "nepostojeci meri " << gx->getGasConcentrationPpm() << endl;
				cout << "------------------- toliko -------------------" << endl;
				return 0;

				break;
			}

		}

		for (;;) {
			b->trep(5, 50);
		}

	}

	return 0;
}
