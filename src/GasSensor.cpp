/*
 * Driver for "GasSensor_TB200B_TB600BC_datasheet"
 *
 */

// C++
#include <bits/stdint-uintn.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <vector>

// hardware driver
#include "GasSensor.h"
#include "UartMux.h"

// rpi specific
// wiringpi specific
#include <wiringPi.h>
#include <wiringSerial.h>

// pomocnici
#include <chrono>
#include <bitset>

/*
 * DEBUGGING
 * the following are UBUNTU/LINUX, and MacOS ONLY terminal color codes.
 * https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
 */
#define CON_RESET   "\033[0m"
#define CON_BLACK   "\033[30m"      /* Black */
#define CON_RED     "\033[31m"      /* Red */
#define CON_GREEN   "\033[32m"      /* Green */
#define CON_YELLOW  "\033[33m"      /* Yellow */
#define CON_BLUE    "\033[34m"      /* Blue */
#define CON_MAGENTA "\033[35m"      /* Magenta */
#define CON_CYAN    "\033[36m"      /* Cyan */
#define CON_WHITE   "\033[37m"      /* White */

using namespace std;

/**
 * Create sensor instance, perform initialization
 */
GasSensor::GasSensor(MuxAdr_t muxAddress, int uartHandle, int default_lev) {
	this->DEBUG_LEVEL = default_lev;
	this->ERROR_CNT = 0;
	this->muxAddress = muxAddress;
	this->runningLed = true;
	this->uartHandle = uartHandle;

	this->H_STAT = OK;	// za pocetak je OK, bilo koja greska ga kvari
	init(SENSOR_INIT_mS);
}

GasSensor::~GasSensor() {
	serialClose(this->uartHandle);
}

/**
 * @brief Perform minimal initialization, report if comm error
 * @note Na izlasku iz ovoga, ako H_STAT i ERROR_CNT nisu nula onda nesto debelo ne valja
 */
void GasSensor::init(uint32_t waitSensorStartup_mS) {
	usleep(waitSensorStartup_mS * 1000);		// Malo vremena da se senzor stabilizuje nakon power-up

	if(DEBUG_LEVEL >=1){
		cout << "init: mux adresa " << getMuxAddress() << endl;
		cout << "init: debug_level = " << DEBUG_LEVEL << endl;
	}

	// Sto pre otkriti ako nema komunikacije sa senzorom
	if(DEBUG_LEVEL >=2){
		cout << "init: sensor connection check (get running lights)" << endl;
	}
	send(cmdRunningLightGetStatus);	// Nije bitan rezultat. Ako ne komunicira, dobice se timeout u send() metodu i postavice se H_STAT i ERROR_CNT

	if (DEBUG_LEVEL == -4){
		getSensorProperties_D1_INTERNALUSEONLY();
	}

	if(DEBUG_LEVEL >=2){
		cout << "init: get props" << endl;
	}
	getSensorProperties_D7();
	if(DEBUG_LEVEL >=1){
		int th = getSensorTypeHex();
		string cudno = (th==0) ? ", this is very likely an error!" : "";
		cout << "init: senzor tip 0x" << th << cudno << endl;
	}

	if(DEBUG_LEVEL >=2){
		cout << "init: set passive" << endl;
	}
	send(cmdSetPassiveMode);

	if(DEBUG_LEVEL >=2){
		cout << "init: set running light ON" << endl;
	}
	send(cmdRunningLightOn);

//	send(cmdSetActiveMode);
//	send(cmdRunningLightOn);

	if(DEBUG_LEVEL >=1){
		if(H_STAT == OK) {
			cout << "init ok: sensor muxAddress=" << this->muxAddress << ", type=" << getSensorTypeStr() << endl;
		} else {
			// ako nema komunikacije sa senzorom, jedino sigurno je muxAddress
			cout << "init fail: sensor muxAddress=" << this->muxAddress << ", H_STAT=" << H_STAT << ", error count=" << ERROR_CNT << endl;
		}
		cout << endl;
	}

}

