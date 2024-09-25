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
	cout << "Hey hey!" << endl;
	cout << "Tek da se zna, ovde imamo:" << endl;
	cout << "  TB600c CO - 100ppm" << endl;
	cout << "  TB600c H2S - 100ppm" << endl;
	cout << "  TB200b O2 - 25%" << endl;
	cout << endl;

	//---------------------------------------
	// Inicijalizacija i osnovni error kodovi
	//---------------------------------------

	H_STATUS = ErrCodes_t::NOT_DEFINED;

	if (  (uartFileDescriptor=serialOpen(hwUart_ttyS0, 9600)) >= 0  ) {
		// /dev/tty0 i /dev/serial0 su jedno te isto na Rpi
		H_STATUS = ErrCodes_t::OK;
		if (DEBUG_LEVEL > 0) {
			cout << "uart=" << hwUart_ttyS0 << " ok" << endl;
		}

	} else if ( (uartFileDescriptor=serialOpen(hwUart_serial0, 9600)) >= 0 ) {
		// /dev/tty0 i /dev/serial0 su jedno te isto na Rpi
		H_STATUS = ErrCodes_t::OK;
		if (DEBUG_LEVEL > 0) {
			cout << "uart=" << hwUart_serial0 << " ok" << endl;
		}

	} else {
		// nisam mogao da otvorim serijski port
		H_STATUS = ErrCodes_t::UART_NOT_AVAILABLE;
		if (DEBUG_LEVEL > 0) {
			cerr << "Could not open " << hwUart_ttyS0 << " or " << hwUart_serial0 << ". We should be running _only_ on rpi zero w, rpi 3 or rpi 4." << endl;
			cerr << "Did you forget to run raspi-config to enable serial peripheral?" << endl;
		}
		return H_STATUS;	// ako nemam uart, nema svrhe ici dalje
	}

	// UART je dostupan, za pocetak. Idemo dalje.

	int we = wiringPiSetupGpio(); // Initializes wiringPi using the Broadcom GPIO pin numbers
	if (we == -1) {
		H_STATUS = ErrCodes_t::WiringPi_NOT_AVAILABLE;
		if (DEBUG_LEVEL > 0) {
			cerr << "Unable to start wiringPi! Error " << errno << endl;
			cerr << "Unable to start wiringPi: %s\n" << endl;
		}
		return H_STATUS;	// ako nemam WiringPi, nema svrhe ici dalje
	}


	// UART i WiringPi su dostupni. Mozemo malo da odahnemo.

	pinMode(UartMuX_pinS1, OUTPUT);
	pinMode(UartMuX_pinS2, OUTPUT);
	pinMode(UartMuX_pinS3, OUTPUT);

	Blinkovi *b = new Blinkovi();

	cout << "kreiram senzore, testiram opcioni parametar DEBUG_LEVEL" << endl;
	cout << "app: ako je DEBUG_LEVEL=0, nece biti nikakvog prikaza" << endl;
	cout << endl;
	cout << "co-100 " << endl;
	GasSensor *co = new GasSensor(adr_CO, uartFileDescriptor, 0);			// default debug_level = 0
	cout << "app: ako nista nije napisao izmedju naziva senzora (npr. \'co\') i ovog reda, dobro je!" << endl;
	cout << endl;
	cout << "h2s-100 " << endl;
	GasSensor *h2s = new GasSensor(adr_H2S, uartFileDescriptor, 1);
	cout << "o2-25 " << endl;
	GasSensor *o2 = new GasSensor(adr_O2, uartFileDescriptor, 1);
	GasSensor *nepostojeci = new GasSensor(adr_itd, uartFileDescriptor, 0);
	cout << "app: ok!" << endl;
	cout << endl;
	

	//////////////////////////
	// Grupe raznih testova //
	//////////////////////////

	while (1) {

		b->trep(1000, 1000);
		b->trep(1000, 1000);

		/**
		 * blok 1 = talk to sensors normally
		 * blok 2 = send raw bytes to sensor 
		 * blok 3 = sensor response time (ispada da je oko 30-40mS)
		 * blok 4 = UartMux test
		 * blok 5 = svaki senzor bi morao da zna svoju adresu
		 * blok 6 = prozivka u krug
		 */

		int blok = 1;
		cout << endl;
		cout << "-----------" << endl;
		cout << "test blok " << blok << endl;
		cout << "-----------" << endl;

		switch (blok) {

			//////////////////////////////
			// TALK TO SENSORS NORMALLY //
			//////////////////////////////
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

					int ht = co->getSensorTypeHex();
					cout << "sensor tip hex " << std::hex << ht << std::dec << endl;

					std::string st = co->getSensorTypeStr();
					cout << "sensor tip str " << st << endl;


					b->trep(5, 50);
					b->trep(5, 50);
					cout << "---- measurements ----" << endl;

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

					ht = h2s->getSensorTypeHex();
					cout << "sensor tip hex " << std::hex << ht << std::dec << endl;

					st = h2s->getSensorTypeStr();
					cout << "sensor tip str " << st << endl;


					b->trep(5, 50);
					b->trep(5, 50);
					cout << "---- measurements ----" << endl;

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

					ht = o2->getSensorTypeHex();
					cout << "sensor tip hex " << std::hex << ht << std::dec << endl;

					st = o2->getSensorTypeStr();
					cout << "sensor tip str " << st << endl;


					b->trep(5, 50);
					b->trep(5, 50);
					cout << "---- measurements ----" << endl;

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


					//////////////////////////////////////
					//////////////////////////////////////
					// nepostojeci, pokvaren, iskljucen //
					//////////////////////////////////////
					//////////////////////////////////////
					cout << "----- talk to nepostojeci -----" << endl;
					cout << "mux will be " << 8 << endl;

					cout << "--------" << endl;
					cout << "set debug level 0 = silent, ne sme prijaviti NISTA sto ga nisam pitao" << endl;
					nepostojeci->setDebugLevel(0);
					nepostojeci->getDebugLevel();
					nepostojeci->getLedStatus();
					nepostojeci->getSensorTypeStr();
					nepostojeci->getGasConcentrationPpm();
					cout << "hajde reci error code " << nepostojeci->getErrorCode() << endl;
					cout << "hajde reci error count " << nepostojeci->getErrorCount() << endl;

					cout << "--------" << endl;
					cout << "set debug level 1 = minimal. Osim rezultata, moze biti i nekih gresaka na konzoli" << endl;
					nepostojeci->setDebugLevel(1);
					cout << "debug level je " << nepostojeci->getDebugLevel() << endl;
					cout << "get led status " << nepostojeci->getLedStatus() << endl;
					cout << "get sensor typ str " << nepostojeci->getSensorTypeStr() << endl;
					cout << "get gas ppm " << nepostojeci->getGasConcentrationPpm() << endl;
					cout << "error code " << nepostojeci->getErrorCode() << endl;
					cout << "error count " << nepostojeci->getErrorCount() << endl;



					cout << "--------" << endl;
					cout << "set debug level 2 = quite verbose" << endl;
					nepostojeci->setDebugLevel(2);
					cout << "debug level is" << nepostojeci->getDebugLevel() << endl;
					cout << "get led status " << nepostojeci->getLedStatus() << endl;
					cout << "get sensor typ str " << nepostojeci->getSensorTypeStr() << endl;
					cout << "get gas ppm " << nepostojeci->getGasConcentrationPpm() << endl;
					cout << "error code " << nepostojeci->getErrorCode() << endl;
					cout << "error count " << nepostojeci->getErrorCount() << endl;

				}
				break;
			}

			//////////////////////////
			// SEND RAW DATA TO SENSOR
			//////////////////////////
			case 2: {
				cout << "send raw bytes to sensor" << endl;
				cout << "------------------------" << endl;
				for (;;) {
					b->trepCnt(blok, 5, 250);

					const char s[] = { 'l', 'e', 'v', 'o', ' ', 'd', 'e', 's', 'n', 'o', '\n' };
					// char s[] = { 0xFF,       0x01,       0x78,            0x40,       0x00,    0x00,     0x00,    0x00,    0x47};

					co->sendRawBytes(s, sizeof(s));
					usleep(1000 * 1000);
				}
				break;
			}

			///////////////////////
			// SENSOR RESPONSE TIME
			///////////////////////
			case 3: {
				cout << "sensor response time" << endl;
				cout << "--------------------" << endl;
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

					vector<int> v;
					v.push_back(3);			// raw number je ok
					v.push_back(adr_CO);	// logicke adrese su ok
					v.push_back(adr_H2S);
					v.push_back(adr_itd);
					v.push_back(17);		// izvan opsega -> nema promene
					v.push_back(0);			// izvan opsega -> nema promene
					v.push_back(-66);		// izvan opsega -> nema promene

					for (int i = 0; i < v.size(); i++) {
						mux->setAddr(v.at(i));
						cout << "mux addr: set "<<v.at(i)<<", get " << mux->getAddr() << endl;
						usleep(1000 * 2000);						
					}

					for (int i = 12; i > -4; i--) {
						mux->setAddr(i);
						cout << "mux addr: set "<<i<<", get " << mux->getAddr() << endl;
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
				cout << "nepostojeci meri " << gx->getGasConcentrationPpm() << endl;
				cout << "------------------- toliko -------------------" << endl;
				return 0;

				break;
			}

			//////////////////////////
			// Prozivka u krug, u krug
			//////////////////////////
			case 6: {
				cout << "Prozivka!" << endl;
				cout << "---------" << endl;
				for ( ; ; ) {
					cout << endl;
					GasSensor *co = new GasSensor(adr_CO, uartFileDescriptor);
					cout << "--------" << endl;
					GasSensor *h2s = new GasSensor(adr_H2S, uartFileDescriptor);
					cout << "--------" << endl;
					GasSensor *o2 = new GasSensor(adr_O2, uartFileDescriptor);
					cout << "--------" << endl;
					cout << endl;
				}

			}

		}

		for (;;) {
			b->trep(5, 50);
		}

	}

	return 0;
}
