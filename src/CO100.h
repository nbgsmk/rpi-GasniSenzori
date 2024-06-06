/*
 * CO100.h
 *
 */

#ifndef SRC_CO100_H_
#define SRC_CO100_H_

#include "app.h"
#include <array>
#include <cstring>
#include <stdexcept>

#include "Sensor_TB600C_CO_100ppm_datasheet.h"


#define MUX_ADR 0b001			// CONFIG mux adresa ovog senzora

class CO100 {

public:
	CO100();
	virtual ~CO100();

	uint8_t rxB[50];							// bytes received from UART

	void init(uint32_t waitSensorStartup_mS);	// inicijalizuj senzor, podesi passive mode, proveri tip
	void setSensorUart();
	void setDebugUart();
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

#endif /* SRC_CO100_H_ */