void GasSensor::getSensorProperties_D1_INTERNALUSEONLY(){
	//
	// VAZNO!! Saljem COMMAND 3 = "D1". Odgovor je drugaciji nego za D7
	// Ovaj odgovor cak nema header!
	//
	std::vector<uint8_t> reply = send(cmdGetTypeRangeUnitDecimals0xd1);

	int expected = cmdGetTypeRangeUnitDecimals0xd1.expectedReplyLen;
	if (reply.size() < expected) {
		// H_STAT je vec podesen u metodu send()
		return;
	}

	if (this->DEBUG_LEVEL == -4){
		cout << "D1:";
		for (int i = 0; i < reply.size(); i++) {
			cout << "   0x" << setfill('0') << setw(2) << hex << static_cast<int>( reply.at(i) ) << "  " << std::dec;
		}
		cout << endl;
		cout << "D1:";
		for (int i = 0; i < reply.size(); i++) {
			std::cout << " " << std::bitset<8>(reply.at(i));
		}
		cout << endl;		
	}

	int d1_sensorTipHex = reply.at(0);
	int d1_maxRange = (reply.at(1) << 8) | reply.at(2);
	string d1_unit = (reply.at(3) == 0x02) ? "ppm" : "ppb";
	int d1_dec = ( reply.at(7) & 0b11110000 ) >> 4;	// decimal places:bit 7~4, zatim shift >>4 da dodje na LSB poziciju
	int d1_sgn = reply.at(7) & 0b00001111;			// sign bits 3~0
	(void)0;
}


/**
 * Query parameters from sensor with _D7_ command, and populate struct sensorProperties.
 * @return nothing.
 * @struct sensorProperties will be populated for later use
 */
void GasSensor::getSensorProperties_D7() {
	//
	// VAZNO!! Saljem COMMAND 4 = "D7". Odgovor je drugaciji nego za D1
	//
	std::vector<uint8_t> reply = send(cmdGetTypeRangeUnitDecimals0xd7);

	int expected = cmdGetTypeRangeUnitDecimals0xd7.expectedReplyLen;
	if (reply.size() < expected) {
		// H_STAT je vec podesen u metodu send()
		return;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0xD7);	// reply header ok?
	if (hdr) {
		if (this->DEBUG_LEVEL == -4){
			cout << "D7:";
			for (int i = 0; i < reply.size(); i++) {
				cout << "   0x" << setfill('0') << setw(2) << hex << static_cast<int>( reply.at(i) ) << "  " << std::dec;
			}
			cout << endl;
			cout << "D7:";
			for (int i = 0; i < reply.size(); i++) {
				std::cout << " " << std::bitset<8>(reply.at(i));
			}
			cout << endl;
			
		}
		
		this->H_STAT = OK;
		sensorProperties.tip = reply.at(2);
		sensorProperties.maxRange = (reply.at(3) << 8) | reply.at(4);
		switch (reply.at(5)) {
			case 0x02:
				strcpy(sensorProperties.unit_str, "mg/m3 and ppm");
				break;

			case 0x04:
				strcpy(sensorProperties.unit_str, "ug/m3 and ppb");
				break;

			case 0x08:
				strcpy(sensorProperties.unit_str, "10g/m3 and %");
				break;

			default:
				strcpy(sensorProperties.unit_str, " _?_ ");
				break;
		}
		uint8_t dec = reply.at(6) & 0b11110000;	// decimal placess: bit 7~4
		uint8_t sgn = reply.at(6) & 0b00001111;	// sign bits 3~0

		int dec_po_njihovom = ((dec & 0b10000000) << 3) | ((dec & 0b01000000) << 2) | ((dec & 0b00100000) << 1) | (dec & 0b00010000);
		int dec_shR = ((dec & 0b10000000) >> 3) | ((dec & 0b01000000) >> 2) | ((dec & 0b00100000) >> 1) | (dec & 0b00010000);
		dec = dec >> 4;							// spustim ih skroz na desno
		int dec_nj_shL = ((dec & 0b1000) << 3) | ((dec & 0b0100) << 2) | ((dec & 0b0010) << 1) | (dec & 0b0001);
		int dec_mshR = ((dec & 0b1000) >> 3) | ((dec & 0b0100) >> 2) | ((dec & 0b0010) >> 1) | (dec & 0b0001);
		
		//originalno
		uint8_t decimals = ((dec & 0b1000) << 3) | ((dec & 0b0100) << 2) | ((dec & 0b0010) << 1) | (dec & 0b0001);

		uint16_t sign = ((sgn & 0b1000) << 3) | ((sgn & 0b0100) << 2) | ((sgn & 0b0010) << 1) | (sgn & 0b0001);
		sensorProperties.decimals = decimals;		// dobije se 4. Jel' moguce da je tolika tacnost?
		sensorProperties.sign = sign;				// dobije se 0 = "negative inhibition". Koji li im djavo to znaci??
		(void)0;
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;		// silently report
		if(DEBUG_LEVEL > 0){
			cout << "sensor properties: wrong response header! STRONGLY suggest to discard any measurements.";
		}
	}
}

