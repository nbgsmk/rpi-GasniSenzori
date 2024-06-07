/*
 * GasSensor.h
 *
 */

#ifndef SRC_GASSENSOR_H_
#define SRC_GASSENSOR_H_

#include <array>
#include <cstring>
#include <stdexcept>
#include "UartMux.h"
#include <stdint.h>
#include <vector>

#include "GasSensor_TB200B_TB600BC_datasheet.h"

#define MUX_ADR 0b001			// CONFIG mux adresa ovog senzora

class GasSensor {

public:
	GasSensor(MuxAdr_t muxAddress);
	virtual ~GasSensor();

	uint8_t rxB[50];							// bytes received from UART

	void init(uint32_t waitSensorStartup_mS);	// inicijalizuj senzor, podesi passive mode, proveri tip
//	void setSensorUart(int uart);
//	void setDebugUart(int uart);
	void setActiveMode();
	void setPassiveMode();
	void setLedOn();
	void setLedOff();
	bool getLedStatus();

	int getMaxRange();							// maksimalni raspon merenja senzora
	int getGasConcentrationPpm();				// koncentracija gasa ppm
	int getGasConcentrationMgM3();				// koncentracija gasa ug/m3
	int getGasPercentageOfMax();				// koncentracija 0~100% od maksimalnog merenja senzora
	float getTemperature();						// sve zajedno merimo
	float getRelativeHumidity();				// sve zajedno merimo

	// stm32 specific and debug only
//	void setSensorUart(UART_HandleTypeDef huart);
//	void setDebugUart(UART_HandleTypeDef huart);
	void sendRawCommand(const uint8_t *plainTxt, uint16_t size);


private:
	int muxAddress;
	bool runningLed;
	struct {
		uint8_t tip;
		uint16_t maxRange;
		char unit_str[15];
		uint8_t decimals;
		uint8_t sign;
	} sensorProperties;

	void getSensorProperties_D7();					// popuni struct sa podacima o senzoru
	std::vector<uint8_t> send(const CmdStruct_t txCmd);				// posalji komande senzoru, cekaj odgovor
	bool isReplyChecksumValid(std::vector<uint8_t> repl);


};

#endif /* SRC_GASSENSOR_H_ */
