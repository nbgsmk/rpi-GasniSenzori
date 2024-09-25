/*
 * GasSensor.h
 *
 */

#ifndef SRC_GASSENSOR_H_
#define SRC_GASSENSOR_H_

#define MEASUREMENT_ERROR (int)-1


#include "UartMux.h"
#include "GasSensor_TB200B_TB600BC_datasheet.h"


class GasSensor {
public:
	GasSensor(MuxAdr_t muxAddress, int uartHandle, int default_level = 0);
	virtual ~GasSensor();

	enum ErrCodes_t {
		OK = 0,
		NOT_DEFINED,
		INIT_FAIL,
		UNEXPECTED_SENSOR_TYPE,
		WRONG_RESPONSE_HEADER,
		SENSOR_TIMEOUT,
		MEASUREMENT_INCOMPLETE,
		MEASUREMENT_CHECKSUM_FAIL,
		MEASUREMENT_OUT_OF_RANGE,
	};
	ErrCodes_t getErrorCode();
	unsigned int getErrorCount();
	void setDebugLevel(int level);
	int getDebugLevel();

	void setActiveMode();
	void setPassiveMode();
	void setLedOn();
	void setLedOff();
	bool getLedStatus();
	int getMuxAddress();						// adresa na uart multiplekseru
	void setChecksumValidatorState(bool state);	// proverava se checksum rezultata ili se ignorise
	bool getChecksumValidatorState();

	int getSensorTypeHex();						// HEX vrednost - tip senzora
	std::string getSensorTypeStr();				// na osnovu HEX vracam tekstualni naziv
	int getMaxRange();							// maksimalni raspon merenja senzora
	int getDecimals();							// broj decimala u rezultatu
	int getGasConcentrationPpm();				// koncentracija gasa ppm
	int getGasConcentrationMgM3();				// koncentracija gasa ug/m3
	int getGasPercentageOfMax();				// koncentracija 0~100% od maksimalnog merenja senzora
	int getTemperature();						// sve zajedno merimo
	int getRelativeHumidity();				// sve zajedno merimo

	// debug only
	void sendRawBytes(const char *rawBytes, unsigned int size);

	// debug only. kasnije moze da se vrati da bude privatna - TODO
	std::vector<uint8_t> send(const CmdStruct_t txCmd);		// posalji komande senzoru, cekaj odgovor


private:
	// Error handling je neophodan
	int DEBUG_LEVEL =0;
	unsigned int ERROR_CNT = 0;
	ErrCodes_t H_STAT = NOT_DEFINED;

	MuxAdr_t muxAddress;
	bool runningLed;
	int uartHandle;
	bool checksumValidatorIsActive = true;

	const unsigned int SENSOR_INIT_mS = 1000;		// minimalno vreme da se stabilizuje nakon power-on. Makar da LED pocne da trepce :-)
	const unsigned int SENSOR_TIMEOUT_mS = 1000;	// TB600-CO-100 prosecan odgovor je oko 40..max 45mS. Ako ne odgovori za celu sekundu, nesto debelo ne valja!

	void init(uint32_t waitSensorStartup_mS);	// inicijalizuj senzor, podesi passive mode, proveri tip

	struct {
		uint8_t tip;
		uint16_t maxRange;
		char unit_str[15];
		uint8_t decimals;
		uint8_t sign;
	} sensorProperties;

	void getSensorProperties_D1_INTERNALUSEONLY();			// d1 i d7 daju drugaciji odgovor - ovo je samo debugging
	void getSensorProperties_D7();							// d7: popuni struct sa podacima o senzoru
	bool isChecksumValid(std::vector<uint8_t> repl);

};

#endif /* SRC_GASSENSOR_H_ */