/**
 * @param active mode: sensor will send measurement data automatically at 1 second intervals
 */
void GasSensor::setActiveMode() {
	send(cmdSetActiveMode);
}

/**
 * @param passive mode = sensor will send measurement data only when requested
 */
void GasSensor::setPassiveMode() {
	send(cmdSetPassiveMode);
}


/**
 * @return Vraca HARDVERSKU adresu na uart multiplekseru (gde je senzor fizicki povezan).
 * @warning Adresa ce biti tacna SAMO u trenutku dok je OVAJ senzor aktivan. Razni procesi mogu pristupati mux-u i menjati ga nezavisno.
 */
int GasSensor::getMuxAddress(){
	return this->muxAddress;
}

/**
 * @return sensor type as HEX value
 */
int GasSensor::getSensorTypeHex(){
	return (int) sensorProperties.tip;
}

/**
 * @return string describing sensor type, such as "H2S", "CO"...
 */
std::string GasSensor::getSensorTypeStr(){
	std::string rez = "";
	// Raw property je HEX a funkcija castuje na INT.
	// NE KORISTI FUNKCIJU da se ne zajebes!
	switch (sensorProperties.tip) {
		case 0x19:
			rez = "CO";
			this->H_STAT = OK;
			break;

		case 0x1C:
			rez = "H2S";
			this->H_STAT = OK;
			break;

		case 0x22:
			rez = "O2";
			this->H_STAT = OK;
			break;


		default:
			/* 
			* ovo se desava ako je:
			* - senzor stvarno prijavio neki tip koji ovde nije definisan
			* - nastupila bilo kakva greska u komunikaciji sa senzorom
			* - senzor nije nista odgovorio i properties.tip je prazan ili nula ili null valjda
			*/
			// ovo se desava i ako je nastupila greska ili
			rez = "hmm...nepoznat senzor!?";
			this->H_STAT = UNEXPECTED_SENSOR_TYPE;
			break;
	};

	return rez;
}


/**
 * @return maximum measurement range from sensor properties
 */
int GasSensor::getMaxRange() {
	return (int) sensorProperties.maxRange;
}

/**
 * @return current gas concentration in ppm
 */
int GasSensor::getGasConcentrationPpm() {
	uint16_t rezultat = 0;
	vector<uint8_t> reply = send(cmdReadGasConcentration);

	int expected = cmdReadGasConcentration.expectedReplyLen;
	if (reply.size() < expected) {
		// H_STAT je vec podesen u metodu send()
		return MEASUREMENT_ERROR;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x86);		// reply header ok?
	if (hdr) {
		this->H_STAT = OK;
		uint16_t ppm = (reply.at(6) * 256) + (reply.at(7));
		uint16_t max = (reply.at(4) * 256) + (reply.at(5));
		rezultat = ppm;
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;
		rezultat = MEASUREMENT_ERROR;
	}
	if ( rezultat < 0 ) {
		// ne vrecaj MEASUREMENT_ERROR jer je rezultat svejedno negativan
		this->H_STAT = MEASUREMENT_OUT_OF_RANGE;
	}
	return (int) rezultat;
}

/**
 * @return gas concentration in mg/m3
 */
