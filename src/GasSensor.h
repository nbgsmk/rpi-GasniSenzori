/*
 * GasSensor.h
 *
 */

#ifndef SRC_GASSENSOR_H_
#define SRC_GASSENSOR_H_

#define MEASUREMENT_ERROR (float)-1


#include "UartMux.h"
#include "GasSensor_TB200B_TB600BC_datasheet.h"


class GasSensor {
public:
	GasSensor(MuxAdr_t muxAddress, int uartHandle, int default_level = 0);
	virtual ~GasSensor();

	enum ErrCodes_t {
		OK = 0,
		NOT_DEFINED = 1,
		INIT_FAIL = 2,
		UNEXPECTED_SENSOR_TYPE = 3,
		WRONG_RESPONSE_HEADER = 4,
		SENSOR_TIMEOUT = 5,
		MEASUREMENT_INCOMPLETE = 6,
		MEASUREMENT_CHECKSUM_FAIL = 7,
		MEASUREMENT_OUT_OF_RANGE = 8,
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
	MuxAdr_t getMuxAddress();					// adresa na uart multiplekseru
	bool getChecksumValidatorState();

	int getSensorTypeHex();						// HEX vrednost - tip senzora
	std::string getSensorTypeStr();				// na osnovu HEX vracam tekstualni naziv
	int getMaxRange();							// maksimalni raspon merenja senzora
	int getDecimals();							// broj decimala u rezultatu
	float getGasConcentrationParticles();		// koncentracija gasa [ppm, ppb, %]
	float getGasConcentrationMass_DO_NOT_USE();	// koncentracija gasa [mg/m3, ug/m3, *10g/m3]
	float getGasPercentageOfMaxParticles();		// koncentracija 0~100% od maksimalnog opsega [ppm, ppb, %]
	std::string getUnitsForParticles();			// jedinica [ppm, ppb, %]... za getGasConcentrationParticles()
	std::string getUnitsForMass_DO_NOT_USE();				// jedinica [mg/m3, ug/m3, *10g/m3]... za getGasConcentrationMass_DO_NOT_USE()
	float getTemperature();						// sve zajedno merimo
	float getRelativeHumidity();				// sve zajedno merimo

	// debug only
	void sendRawBytes(const char *rawBytes, unsigned int size);

	// debug only. kasnije moze da se vrati da bude privatna - TODO
	std::vector<uint8_t> send(const CmdStruct_t txCmd);		// posalji komande senzoru, cekaj odgovor
//	struct ReplStruct_t {
//		std::vector<uint8_t> merenja;
//		ErrCodes_t status;
//	};


private:
	// Error handling je neophodan
	int DEBUG_LEVEL =0;
	unsigned int ERROR_CNT = 0;
	ErrCodes_t H_STAT = NOT_DEFINED;

	MuxAdr_t muxAddress;
	bool runningLed;
	int uartHandle;
	bool checksumValidatorIsActive = true;
	void setChecksumValidatorState(bool state);			// proverava se checksum rezultata ili se ignorise

	const unsigned int SENSOR_INIT_mS = 1000;			// minimalno vreme da se stabilizuje nakon power-on. Makar da LED pocne da trepce :-)
	const unsigned int SENSOR_TIMEOUT_mS = 1000;		// TB600-CO-100 prosecan odgovor je oko 40..max 45mS. Ako ne odgovori za celu sekundu, nesto debelo ne valja!

	void init(uint32_t waitSensorStartup_mS);			// inicijalizuj senzor, podesi passive mode, proveri tip

	struct {
		ErrCodes_t state = NOT_DEFINED;					// za sada write only ali ga kasnije ne proveravam. // TODO
		uint8_t tip = 0;
		float maxRange = -1;
		char unit_str_D7_concentration_1_particles[50] = "";
		char unit_str_D7_concentration_2_mass[50] = "";
		int decimals = -1;
		int sign = -1;
	} sensorProperties;

	// d1 i d7 daju drugaciji byte array kao odgovor
	ErrCodes_t getSensorProperties_D1_DO_NOT_USE();		// d1: popuni struct sa podacima o senzoru
	ErrCodes_t getSensorProperties_D7();				// d7: popuni struct sa podacima o senzoru
	bool isChecksumValid(std::vector<uint8_t> repl);

};

#endif /* SRC_GASSENSOR_H_ */
