/*
 * Driver for "TB200B_TB600BC_datasheet" sensor
 *
 */

// zajednicki
#include "GasSensor.h"

#include "app.h"

// hardware driver
#include <bits/stdint-uintn.h>
#include <algorithm>
#include <cstdio>

using namespace std;

/**
 * Constructor to create sensor and perform minimal initialization
 */
GasSensor::GasSensor(int muxAddress) {
	this->muxAddress = muxAddress;

}

GasSensor::~GasSensor() {

}


// stm32 specific
void GasSensor::setSensorUart(int uart) {
//	hUrt = sensorUart; // dummy todo



}

// stm32 specific
void GasSensor::setDebugUart(int uart) {
//	hUrt = sensorUart; // dummy todo

}


/**
 * Perform minimal initialization
 */
void GasSensor::init(uint32_t waitSensorStartup_mS) {
	HAL_Delay(waitSensorStartup_mS);
	send(cmdSetPassiveMode);
	send(cmdRunningLightOff);
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
	vector<uint8_t> reply = send(cmdGetTypeRangeUnitDecimals0xd7);

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
		// stm32 specific
		// ENABLE EXCEPTIONS for stm32
		// Right click the project -> Properties -> C/C++ Build -> Settings -> Tool Settings -> MCU G++ Compiler -> Miscellaneous
		// add flag -fexceptions
		printf("Wrong sensor type! Expected 0x19, got |%04x|", tip);
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
	vector<uint8_t> reply = send(cmdReadGasConcentration);

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
	// TODO mogao bi da vraca enum on, off, error
	bool rezultat;
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
	uint8_t txArr[txStruct.cmd.size()];
	std::copy(txStruct.cmd.begin(), txStruct.cmd.end(), txArr);

	// stm32 specific
	// send bytes to debug port
    uint8_t c[] = {"\n cmd="};
    HAL_UART_Transmit(1, c, sizeof(c), 500);
    HAL_UART_Transmit_DBG(2, c, sizeof(c), 500);


    // send command to sensor and immediately wait to receive tx.expectedReplyLen bytes
    std::vector<uint8_t> reply;
    HAL_UART_Transmit(hUrt, c, sizeof(c), 500);
	if (txStruct.expectedReplyLen > 0) {
		if(HAL_UART_Receive(1, rxB, 100, 500)==HAL_OK){
			for (int i = 0; i < txStruct.expectedReplyLen; ++i) {
				reply.push_back(rxB[i]);
			}


		    uint8_t r[] = " reply=";
		    HAL_UART_Transmit(hUrtDbg, r, sizeof(r), 500);
		    HAL_UART_Transmit(hUrtDbg, rxB, txStruct.expectedReplyLen, 500);
		    uint8_t c[] = ";\n";
		    HAL_UART_Transmit(hUrtDbg, c, sizeof(c), 500);
		} else {
		    uint8_t jbg[] = "\n no reply \n";
		    HAL_UART_Transmit(hUrtDbg, jbg, sizeof(jbg), 500);
		}
	}
	return reply;
}



void GasSensor::sendRawCommand(const uint8_t *plainTxt, uint16_t size){
	std::vector<uint8_t> s(plainTxt, plainTxt + size);
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