int GasSensor::getGasConcentrationMgM3() {
	uint16_t rezultat = 0;
	std::vector<uint8_t> reply = send(cmdReadGasConcentration);

	int expected = cmdReadGasConcentration.expectedReplyLen;
	if (reply.size() < expected) {
		// H_STAT je vec podesen u metodu send()
		return MEASUREMENT_ERROR;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x86);		// reply header ok?
	if (hdr) {
		this->H_STAT = OK;
		uint16_t mgm3 = (reply.at(2) * 256) + (reply.at(3));
		uint16_t max = (reply.at(4) * 256) + (reply.at(5));
		rezultat = mgm3;
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;
		rezultat = MEASUREMENT_ERROR;
	};
	if ( rezultat < 0 ) {
		// nema potrebe da vracam MEASUREMENT_ERROR jer je i ovako negativno
		this->H_STAT = MEASUREMENT_OUT_OF_RANGE;
	}
	return (int) rezultat;
}

/**
 * @return gas concentration normalized to 0 ~ 100% of max measurement range
 */
int GasSensor::getGasPercentageOfMax() {
	uint16_t rezultat = 0;
	vector<uint8_t> reply = send(cmdReadGasConcentration);

	int expected = cmdReadGasConcentration.expectedReplyLen;
	if (reply.size() < expected) {
		// H_STAT je vec podesen u metodu send()
		return MEASUREMENT_ERROR;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x86);		// reply header ok?
	if (hdr) {
		this->H_STAT = OK;
		uint16_t ppm = (reply.at(6) * 256) + (reply.at(7));
		uint16_t max = (reply.at(4) * 256) + (reply.at(5));
		rezultat = ((float) ppm / (float) max) * 100;
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;
		rezultat = MEASUREMENT_ERROR;
	}
	if ( (rezultat < 0) || (rezultat > 100) ) {
		rezultat = MEASUREMENT_ERROR;
		this->H_STAT = MEASUREMENT_OUT_OF_RANGE;
	}
	return (int) rezultat;
}



/**
 * @return temperature from combined reading (datasheet Command 6)
 */
int GasSensor::getTemperature() {
	uint16_t rezultat = 0;
	vector<uint8_t> reply = send(cmdReadGasConcentrationTempAndHumidity);
	
	int expected = cmdReadGasConcentrationTempAndHumidity.expectedReplyLen;
	if (reply.size() < expected) {
		// H_STAT je vec podesen u metodu send()
		return (float) MEASUREMENT_ERROR;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x87);		// reply header ok?
	if (hdr) {
		this->H_STAT = OK;
		float temp = ((reply.at(8) << 8) | (reply.at(9))) / 100;
		float humid = ((reply.at(10) << 8) | (reply.at(11))) / 100;
		rezultat = temp;
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;
		rezultat = MEASUREMENT_ERROR;
	}
	return (int) rezultat;
}

/**
 * @return relative humidity from combined reading (datasheet Command 6)
 */
int GasSensor::getRelativeHumidity() {
	uint16_t rezultat = 0;
	vector<uint8_t> reply = send(cmdReadGasConcentrationTempAndHumidity);

	int expected = cmdReadGasConcentrationTempAndHumidity.expectedReplyLen;
	if (reply.size() < expected) {
		// H_STAT je vec podesen u metodu send()
		return MEASUREMENT_ERROR;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x87);		// reply header ok?
	if (hdr) {
		this->H_STAT = OK;
		float temp = ((reply.at(8) << 8) | (reply.at(9))) / 100;
		float humid = ((reply.at(10) << 8) | (reply.at(11))) / 100;
		rezultat = humid;
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;
		rezultat = MEASUREMENT_ERROR;
	}
	if ( (rezultat < 0) || (rezultat > 100) ) {
		this->H_STAT = MEASUREMENT_OUT_OF_RANGE;
	}
	return (int) rezultat;
}

/**
 * Sensor activity led will blink during operation
 */
void GasSensor::setLedOn() {
	send(cmdRunningLightOn);
}

/**
 * @brief Sensor activity led will be off
 */
void GasSensor::setLedOff() {
	send(cmdRunningLightOff);
}

/**
 * @brief Get state of sensor activity led
 * @return blinking=true, off=false
 */
