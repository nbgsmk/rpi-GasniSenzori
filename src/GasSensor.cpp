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
 * Constructor to create sensor and perform minimal initialization
 */
GasSensor::GasSensor(MuxAdr_t muxAddress, int uartHandle) {
	this->muxAddress = muxAddress;
	this->runningLed = true;
	this->uartHandle = uartHandle;

}

GasSensor::~GasSensor() {
	serialClose(this->uartHandle);
}

/**
 * Perform minimal initialization
 */
void GasSensor::init(uint32_t waitSensorStartup_mS) {
	#define DEBUG 0
	usleep(waitSensorStartup_mS * 1000);

	// Najbezbolniji nacin da sto ranije otkrijem ako ne komuniciram sa senzorom
	// Nije potreban nikakav rezultat. Ako ne komunicira, dobice se timeout
	send(cmdRunningLightGetStatus);

	#if DEBUG >= 1
		cout << "set passive" << endl;
	#endif
	send(cmdSetPassiveMode);

	#if DEBUG >= 1
		cout << "set running light ON" << endl;
	#endif
	send(cmdRunningLightOn);

	#if DEBUG >= 1
		cout << "get props" << endl;
	#endif
	getSensorProperties_D7();

//	send(cmdSetActiveMode);
//	send(cmdRunningLightOn);
}

/**
 * Query parameters from sensor and populate struct
 */
void GasSensor::getSensorProperties_D7() {
	//
	// VAZNO!! Saljem COMMAND 4 = "D7". Odgovor je drugaciji nego za D1
	//
	std::vector<uint8_t> reply = send(cmdGetTypeRangeUnitDecimals0xd7);

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0xD7);	// reply header ok?
	uint8_t tip = reply.at(2);
	if (hdr && tip) {
		// sensor type ok
		sensorProperties.tip = tip;
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
		uint8_t dec = reply.at(6) & 0b11110000;	// decimals bit 7~4, sign bits 3~0
		uint8_t sgn = reply.at(6) & 0b00001111;	// decimals bit 7~4, sign bits 3~0

		dec = dec >> 4;							// spustim ih skroz na desno
		uint8_t decimals = ((dec & 0b1000) << 3) | ((dec & 0b0100) << 2) | ((dec & 0b0010) << 1) | (dec & 0b0001);

		uint16_t sign = ((sgn & 0b1000) << 3) | ((sgn & 0b0100) << 2) | ((sgn & 0b0010) << 1) | (sgn & 0b0001);
		sensorProperties.decimals = decimals;		// dobije se 4. Jel' moguce da je tolika tacnost?
		sensorProperties.sign = sign;				// dobije se 0 = "negative inhibition". Koji li im djavo to znaci??

	} else {
		throw std::invalid_argument("Wrong sensor type! Expected 0x19, got ??");		// FIXME dodati ovde hex parametar tip, izbaciti printf
	}

}

/**
 * Set active mode: sensor will send measurement data automatically in 1 second intervals
 */
void GasSensor::setActiveMode() {
	send(cmdSetActiveMode);
}

/**
 * Set passive mode: sensor will send measurement data only when requested
 */
void GasSensor::setPassiveMode() {
	send(cmdSetPassiveMode);
}


/**
 * Tip senzora HEX vrednost. Vidi datasheet
 */
int GasSensor::getSensorTypeHex(){
	return (int) sensorProperties.tip;
}


std::string GasSensor::getSensorTypeStr(){
	std:string rez = "hmm...neki nepoznat senzor!?";
	// Raw property je HEX a funkcija castuje na INT.
	// NE KORISTI FUNKCIJU da se ne zajebes!
	switch (sensorProperties.tip) {
		case 0x19:
			rez = "CO";
			break;

		case 0x1C:
			rez = "H2S";
			break;

		case 0x22:
			rez = "O2";
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
	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x86);		// reply header ok?
	if (hdr) {
		uint16_t ppm = (reply.at(6) * 256) + (reply.at(7));
		uint16_t max = (reply.at(4) * 256) + (reply.at(5));
		rezultat = ppm;
	} else {
		rezultat = SENSOR_DATA_ERROR;
	}
	return (int) rezultat;
}

/**
 * @return current gas concentration in ppm
 */
int GasSensor::getGasConcentrationMgM3() {
	uint16_t rezultat = 0;
	std::vector<uint8_t> reply = send(cmdReadGasConcentration);
	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x86);		// reply header ok?
	if (hdr) {
		uint16_t mgm3 = (reply.at(2) * 256) + (reply.at(3));
		uint16_t max = (reply.at(4) * 256) + (reply.at(5));
		rezultat = mgm3;
	} else {
		rezultat = SENSOR_DATA_ERROR;
	}
	return (int) rezultat;
}

/**
 * @return gas concentration normalized to 0~100% of max measurement range
 */
int GasSensor::getGasPercentageOfMax() {
	uint16_t rezultat = 0;
	vector<uint8_t> reply = send(cmdReadGasConcentration);
	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x86);		// reply header ok?
	if (hdr) {
		uint16_t ppm = (reply.at(6) * 256) + (reply.at(7));
		uint16_t max = (reply.at(4) * 256) + (reply.at(5));
		rezultat = ((float) ppm / (float) max) * 100;
	} else {
		rezultat = SENSOR_DATA_ERROR;
	}
	return (int) rezultat;
}

/**
 * @return temperature from combined reading (datasheet Command 6)
 */
