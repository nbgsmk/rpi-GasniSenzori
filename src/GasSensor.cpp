/*
 * Driver for "GasSensor_TB200B_TB600BC_datasheet"
 *
 */

// C++
#include <bits/stdint-uintn.h>
#include <algorithm>
#include <array>
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
	usleep(waitSensorStartup_mS * 1000);

	cout << "set passive" << endl;
	send(cmdSetPassiveMode);
	cout << "set running light" << endl;
	send(cmdRunningLightOn);
	cout << "get props" << endl;
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

	bool hdr = (reply.at(0)==0xFF)  && (reply.at(1)==0xD7);	// reply header ok?
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
		uint8_t decimals =	((dec & 0b1000)<<3) |
							((dec & 0b0100)<<2) |
							((dec & 0b0010)<<1) |
							 (dec & 0b0001);

		uint16_t sign =		((sgn & 0b1000)<<3) |
							((sgn & 0b0100)<<2) |
							((sgn & 0b0010)<<1) |
							 (sgn & 0b0001);
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
 * @return maximum measurement range from sensor properties
 */
int GasSensor::getMaxRange() {
	return (int)sensorProperties.maxRange;
}

/**
 * @return current gas concentration in ppm
 */
int GasSensor::getGasConcentrationPpm() {
	uint16_t rezultat = 0;
	vector<uint8_t> reply = send(cmdReadGasConcentration);
	bool hdr = (reply.at(0)==0xFF)  && (reply.at(1)==0x86);		// reply header ok?
	if (hdr) {
		uint16_t ppm = (reply.at(6) * 256) + (reply.at(7));
		uint16_t max = (reply.at(4) * 256) + (reply.at(5));
		rezultat = ppm;
	} else {
		rezultat = SENSOR_DATA_ERROR;
	}
	return (int)rezultat;
}


/**
 * @return current gas concentration in ppm
 */
int GasSensor::getGasConcentrationMgM3() {
	uint16_t rezultat = 0;
	std::vector<uint8_t> reply = send(cmdReadGasConcentration);

	bool hdr = (reply.at(0)==0xFF)  && (reply.at(1)==0x86);		// reply header ok?
	if (hdr) {
		uint16_t mgm3 = (reply.at(2) * 256) + (reply.at(3));
		uint16_t max = (reply.at(4) * 256) + (reply.at(5));
		rezultat = mgm3;
	} else {
		rezultat = SENSOR_DATA_ERROR;
	}
	return (int)rezultat;
}



/**
 * @return gas concentration normalized to 0~100% of max measurement range
 */
int GasSensor::getGasPercentageOfMax() {
	uint16_t rezultat = 0;
	vector<uint8_t> reply = send(cmdReadGasConcentration);

	bool hdr = (reply.at(0)==0xFF)  && (reply.at(1)==0x86);		// reply header ok?
	if (hdr) {
		uint16_t ppm = (reply.at(6) * 256) + (reply.at(7));
		uint16_t max = (reply.at(4) * 256) + (reply.at(5));
		rezultat = ((float)ppm / (float)max) * 100;
	} else {
		rezultat = SENSOR_DATA_ERROR;
	}
	return (int)rezultat;
}


/**
 * @return temperature from combined reading (datasheet Command 6)
 */
float GasSensor::getTemperature() {
	uint16_t rezultat = 0;
	vector<uint8_t> reply = send(cmdReadGasConcentrationTempAndHumidity);

	bool hdr = (reply.at(0)==0xFF)  && (reply.at(1)==0x87);		// reply header ok?
	if (hdr) {
		float temp = ( (reply.at(8) << 8) | (reply.at(9)) ) / 100;
		float humid = ( (reply.at(10) << 8) | (reply.at(11)) ) / 100;
		rezultat = temp;
	} else {
		rezultat = SENSOR_DATA_ERROR;
	}
	return (int)rezultat;
}


/**
 * @return relative humidity from combined reading (datasheet Command 6)
 */
float GasSensor::getRelativeHumidity() {
	uint16_t rezultat = 0;
	vector<uint8_t> reply = send(cmdReadGasConcentrationTempAndHumidity);

	bool hdr = (reply.at(0)==0xFF)  && (reply.at(1)==0x87);		// reply header ok?
	if (hdr) {
		float temp = ( (reply.at(8) << 8) | (reply.at(9)) ) / 100;
		float humid = ( (reply.at(10) << 8) | (reply.at(11)) ) / 100;
		rezultat = humid;
	} else {
		rezultat = SENSOR_DATA_ERROR;
	}
	return (int)rezultat;
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
	bool hdr = (reply.at(0)==0xFF)  && (reply.at(1)==0x8A);		// reply header ok?
	if (hdr) {
		rezultat = (reply.at(2) == 1) ? true : false;
	}
	return rezultat;
}



//////////////////
// pomocnici
//////////////////



/**
 * send raw command to sensor and wait for reply
 */
std::vector<uint8_t> GasSensor::send(const CmdStruct_t txStruct) {

//	UartMux *mux = new UartMux();
//	mux->setAddr(this->uartHandle);


    // send command to sensor and immediately wait to receive tx.expectedReplyLen bytes
    std::vector<uint8_t> reply;
    reply.clear();

//	cout << "send sajz " << txStruct.cmd.size() << ", expect " << txStruct.expectedReplyLen << endl;
    for (unsigned int i = 0; i < txStruct.cmd.size(); i++) {
    	//cout << "send loop " << i << ", " << std::hex << static_cast<unsigned int>(txStruct.cmd[i]) << endl;
    	serialPutchar(this->uartHandle, txStruct.cmd[i]);
    	// At 9600 baud, the bit time is about 104 microseconds which makes each character sent take 1.04 milliseconds
    	usleep(500);	// plenty of time between characters
	};

	if (txStruct.expectedReplyLen > 0) {
//		cout << "cekam " << txStruct.expectedReplyLen << endl;
		unsigned int mS = 300;
//		cout << "usleep " << mS << "mS" << endl;
		usleep(mS * 1000);
		int avail = serialDataAvail(this->uartHandle);
//		cout << "serial avail " << avail << endl;
		while(serialDataAvail(this->uartHandle)){
			int x = serialGetchar(this->uartHandle);
//			cout << "rcv " << std::hex << static_cast<unsigned int>(x) << endl;
			reply.push_back(x);
		};

		} else {
//		    uint8_t jbg[] = "\n no reply \n";
//		    HAL_UART_Transmit(hUrtDbg, jbg, sizeof(jbg), 500);
		}
	serialFlush(this->uartHandle);
	return reply;
}



void GasSensor::sendRawCommand(const char *rawBytes, unsigned int size){
	vector<uint8_t> s(rawBytes, rawBytes + size);
	CmdStruct_t cmd = { s, size };
	send(cmd);
}


bool GasSensor::isReplyChecksumValid(std::vector<uint8_t> repl) {
	uint8_t sum = 0;
	for (unsigned int i = 1; i < (repl.size()-1); ++i) {
		// NOTA:
		// -- Petlja pocinje od JEDINICE a ne od nule. Ne sabira se nulti element (obicno je 0xFF)
		// -- NE SABIRA SE POSLEDNJI element jer on je checksum
		// Vidi objasnjenje u datasheet-u
		sum += repl.at(i);
	}
	sum = ~sum;	// bitwise not
	sum = sum + 1;
	if( sum == repl.at(repl.size()-1) ){
		return true;
	} else {
		return false;
	};

}
