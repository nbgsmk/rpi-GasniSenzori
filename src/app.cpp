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
// timestamp only
#include <chrono>
#include <ctime>
// #include <format>

// hardware driver
#include <wiringPi.h>
#include <wiringSerial.h>
#include "UartMux.h"
#include "GasSensor.h"

// pomocnici
// #include "Blinkovi.h"


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

	cout << "kreiram senzore, testiram opcioni parametar DEBUG_LEVEL u konstruktoru" << endl;
	cout << "app: ako je DEBUG_LEVEL=0 ili se izostavi, nece biti nikakvog prikaza" << endl;
	cout << "app: co-100 " << endl;
	GasSensor *co = new GasSensor(adr_CO, uartFileDescriptor, 0);			// default debug_level = 0
	cout << "app: h2s-100 " << endl;
	GasSensor *h2s = new GasSensor(adr_H2S, uartFileDescriptor, 0);
	cout << "app: o2-25 " << endl;
	GasSensor *o2 = new GasSensor(adr_O2, uartFileDescriptor, 0);
	cout << "app: na primer neispravan senzor " << endl;
	GasSensor *nepostojeci = new GasSensor(adr_itd, uartFileDescriptor, 0);
	cout << "app: osim tagova (\'app: co-100\'...), mora biti silent cak i ako ima gresaka" << endl;
	cout << "app: sensors created" << endl;
	cout << endl;
	

	//////////////////////////
	// Grupe raznih testova //
	//////////////////////////

	while (1) {

		/**
		 * blok 1 = talk to sensors normally
		 * blok 2 = send raw bytes to sensor 
		 * blok 3 = sensor response time (Rpi4: 100..110mS. Varijacije na uart test rigu 80..150mS)
		 * blok 4 = UartMux test
		 * blok 5 = svaki senzor mora da zna svoj tip i adresu
		 * blok 51= isto kao 5 ali output formatiran kao csv, tsv...
		 * blok 6 = prozivka u krug
		 * blok 7 = DEBUG_LEVEL test + senzor ispravan
		 * blok 8 = DEBUG_LEVEL test + senzor NIJE ispravan
		 * blok 9 = unosim greske u uart vezu i prikazujem sta se desava
		 */

		int blok = 51;
		cout << endl;
		cout << "-----------" << endl;
		cout << "TEST BLOK " << blok << endl;
		cout << "-----------" << endl;

		switch (blok) {

			//////////////////////////////
			// TALK TO SENSORS NORMALLY //
			//////////////////////////////
			case 1: {
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


					cout << "---- measurements ----" << endl;

					int dec = co->getDecimals();
					cout << "br decimala " << dec << endl;

					auto ppm = co->getGasConcentrationParticles();
					cout << "gas particles " << co->getGasConcentrationParticles() << " " << co->getUnitsForParticles() << endl;

					auto mg = co->getGasConcentrationMass_DO_NOT_USE();
					cout << "gas vol. " << co->getGasConcentrationMass_DO_NOT_USE() << " " << co->getUnitsForMass_DO_NOT_USE() << endl;

					auto percOfMax = co->getGasPercentageOfMaxParticles();
					cout << "gas percentage of max scale " << percOfMax << endl;

					auto celsius = co->getTemperature();
					cout << "temperature " << celsius << " C " << endl;

					auto rh = co->getRelativeHumidity();
					cout << "humidity " << rh << " % " << endl;

					cout << "----  done  ----\n" << endl;
					cout << endl;
					usleep(7000 * 1000);


					///////////////////////////
					///////////////////////////
					// -------- H2S -------- //
					///////////////////////////
					///////////////////////////
					cout << "----- talk to H2S -----" << endl;
					cout << "mux will be " << adr_H2S << endl;
					cout << "toggle sensor running led a few times, just to know we are here" << endl;
					for (int i = 0; i < 3; ++i) {
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


					cout << "---- measurements ----" << endl;

					dec = h2s->getDecimals();
					cout << "br decimala " << dec << endl;

					ppm = h2s->getGasConcentrationParticles();
					cout << "gas particles " << h2s->getGasConcentrationParticles() << " " << h2s->getUnitsForParticles() << endl;

					mg = h2s->getGasConcentrationMass_DO_NOT_USE();
					cout << "gas vol. " << h2s->getGasConcentrationMass_DO_NOT_USE() << " " << h2s->getUnitsForMass_DO_NOT_USE() << endl;

					percOfMax = h2s->getGasPercentageOfMaxParticles();
					cout << "gas percentage of max scale " << percOfMax << endl;

					celsius = h2s->getTemperature();
					cout << "temperature " << celsius << " C " << endl;

					rh = h2s->getRelativeHumidity();
					cout << "humidity " << rh << " % " << endl;

					cout << "----  done  ----\n" << endl;
					cout << endl;
					usleep(7000 * 1000);



					///////////////////////////
					///////////////////////////
					// -------- O2 -------- //
					///////////////////////////
					///////////////////////////
					cout << "----- talk to O2 -----" << endl;
					cout << "mux will be " << adr_O2 << endl;
					cout << "toggle sensor running led a few times, just to know we are here" << endl;
					for (int i = 0; i < 3; ++i) {
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


					cout << "---- measurements ----" << endl;

					dec = o2->getDecimals();
					cout << "br decimala " << dec << endl;

					ppm = o2->getGasConcentrationParticles();
					cout << "gas particles " << o2->getGasConcentrationParticles() << " " << o2->getUnitsForParticles() << endl;

					mg = o2->getGasConcentrationMass_DO_NOT_USE();
					cout << "gas vol. " << o2->getGasConcentrationMass_DO_NOT_USE() << " " << o2->getUnitsForMass_DO_NOT_USE() << endl;

					percOfMax = o2->getGasPercentageOfMaxParticles();
					cout << "gas percentage of max scale " << percOfMax << endl;

					celsius = o2->getTemperature();
					cout << "temperature " << celsius << " C " << endl;

					rh = o2->getRelativeHumidity();
					cout << "humidity " << rh << " % " << endl;

					cout << "----  done  ----\n" << endl;
					cout << endl;
					usleep(7000 * 1000);
				}
				break;
			}

			//////////////////////////
			// SEND RAW DATA TO SENSOR
			//////////////////////////
			case 2: {
				//////////////////////////////////////////
				//////////////////////////////////////////
				int lev = 4;
				cout << "debug=" << lev << endl;
				co->setDebugLevel(lev);
				cout << "send raw bytes to sensor" << endl;
				cout << "------------------------" << endl;
				for (;;) {

					const char s[] = { 'l', 'e', 'v', 'o', ' ', 'd', 'e', 's', 'n', 'o', '\n' };
					// char s[] = { 0xFF,       0x01,       0x78,            0x40,       0x00,    0x00,     0x00,    0x00,    0x47};

					co->sendRawBytes(s, sizeof(s));
					usleep(1000 * 5000);
				}
				break;
			}

			///////////////////////
			// SENSOR RESPONSE TIME
			///////////////////////
			case 3: {
				cout << "sensor response time" << endl;
				cout << "testiram samo jednu komandu" << endl;
				cout << "---------------------------------" << endl;
				cout << "neophodan je debug level >=2" << endl;
				GasSensor *xr = new GasSensor(adr_H2S, uartFileDescriptor, 2);

				for (;;) {
					cout << "get typ " << xr->getSensorTypeStr() << endl;
					float pp = xr->getGasConcentrationParticles();		// ovo prijavljuje svoj response time
					string uu = xr->getUnitsForParticles();				// ovo samo cita ranije popunjen properties, ne oduzima nikakvo vreme u runtime
					cout << "get parts=" << pp << uu << endl;
					usleep(1000 * 5000);
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
						usleep(1000 * 300);						
					}
					cout << endl;
				}
				break;
			}



			////////////////////////////////////////////////
			// Svaki senzor zna za samog sebe ko je i sta je
			////////////////////////////////////////////////
			case 5: {
				GasSensor *sa = new GasSensor(4, uartFileDescriptor);
				GasSensor *sb = new GasSensor(5, uartFileDescriptor);
				GasSensor *sc = new GasSensor(3, uartFileDescriptor);

				for (;;){
					cout << "-------------- self-aware -------------" << endl;
					cout << "--- ko je na ovoj adresi i sta meri ---" << endl;
					cout << "---------------------------------------" << endl;
					cout << "Ja sam " << sa->getSensorTypeStr() << "\t na adresi " << sa->getMuxAddress() << "\t merim " << sa->getGasConcentrationParticles() << " " << sa->getUnitsForParticles() << "\t T=" << sa->getTemperature() << "°C, \t rh=" << sa->getRelativeHumidity() << "%" << endl;
					cout << "Ja sam " << sb->getSensorTypeStr() << "\t na adresi " << sb->getMuxAddress() << "\t merim " << sb->getGasConcentrationParticles() << " " << sb->getUnitsForParticles() << "\t T=" << sb->getTemperature() << "°C, \t rh=" << sb->getRelativeHumidity() << "%" << endl;
					cout << "Ja sam " << sc->getSensorTypeStr() << "\t na adresi " << sc->getMuxAddress() << "\t merim " << sc->getGasConcentrationParticles() << " " << sc->getUnitsForParticles() << "\t T=" << sc->getTemperature() << "°C, \t rh=" << sc->getRelativeHumidity() << "%" << endl;
					cout << endl;

					cout << "senzor na nepostojecoj adresi 2 ?" << endl;
					GasSensor *nep = new GasSensor(8, uartFileDescriptor);
					cout << "nepostojeci particles   " << nep->getGasConcentrationParticles() << endl;
					cout << "nepostojeci mass        " << nep->getGasConcentrationMass_DO_NOT_USE() << endl;
					cout << "nepostojeci temperatura " << nep->getTemperature() << endl;
					cout << "nepostojeci humidity    " << nep->getRelativeHumidity() << endl;
					cout << "------------------- toliko -------------------" << endl;

					usleep(1000 * 3000);
				}

				break;
			}

			//////////////////////////////////////////////////
			// Isto kao 5 ali output pogodan za import kao csv
			//////////////////////////////////////////////////
			case 51: {
				GasSensor *sa = new GasSensor(4, uartFileDescriptor);
				GasSensor *sb = new GasSensor(5, uartFileDescriptor);
				GasSensor *sc = new GasSensor(3, uartFileDescriptor);
				string sep = ";";	// separator izmedju elemenata
				cout << "------------- printout za data logger -------------" << endl;

				// explanation //
				cout << "timestamp" << sep;		// na pocetku svega timestamp
				cout << "s1 (units)" << sep << "s2 (units)" << sep << "s3 (units)" << sep;
				cout << "s1 temperature" << sep << "s2 temperature" << sep << "s3 temperature" << sep;
				cout << "s1 rel.humidity" << sep << "s2 rel.humidity" << sep << "s3 rel.humidity";		// na poslednjem elementu ne ide separator
				cout << endl;																				// vec novi red

				// actual data headers //
				cout << "ISO time" << sep;
				cout << sa->getSensorTypeStr() << " (" << sa->getUnitsForParticles() << ")" << sep;
				cout << sb->getSensorTypeStr() << " (" << sb->getUnitsForParticles() << ")" << sep;
				cout << sc->getSensorTypeStr() << " (" << sc->getUnitsForParticles() << ")" << sep;			
				cout << "s1 T(°C)" << sep << "s2 T(°C)" << sep << "s3 T(°C)" << sep;
				cout << "s1 Rh(%)" << sep << "s2 Rh(%)" << sep << "s3 Rh(%)";							// na poslednjem elementu ne ide separator
				cout << endl;																				// vec novi red

				for(;;) {

					// faking timestamp
					time_t now;
					time(&now);
					char buf[sizeof "2011-10-08T07:07:09Z"];
					strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
					cout << "\"" << buf << "\"" << sep;

					// measurements //
					cout << sa->getGasConcentrationParticles() << sep;
					cout << sb->getGasConcentrationParticles() << sep;
					cout << sc->getGasConcentrationParticles() << sep;
				
					cout << sa->getTemperature() << sep;
					cout << sb->getTemperature() << sep;
					cout << sc->getTemperature() << sep;
				
					cout << sa->getRelativeHumidity() << sep;
					cout << sb->getRelativeHumidity() << sep;
					cout << sc->getRelativeHumidity();			// na poslednjem elementu ne ide zarez
					cout << endl;								// vec novi red
					usleep(1000 * 5000);
				}

			}

			//////////////////////////
			// Prozivka u krug, u krug
			//////////////////////////
			case 6: {
				cout << "Prozivka!" << endl;
				cout << "---------" << endl;
				for ( ; ; ) {
					cout << endl;
					GasSensor *co = new GasSensor(adr_CO, uartFileDescriptor, 1);
					cout << "--------" << endl;
					GasSensor *h2s = new GasSensor(adr_H2S, uartFileDescriptor, 1);
					cout << "--------" << endl;
					GasSensor *o2 = new GasSensor(adr_O2, uartFileDescriptor, 1);
					cout << "--------" << endl;
					cout << endl;
				}
				break;
			}



			/////////////////////////////////////////
			// Testiram DEBUG_LEVEL + senzor ispravan
			/////////////////////////////////////////
			case 7: {
				for ( ; ; ) {
					cout << "----- DEBUG_LEVEL + senzor ispravan -----" << endl;
					cout << "------------ kreiram senzor -------------" << endl;
					cout << "DEBUG_LEVEL 0 => silent, ne sme prijaviti NISTA sto ga nisam pitao" << endl;
					GasSensor *gs7 = new GasSensor(adr_H2S, uartFileDescriptor, 0);
					cout << "saljem razne komande" << endl;
					gs7->setDebugLevel(0);
					gs7->getDebugLevel();
					gs7->getLedStatus();
					gs7->getSensorTypeStr();
					gs7->getGasConcentrationParticles();
					cout << "nadam se da je bila tisina" << endl;
					cout << "a sad mi se predstavi: ja sam " << gs7->getSensorTypeStr() << " na adresi " << gs7->getMuxAddress() << endl;
					cout << "zatim reci error code " << gs7->getErrorCode() << endl;
					cout << "i reci error count " << gs7->getErrorCount() << endl;

					cout << "--------" << endl;
					cout << "DEBUG_LEVEL 1 => minimal. Osim rezultata, moze biti i nekih gresaka na konzoli" << endl;
					gs7->setDebugLevel(1);
					cout << "debug level je " << gs7->getDebugLevel() << endl;
					cout << "get led status " << gs7->getLedStatus() << endl;
					cout << "get sensor typ str " << gs7->getSensorTypeStr() << endl;
					cout << "get gas particles " << gs7->getGasConcentrationParticles() << endl;
					cout << "error code " << gs7->getErrorCode() << endl;
					cout << "error count " << gs7->getErrorCount() << endl;

					cout << "--------" << endl;
					cout << "DEBUG_LEVEL 2 = quite verbose" << endl;
					gs7->setDebugLevel(2);
					cout << "debug level je " << gs7->getDebugLevel() << endl;
					cout << "get led status " << gs7->getLedStatus() << endl;
					cout << "get sensor typ str " << gs7->getSensorTypeStr() << endl;
					cout << "get gas particles " << gs7->getGasConcentrationParticles() << endl;
					cout << "error code " << gs7->getErrorCode() << endl;
					cout << "error count " << gs7->getErrorCount() << endl;
					cout << endl;
					cout << endl;
					cout << endl;

					usleep(1000 * 10000);
				}

				break;
			}




			///////////////////////////////////////////
			// Testiram DEBUG_LEVEL - senzor neispravan
			///////////////////////////////////////////
			case 8: {
				for ( ; ; ) {			
					//////////////////////////////////////
					//////////////////////////////////////
					// nepostojeci, pokvaren, iskljucen //
					//////////////////////////////////////
					//////////////////////////////////////
					cout << "----- DEBUG_LEVEL + senzor nije ispravan -----" << endl;
					cout << "------------ talk to nepostojeci -------------" << endl;
					cout << "mux will be " << nepostojeci->getMuxAddress() << endl;

					cout << "--------" << endl;
					cout << "DEBUG_LEVEL 0 => silent, ne sme prijaviti NISTA sto ga nisam pitao" << endl;
					cout << "saljem komande" << endl;
					nepostojeci->setDebugLevel(0);
					nepostojeci->getDebugLevel();
					nepostojeci->getLedStatus();
					nepostojeci->getSensorTypeStr();
					nepostojeci->getGasConcentrationParticles();
					cout << "nadam se da je bila tisina" << endl;
					cout << "a sad mi reci error code " << nepostojeci->getErrorCode() << endl;
					cout << "a sad mi reci error count " << nepostojeci->getErrorCount() << endl;

					cout << "--------" << endl;
					cout << "DEBUG_LEVEL 1 => minimal. Osim rezultata, moze biti i nekih gresaka na konzoli" << endl;
					nepostojeci->setDebugLevel(1);
					cout << "debug level je " << nepostojeci->getDebugLevel() << endl;
					cout << "get led status " << nepostojeci->getLedStatus() << endl;
					cout << "get sensor typ str " << nepostojeci->getSensorTypeStr() << endl;
					cout << "get gas particles " << nepostojeci->getGasConcentrationParticles() << endl;
					cout << "error code " << nepostojeci->getErrorCode() << endl;
					cout << "error count " << nepostojeci->getErrorCount() << endl;



					cout << "--------" << endl;
					cout << "DEBUG_LEVEL 2 = quite verbose" << endl;
					nepostojeci->setDebugLevel(2);
					cout << "debug level is" << nepostojeci->getDebugLevel() << endl;
					cout << "get led status " << nepostojeci->getLedStatus() << endl;
					cout << "get sensor typ str " << nepostojeci->getSensorTypeStr() << endl;
					cout << "get gas particles " << nepostojeci->getGasConcentrationParticles() << endl;
					cout << "error code " << nepostojeci->getErrorCode() << endl;
					cout << "error count " << nepostojeci->getErrorCount() << endl;
					cout << endl;
					cout << endl;
					cout << endl;

					usleep(1000 * 10000);
				}

				break;
			}


			//////////////////
			// ERROR reporting
			//////////////////
			case 9: {
				
				float merenje;
					unsigned int instance = adr_O2;
					cout << "kreiram GasSensor *xx = new GasSensor( " << instance << " )" << endl;
					GasSensor *xx = new GasSensor(instance, uartFileDescriptor, 0);

				for ( ; ; ) {
					cout << endl;
					cout << "-------------------------------------" << endl;
					cout << "Sensor errors" << endl;
					cout << "O2 je jedini povezan na uart test rig" << endl;
					cout << "-------------------------------------" << endl;
					// unsigned int instance = adr_O2;
					// cout << "kreiram GasSensor *xx = new GasSensor( " << instance << " )" << endl;
					// GasSensor *xx = new GasSensor(instance, uartFileDescriptor, 0);
					cout << "koliko je bilo gresaka: " << xx->getErrorCount() << endl;
					cout << "poslednji errorCode   : " << xx->getErrorCode() << endl;
					cout << "mux adr:	"		<< xx->getMuxAddress() 					<< "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;
					cout << "CRC checking:	" 	<< xx->getChecksumValidatorState()		<< "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;
					cout << "setting led off" << endl;
					xx->setLedOff();
					cout << "led stat:	"		<< xx->getLedStatus() 					<< "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;
					cout << "setting led on" << endl;
					xx->setLedOn();
					cout << "led stat:	"		<< xx->getLedStatus() 					<< "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;

					cout << "tip (hex):	" 	<< xx->getSensorTypeHex() 					<< "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;
					cout << "tip (str):	" 	<< xx->getSensorTypeStr() 					<< "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;
					cout << "max range:	" 	<< xx->getMaxRange() 						<< "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;
					cout << "decimala:	" 	<< xx->getDecimals() 						<< "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;
					cout << "particles:	" 	<< xx->getGasConcentrationParticles() 		<< "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;
					cout << "mass/m3:	" 	<< xx->getGasConcentrationMass_DO_NOT_USE() << "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;
					cout << "% of max:	" 	<< xx->getGasPercentageOfMaxParticles()		<< "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;
					cout << "temp °C:	" 	<< xx->getTemperature()						<< "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;
					cout << "humidity:	" 	<< xx->getRelativeHumidity()				<< "\t errorCount: " << xx->getErrorCount() << "\t errorCode: " << xx->getErrorCode() << endl;

					cout << endl;
					cout << "mala pauza..." << endl;
					usleep(1000 * 10000);
					cout << "...pa cemo ponovo" << endl;
					cout << endl;

				}
				break;
			}

		}

		for (;;) {
			// nop
		}

	}

	return 0;
}