float GasSensor::getTemperature() {
	uint16_t rezultat = 0;
	vector<uint8_t> reply = send(cmdReadGasConcentrationTempAndHumidity);
	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x87);		// reply header ok?
	if (hdr) {
		float temp = ((reply.at(8) << 8) | (reply.at(9))) / 100;
		float humid = ((reply.at(10) << 8) | (reply.at(11))) / 100;
		rezultat = temp;
	} else {
		rezultat = SENSOR_DATA_ERROR;
	}
	return (int) rezultat;
}

/**
 * @return relative humidity from combined reading (datasheet Command 6)
 */
float GasSensor::getRelativeHumidity() {
	uint16_t rezultat = 0;
	vector<uint8_t> reply = send(cmdReadGasConcentrationTempAndHumidity);
	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x87);		// reply header ok?
	if (hdr) {
		float temp = ((reply.at(8) << 8) | (reply.at(9))) / 100;
		float humid = ((reply.at(10) << 8) | (reply.at(11))) / 100;
		rezultat = humid;
	} else {
		rezultat = SENSOR_DATA_ERROR;
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
 * Sensor activity led will be off
 */
void GasSensor::setLedOff() {
	send(cmdRunningLightOff);
}

/**
 * Sensor activity led will be off
 */
bool GasSensor::getLedStatus() {
	bool rezultat = false;
	vector<uint8_t> reply = send(cmdRunningLightGetStatus);
	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x8A);		// reply header ok?
	if (hdr) {
		rezultat = (reply.at(2) == 1) ? true : false;
	}
	return rezultat;
}


/**
 * send raw command to sensor and wait for reply of specific length
 */
std::vector<uint8_t> GasSensor::send(const CmdStruct_t txStruct) {
	#define DEBUG 0

	UartMux *mux = new UartMux();
	mux->setAddr(this->muxAddress);

	std::vector<uint8_t> reply;
	reply.clear();
	serialFlush(this->uartHandle);

	#if DEBUG>=1
		cout << "send sajz " << txStruct.cmd.size() << ", will expect " << txStruct.expectedReplyLen << endl;
		#if DEBUG>=2
			cout << "cmd to send:";
			for (unsigned int i = 0; i < txStruct.cmd.size(); ++i) {
					cout << " " << std::hex << static_cast<unsigned int>(txStruct.cmd[i]) << std::dec;
			}
			cout << std::dec << endl;
		#endif
	#endif
	for (unsigned int i = 0; i < txStruct.cmd.size(); i++) {
		serialPutchar(this->uartHandle, txStruct.cmd[i]);
		// At 9600 baud, the 1 bit time is ~104 microseconds, amd 1 full character is 1.04mS
		usleep(1000);	// some guard time between characters. No reason, just because.
	};


	// Ako ocekujem bilo kakav odgovor od senzora
	if (txStruct.expectedReplyLen > 0) {
		unsigned int mS = 0;
		unsigned int timeOut_mS = 2000;		// TB600-CO-100 prosecan odgovor je oko 40..max 45mS. Ako za DVE SEKUNDE ne stigne nista, onda jbga!

		// dok ne stignu SVI ocekivani karakteri
		while (serialDataAvail(this->uartHandle) < (int) txStruct.expectedReplyLen) {
			usleep(1000);
			mS++;
			if (mS >= timeOut_mS) {
				// Ne volim sa pola metoda da uradim return. Mozda je potreban neki cleanup
				break;
			}
		}

		if (mS < timeOut_mS) {
			// ako su stigli bajtovi i nije nastupio timeout
			while ( serialDataAvail(this->uartHandle) > 0 ) {
				uint8_t x = serialGetchar(this->uartHandle);
				reply.push_back(x);
			};
			#if DEBUG>=2
				cout << "after sleeping " << mS << "mS" << endl;
				cout << "rcv: ";
				if (reply.size() == 0) { cout << CON_RED << "NONE!"; };
				if (reply.size() != txStruct.expectedReplyLen) { cout << CON_YELLOW; };
				for (unsigned int i = 0; i < reply.size(); ++i) {
					cout << " " << std::hex << static_cast<unsigned int>(reply.at(i)) << std::dec;
				}
				cout << CON_RESET << endl;
				cout << "reply checksum is " << (isChecksumValid(reply) ? "ok" : "NOT OK!") << endl;
				cout << endl;
			#endif
			if (txStruct.checksumPresent == true) {
				bool valid = isChecksumValid(reply);
				if ((this->checksumValidation == true) & (valid == false)) {
					// TODO nesto uraditi!
					cout << "reply checksum is " << CON_RED << "NOT OK!" << CON_RESET << endl;
				}
			} else {

			}

		} else {
			cout << "ERROR!! Timeout in sensor comunication after " << mS << "mS" << endl;
		}
	}

	serialFlush(this->uartHandle);
	return reply;
}

void GasSensor::setChecksumValidation(bool on_off) {
	this->checksumValidation = on_off;
}

int GasSensor::getDecimals() {
	return sensorProperties.decimals;
}

bool GasSensor::isChecksumValid(std::vector<uint8_t> repl) {
	uint8_t sum = 0;
	for (unsigned int i = 1; i < (repl.size() - 1); ++i) {
		// NOTA:
		// -- Datasheet kaze: checksum se racuna od JEDINICE zakljucno sa PRETPOSLEDNJIM bajtom
		// -- Ne sabira se nulti i poslednji. Poslednji je checksum.
		sum += repl.at(i);
	}
	sum = ~sum;	// bitwise not
	sum = sum + 1;
	if (sum == repl.at(repl.size() - 1)) {
		return true;
	} else {
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