bool GasSensor::getLedStatus() {
	bool rezultat = false;
	vector<uint8_t> reply = send(cmdRunningLightGetStatus);

	int expected = cmdRunningLightGetStatus.expectedReplyLen;
	if (reply.size() < expected) {
		// H_STAT je vec podesen u metodu send()
		return false;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x8A);		// reply header ok?
	if (hdr) {
		this->H_STAT = OK;
		rezultat = (reply.at(2) == 1) ? true : false;
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;
		rezultat = false;
	}
	return rezultat;
}


/**
 * @brief
 * Send raw command to sensor and wait for zero or more bytes in reply.
 * If any reply is expected, checksum will be (optionally) checked.
 * In case of errors, H_STAT and ERROR_CNT will be set.
 * 
 * @warning
 * WARNING: Sensor measurement results are NOT CHECKED for consistency in this method!!
 * 
 * WARNING: Caller MUST verify on their own if results returned from here make any sense in the real world.
 * 
 * WARNING: This method ensures only:
 *   1) a reply is received within timeout 
 *   2) the checksum is ok (optional)
 * 
 * 
 * @return vector of raw bytes returned from sensor
 * @note will increment a global ERROR_CNT if required
 */
std::vector<uint8_t> GasSensor::send(const CmdStruct_t txStruct) {

	this->H_STAT = OK;		// ako nesto ne bude ok, ovo ce se pokvariti

	UartMux *mux = new UartMux();
	mux->setAddr(this->muxAddress);

	std::vector<uint8_t> reply;
	reply.clear();
	serialFlush(this->uartHandle);

	if(DEBUG_LEVEL >= 2) {
		cout << "send sajz " << txStruct.cmd.size() << ", will expect " << txStruct.expectedReplyLen << endl;
		if(DEBUG_LEVEL >= 3) {
			cout << "cmd to send:";
			for (unsigned int i = 0; i < txStruct.cmd.size(); ++i) {
				cout << " " << std::hex << static_cast<unsigned int>(txStruct.cmd[i]) << std::dec;
			}
			cout << std::dec << endl;
		}
	}
	for (unsigned int i = 0; i < txStruct.cmd.size(); i++) {
		serialPutchar(this->uartHandle, txStruct.cmd[i]);
		// At 9600 baud, the 1 bit time is ~104 microseconds, and 1 full character is 1.04mS
		usleep(1000);	// some guard time between characters. No reason, just because.
	};


	// Ako ocekujem neki odgovor od senzora
	if (txStruct.expectedReplyLen > 0) {
		unsigned int mS = 0;

		/*
		 * Na pocetku je buffer prazan tj serialDataAvail() == 0
		 * pa se polako puni karakterima, kako pristizu od senzora
		 * Zato cekam da
		 * - available postane jednako expected
		 * - ili istekne timeout (senzor je neispravan ili fizicki iskljucen)
		 */
		while (serialDataAvail(this->uartHandle) < (int) txStruct.expectedReplyLen) {
			usleep(1000);
			mS++;
			if (mS >= SENSOR_TIMEOUT_mS) {
				// isteklo vreme!
				// Samo break iz petlje, neka metod nastavi dalje, ako je potreban neki cleanup
				this->H_STAT = SENSOR_TIMEOUT;
				if(DEBUG_LEVEL >=1){
					cout << "ERROR!! Timeout in sensor comunication after " << mS << "mS" << endl;
				}
				break;
			}
		}
		// ovde dodjemo kad je istekao timeout ili je stiglo dovoljno karaktera


		if (mS < SENSOR_TIMEOUT_mS) {
			// ovde smo ako nije nastupio timeout i stiglo je dovoljno bajtova
			while ( serialDataAvail(this->uartHandle) > 0 ) {
				uint8_t x = serialGetchar(this->uartHandle);
				reply.push_back(x);
			};
			// stigao je odgovor, kakav-takav

			// podesimo error statuse
			bool sajzOk = (reply.size() == txStruct.expectedReplyLen);		// ne diraj duplo jednako!
			if ( ! sajzOk ) { this->H_STAT = MEASUREMENT_INCOMPLETE; };		// stiao je pogresan broj bajtova


			// pa sad slicno ali za console debugging
			if (DEBUG_LEVEL >= 2) {
				cout << "reply received after " << mS << "mS" << endl;
				cout << "rcv: ";
				if (reply.size() == 0) {
					cout << CON_RED << "NONE!";		// nije stiglo nista! prazan vector zasluzuje crvena slova!
				} else if ( ! sajzOk ) {
					cout << CON_YELLOW; 			// ako je stigla pogresna kolicina, onda zuta slova
				};
				for (unsigned int i = 0; i < reply.size(); ++i) {
					cout << " " << std::hex << static_cast<unsigned int>(reply.at(i)) << std::dec;	// i to ovoliko bajtova
				}
				cout << CON_RESET << endl;			// ispis na normalu
				cout << endl;
			}

			// neke komande vracaju odgovor i checksum, a neke samo odgovor bez checksuma, a neke nista
			if (txStruct.checksumPresent == true) {
				bool chk = isChecksumValid(reply);
				if ((this->checksumValidatorIsActive == true) && (chk == false)) {
					this->H_STAT = MEASUREMENT_CHECKSUM_FAIL;
					if(DEBUG_LEVEL > 0) {
						cout << "reply checksum is " << CON_RED << "NOT OK!" << CON_RESET << endl;
					}
				}
			}

		}

	}

	if (H_STAT != OK){
		ERROR_CNT++;
	}

	serialFlush(this->uartHandle);
	return reply;
}



//////////////////////////
//////////////////////////
//// 	UPRAVLJANJE 	//
//////////////////////////
//////////////////////////


/**
 * @return najskoriji error kod
 */
GasSensor::ErrCodes_t GasSensor::getErrorCode(){
	return H_STAT;
}

/**
 * @return how many errors ocurred since last power on
 * @note can be reset only by power-cycling the device, or creating a new instance of the class
 */
unsigned int GasSensor::getErrorCount(){
	return this->ERROR_CNT;
}


/**
 * @brief set debug verbosity from now on
 */
void GasSensor::setDebugLevel(int level){
	this->DEBUG_LEVEL = level;
}

/**
 * @brief get current debug verbosity
 */
int GasSensor::getDebugLevel(){
	return this->DEBUG_LEVEL;
}



/**
 * @brief Checksum validation will be performed or skipped
 */
void GasSensor::setChecksumValidatorState(bool state) {
	this->checksumValidatorIsActive = state;
}

/**
 * @brief Get current state of checksum validation
 * @return true = checksum is validated, false = checksum is discarded
 */
bool GasSensor::getChecksumValidatorState(){
	return this->checksumValidatorIsActive;
}

/**
 * @brief Get sensor precision
 * @return number of decimal places as reported by sensor
 */
int GasSensor::getDecimals() {
	return sensorProperties.decimals;
}

/**
 * @brief Calculate checksum and compare with expected
 * @return true = sensor data is valid against reported checksum
 */
bool GasSensor::isChecksumValid(std::vector<uint8_t> repl) {
	/*
	* NOTA:
	* Datasheet kaze da se checksum racuna:
	* - zbir bajtova od JEDINICE zakljucno sa PRETPOSLEDNJIM
	* - ne sabiraju se nulti i poslednji bajt. Poslednji je checksum
	* - od zbira se uradi bitwise not
	* - doda se jedan
	*/
	uint8_t sum = 0;
	const int JEDAN = 1;	// nemoj u brzini da ovo ispravis na nulu!!
	for (unsigned int i = JEDAN; i < (repl.size()-JEDAN); ++i) {
		sum += repl.at(i);
	}
	sum = ~sum;	// bitwise not
	sum = sum + 1;
	if (sum == repl.at(repl.size() - 1)) {
		return true;
	} else {
		this->H_STAT = MEASUREMENT_CHECKSUM_FAIL;
		return false;
	};
}

//////////////////
//////////////////
// pomocnici
//////////////////
//////////////////


void GasSensor::sendRawBytes(const char *rawBytes, unsigned int size) {
	vector<uint8_t> s(rawBytes, rawBytes + size);
	CmdStruct_t cmd = { s, size };
	send(cmd);
}


