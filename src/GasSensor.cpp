/*
 * Driver for "GasSensor_TB200B_TB600BC_datasheet"
 *
 */

// C++
#include <bits/stdint-uintn.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
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
#include "Logger.h"

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


/////////////////////////////////////////////////
/////////////////////////////////////////////////
// init, upravljanje
/////////////////////////////////////////////////
/////////////////////////////////////////////////

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
	send(cmdRunningLightGetStatus);	// Nije bitan rezultat koji se vraca. Ako nema komunikacije, dobice se timeout unutar send() metoda i postavice se H_STAT i ERROR_CNT


	const int _metod = 7;
	switch (_metod) {
		case 1:
			if(DEBUG_LEVEL >=2){
				cout << "init: get props via d1" << endl;
			}
			getSensorProperties_D1_DO_NOT_USE();		
			break;
		
		case 7:
			if(DEBUG_LEVEL >=2){
				cout << "init: get props via d7" << endl;
			}
			getSensorProperties_D7();
			break;
	}

	
	if(DEBUG_LEVEL >=1){
		int th = getSensorTypeHex();
		string cudno = (th==0) ? ", this is very likely an error!" : "";
		cout << "init: senzor tip 0x" << th << cudno << endl;
	}

	if(DEBUG_LEVEL >=2){
		cout << "init: set passive" << endl;
	}
	setPassiveMode();


	if(DEBUG_LEVEL >=2){
		cout << "init: set running light ON" << endl;
	}
	setLedOn();

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



/**
 * @param active mode: sensor will send measurement data automatically at 1 second intervals
 */
void GasSensor::setActiveMode() {
	send(cmdSetActiveMode);

	//
	// usleep(1000 * 2500);	// vidi ispod: setPassiveModeAndWaitInit()
							// Za svaki slucaj, cekam i ovde 2.5 sekunde.
}

/**
 * @param passive mode = sensor will send measurement data only when requested
 */
void GasSensor::setPassiveMode() {
	send(cmdSetPassiveMode);

	// Nije jasno ovo ispod ali do sada nikad nije pravilo probleme. 
	// usleep(1000 * 2500);	// (^1) 4.1.2 Switch to Query Mode: nije bas jasna njihova recenica na engleskom.
							// Da li hoce da kaze da odgovor stize u okviru 2 sekunde ili da sledeci odgovor dolazi tek nakon 2 sekunde?
							// "there will be data reply within..." kakav data reply kad se ovim aktivira pasivni mod? Ne ocekuje se nikakav reply.
							// Sta god znacilo, dobijao sam timeout na sledecoj komandi nakon slanja ove komande.
							// Zato, za svaki slucaj, cekam 2.5 Sec
}


/**
 * @return Vraca HARDVERSKU adresu na uart multiplekseru (gde je senzor fizicki povezan).
 * @warning Adresa ce biti tacna SAMO u trenutku dok je OVAJ senzor aktivan. Razni procesi mogu pristupati mux-u i menjati ga nezavisno.
 */
MuxAdr_t GasSensor::getMuxAddress(){
	return this->muxAddress;
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
 * @return true = led is blinking; false = led is off
 */
bool GasSensor::getLedStatus() {
	bool rezultat = false;
	vector<uint8_t> reply = send(cmdRunningLightGetStatus);

	// --------------------------------------
	// H_STAT dolazi PODESEN iz metoda send()
	// ako bilo sta ne valja, ovde je bezbedno da samo izadjem
	// --------------------------------------
	if (H_STAT != OK) {
		return false;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x8A);		// reply header ok?
	if (hdr) {
		rezultat = (reply.at(2) == 1) ? true : false;
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;
		rezultat = false;
	}
	return rezultat;
}



/////////////////////////////////////////////////
/////////////////////////////////////////////////
// sensor properties
/////////////////////////////////////////////////
/////////////////////////////////////////////////


/**
 * Query parameters from sensor with _D1_ command
 * @struct sensorProperties will be populated for later use
 * @return H_STAT modified locally in case of error, or passed-through from send() method (still could be error)
 */
GasSensor::ErrCodes_t GasSensor::getSensorProperties_D1_DO_NOT_USE(){
	//
	// VAZNO!! COMMAND = "D1". Odgovor je drugaciji nego za D7.
	// VAZNO VAZNO!! Ovo se pominje SAMO u datasheetu TB600. Necemo ga koristiti jer nije jasno da li se vazi i za TB200.
	// Ovaj odgovor cak nema header!
	//
	std::vector<uint8_t> reply = send(cmdGetTypeRangeUnitDecimals0xd1);

	if (this->DEBUG_LEVEL == -4){
		cout << "D1:";
		for (unsigned int i = 0; i < reply.size(); i++) {
			cout << "   0x" << setfill('0') << setw(2) << hex << static_cast<int>( reply.at(i) ) << "  " << std::dec;
		}
		cout << endl;
		cout << "D1:";
		for (unsigned int i = 0; i < reply.size(); i++) {
			std::cout << " " << std::bitset<8>(reply.at(i));
		}
		cout << endl;		
	}


	// H_STAT je vec podesen u metodu send()
	sensorProperties.state = H_STAT;	// ---> proveriti prilikom procesiranja merenja
	if (OK != H_STAT) {					// ako nije ok, nema svrhe ici dalje
		return H_STAT;					// ukljucujuci i timeout, nedovoljno bajtova, checksum ako je ukljucen itd
	}

	// H_STAT je sigurno OK, send() je vratio ok

	sensorProperties.tip = reply.at(0);
	sensorProperties.maxRange = (float) ( (reply.at(1) << 8) | reply.at(2) );
	switch (reply.at(3)) {
		case 0x02:
			strcpy(sensorProperties.unit_str_D7_concentration_1_particles, "ppm");
			strcpy(sensorProperties.unit_str_D7_concentration_2_mass, "mg/m3");
			break;

		case 0x04:
			strcpy(sensorProperties.unit_str_D7_concentration_1_particles, "ppb");
			strcpy(sensorProperties.unit_str_D7_concentration_2_mass, "ug/m3");
			break;

		case 0x08:
			strcpy(sensorProperties.unit_str_D7_concentration_1_particles, "%");
			strcpy(sensorProperties.unit_str_D7_concentration_1_particles, "*10g/m3");
			break;

		default:
			this->H_STAT = UNEXPECTED_SENSOR_TYPE;
			strcpy(sensorProperties.unit_str_D7_concentration_1_particles, "d1:_?_unit_");
			strcpy(sensorProperties.unit_str_D7_concentration_2_mass, "d1:_?_unit_");
			break;
	}
	sensorProperties.decimals = (int) ( ( reply.at(7) & 0b11110000 ) >> 4 );	// decimal places:bit 7~4, zatim shift >>4 da dodje na LSB poziciju
	// TB600 datasheet ne pominje sign!?
	// sensorProperties.sign = (int) ( reply.at(7) & 0b00001111 );					// sign bits 3~0
	// Pretpostavicu da je pozitivno!? Ne mogu da mnozim sa praznim.
	sensorProperties.sign = (int)+1;

	sensorProperties.state = this->H_STAT;
	return H_STAT;		// propagiram H_STAT u caller metod (sta god da je vratio send())
}


/**
 * Query parameters from sensor with _D7_ command
 * @struct sensorProperties will be populated for later use
 * @return H_STAT modified locally in case of error, or passed-through from send() method (still could be error)
 */
GasSensor::ErrCodes_t GasSensor::getSensorProperties_D7() {
	//
	// VAZNO!! Saljem COMMAND 1 = "D7". Odgovor je drugaciji nego za D1
	//
	std::vector<uint8_t> reply = send(cmdGetTypeRangeUnitDecimals0xd7);

	if (this->DEBUG_LEVEL == -4){
		cout << "D7:";
		for (unsigned int i = 0; i < reply.size(); i++) {
			cout << "   0x" << setfill('0') << setw(2) << hex << static_cast<int>( reply.at(i) ) << "  " << std::dec;
		}
		cout << endl;
		cout << "D7:";
		for (unsigned int i = 0; i < reply.size(); i++) {
			std::cout << " " << std::bitset<8>(reply.at(i));
		}
		cout << endl;
	}

	// H_STAT je vec podesen u metodu send()
	sensorProperties.state = H_STAT;	// ---> proveriti prilikom procesiranja merenja
	if (OK != H_STAT) {					// ako nije ok, nema svrhe ici dalje
		return H_STAT;					// ukljucujuci i timeout, nedovoljno bajtova, checksum ako je ukljucen itd
	}

	// H_STAT je sigurno OK, send() je vratio ok

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0xD7);	// reply header ok?
	if (hdr) {
		sensorProperties.tip = reply.at(2);
		sensorProperties.maxRange = (float) ( (reply.at(3) << 8) | reply.at(4) );	// range u jedinicama prema "concentration 1"
		switch (reply.at(5)) {
			case 0x02:
				strcpy(sensorProperties.unit_str_D7_concentration_1_particles, "ppm");
				strcpy(sensorProperties.unit_str_D7_concentration_2_mass, "mg/m3");
				break;

			case 0x04:
				strcpy(sensorProperties.unit_str_D7_concentration_1_particles, "ppb");
				strcpy(sensorProperties.unit_str_D7_concentration_2_mass, "ug/m3");
				break;

			case 0x08:
				strcpy(sensorProperties.unit_str_D7_concentration_1_particles, "%");
				strcpy(sensorProperties.unit_str_D7_concentration_2_mass, "*10g/m3");
				break;

			default:
				this->H_STAT = UNEXPECTED_SENSOR_TYPE;
				strcpy(sensorProperties.unit_str_D7_concentration_1_particles, "d7:_?_unit_");
				strcpy(sensorProperties.unit_str_D7_concentration_2_mass, "d7:_?_unit_");
				break;
		}
		uint8_t dec = reply.at(6) & 0b11110000;		// decimal placess: bit 7~4
		sensorProperties.decimals = (int)(dec>>4);	// shift to lsb  (tabela 5. Decimal Places)

		uint8_t sgn = reply.at(6) & 0b00001111;					// sign: bits 3~0
		sensorProperties.sign = (int) (sgn==0) ? (+1) : (-1);	// 0=>positive, 1=>negative (tabela 4. Data Of Type)

		__asm("NOP");
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;		// silently report
		if(DEBUG_LEVEL > 0){
			cout << "sensor properties: wrong response header! STRONGLY suggest to discard any measurements.";
		}
	}

	sensorProperties.state = this->H_STAT;
	return H_STAT; 		// propagiram H_STAT u caller metod (sta god da je vratio send()
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
	std::stringstream rez;
	rez << "";
	// Raw property je HEX a funkcija castuje na INT.
	// NE KORISTI FUNKCIJU da se ne zajebes!
	switch (sensorProperties.tip) {
		case 0x19:
			rez << "CO";
			break;

		case 0x1C:
			rez << "H2S";
			break;

		case 0x22:
			rez << "O2";
			break;

		default:
			/* 
			* ovo se desava ako je:
			* - senzor stvarno prijavio neki tip koji ovde nije definisan - onda treba dopuniti slucajeve gore
			* - nastupila bilo kakva greska u komunikaciji sa senzorom
			* - senzor nije nista odgovorio i properties.tip je prazan ili nula ili null valjda
			*/
			// ovo se desava i ako je nastupila greska ili
			rez << "unknown typ " << std::hex << sensorProperties.tip;
			this->H_STAT = UNEXPECTED_SENSOR_TYPE;
			break;
	};

	return rez.str();
}


/**
 * @return maximum measurement range from sensor properties
 */
int GasSensor::getMaxRange() {
	return (int) sensorProperties.maxRange;
}

/**
 * @brief Get sensor precision
 * @return number of decimal places as reported by sensor
 */
int GasSensor::getDecimals() {
	return sensorProperties.decimals;
}


/**
 * @brief For 0xD7 properties: get units for particle measurement ie "Concentration 1" from the datasheet table
 * @return one of: ppm, ppb, %... (datasheet "3. Gas Concentration Unit of Type")
 */
std::string GasSensor::getUnitsForParticles(){
	return sensorProperties.unit_str_D7_concentration_1_particles;
}



/**
 * @brief DO NOT USE-> For 0xD1 properties: get units for mass measurement ie "Concentration 2" from the datasheet table
 * @return one of: mg/m3, ug/m3, 10g/m3... (datasheet "3. Gas Concentration Unit of Type")
 */
std::string GasSensor::getUnitsForMass_DO_NOT_USE(){
	return sensorProperties.unit_str_D7_concentration_2_mass;
}






/////////////////////////////////////////////////
/////////////////////////////////////////////////
// merenja
/////////////////////////////////////////////////
/////////////////////////////////////////////////


/**
 * @return current gas concentration in units of ppm, ppb, %
 */
float GasSensor::getGasConcentrationParticles() {
	float rezultat = 0.0f;
	vector<uint8_t> reply = send(cmdReadGasConcentrationTempAndHumidity);
	getSensorProperties_D7();	// popunim D7 da bih imao najnoviji sign. sta ako je bas malopre bila negativna vrednost, npr -5°C

	if (this->DEBUG_LEVEL == -4){
		cout << "(" << getSensorTypeStr() << " raw measurement[hex]: ";		// "O2 raw ppm: 0x02, 0x1c...
		Logger::dhex(reply);
		cout << ")" << endl;
	}

	if (OK != H_STAT) {
		// H_STAT je vec podesen u metodu send()
		return MEASUREMENT_ERROR;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x87);		// reply header ok?
	if (hdr) {
		float max = (float) ( (reply.at(4) << 8) | (reply.at(5)) );	// range odgovarajuci za "concentration 1". Daje skaliranu vrednost npr 100ppm (ili 25% za O2)
		float ppm = (float) ( (reply.at(6) << 8) | (reply.at(7)) );	// byte[7:6]=Concentration 1 ppm, ppb... se pominje u oba datasheeta
		ppm = ppm / powf(10.0, sensorProperties.decimals);			// i mora da se deli sa 10^(br decimala) tj 2->delim sa 100; 3->delim sa 1000
		ppm = ppm * sensorProperties.sign;							// i pomnozim sa znakom
		rezultat = ppm;
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;
		rezultat = MEASUREMENT_ERROR;
	}
	if ( rezultat < 0 ) {
		this->H_STAT = MEASUREMENT_OUT_OF_RANGE;
		rezultat = MEASUREMENT_ERROR;
	}
	return rezultat;
}

/**
 * @return gas concentration in units of mg/m3, ug/m3, *10g/m3
 */
float GasSensor::getGasConcentrationMass_DO_NOT_USE() {
	float rezultat = 0.0f;
	std::vector<uint8_t> reply = send(cmdReadGasConcentrationTempAndHumidity);
	getSensorProperties_D7();	// popunim D7 da bih imao najnoviji sign. sta ako je bas malopre bila negativna vrednost?

	if (this->DEBUG_LEVEL == -4){
		cout << "(" << getSensorTypeStr() << " raw measurement[hex]: "; 	// "O2 raw mg/m3: 0x02, 0x1c...
		Logger::dhex(reply);
		cout << ")" << endl;
	}

	if (OK != H_STAT) {
		// H_STAT je vec podesen u metodu send()
		return MEASUREMENT_ERROR;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x87);		// reply header ok?
	if (hdr) {
		// float max = (float) ( (reply.at(4) << 8) | (reply.at(5)) );	// NE!!! Max range se odnosi samo na ppm, ppb...tj "concentration 1"
		float mgm3 = (float) ( (reply.at(2) << 8) | (reply.at(3)) );	// byte[7:6]=Concentration 2 mg/m3, ug/m3... se pominje samo u TB600
		mgm3 = mgm3 / powf(10.0, sensorProperties.decimals);			// podelim sa 10^(br decimala)
		mgm3 = mgm3 * sensorProperties.sign;							// i pomnozim sa znakom
		rezultat = mgm3;
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;
		rezultat = MEASUREMENT_ERROR;
	};
	if ( rezultat < 0 ) {
		this->H_STAT = MEASUREMENT_OUT_OF_RANGE;
		rezultat = MEASUREMENT_ERROR;
	}
	return rezultat;
}

/**
 * @return gas concentration normalized to 0 ~ 100% of max range. Unit is ppm,ppb,%
 */
float GasSensor::getGasPercentageOfMaxParticles() {
	float rezultat = 0.0f;
	vector<uint8_t> reply = send(cmdReadGasConcentrationTempAndHumidity);
	getSensorProperties_D7();	// popunim D7 da bih imao najnoviji sign. sta ako je bas malopre bila negativna vrednost?

	if (this->DEBUG_LEVEL == -4){
		cout << "(" << getSensorTypeStr() << " raw measurement[hex]: ";	// "O2 raw %: 0x02, 0x1c...
		Logger::dhex(reply);
		cout << ")" << endl;
	}

	if (OK != H_STAT) {
		// H_STAT je vec podesen u metodu send()
		return MEASUREMENT_ERROR;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x87);		// reply header ok?
	if (hdr) {
		float max = (float) ( (reply.at(4) << 8) | (reply.at(5)) );		// range vec daje skaliranu vrednost npr 100 ili 25ppm
		float ppm = (float) ( (reply.at(6) << 8) | (reply.at(7)) );		// ovo mora da se podeli sa brojem decimala
		ppm = ppm / powf(10.0f, sensorProperties.decimals);				// podeli sa brojem decimala
		rezultat = (ppm / max) * 100.0f;
		rezultat = std::abs(rezultat);									// ako je ppm bio negativan
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;
		rezultat = MEASUREMENT_ERROR;
	}
	if (rezultat < 0) {
		this->H_STAT = MEASUREMENT_OUT_OF_RANGE;
		rezultat = MEASUREMENT_ERROR;
	}
	return rezultat;
}


/////////////////////////////////////////////////
/////////////////////////////////////////////////
// bonus merenja
/////////////////////////////////////////////////
/////////////////////////////////////////////////


/**
 * @return temperature from combined reading (datasheet Command 2)
 */
float GasSensor::getTemperature() {
	float rezultat = 0.0f;
	vector<uint8_t> reply = send(cmdReadGasConcentrationTempAndHumidity);
	getSensorProperties_D7();	// popunim D7 da bih imao najnoviji sign. sta ako je bas malopre bila negativna vrednost?

	if (OK != H_STAT) {
		// H_STAT je vec podesen u metodu send()
		return (float) MEASUREMENT_ERROR;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x87);		// reply header ok?
	if (hdr) {
		float temp =  (float) ( (reply.at(8) << 8)  | (reply.at(9))  );
		float humid = (float) ( (reply.at(10) << 8) | (reply.at(11)) );
		temp = temp / 100.0f;
		humid = humid / 100.0f;
		rezultat = temp;
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;
		rezultat = MEASUREMENT_ERROR;
	}
	return rezultat;
}

/**
 * @return relative humidity from combined reading (datasheet Command 6)
 */
float GasSensor::getRelativeHumidity() {
	float rezultat = 0.0f;
	vector<uint8_t> reply = send(cmdReadGasConcentrationTempAndHumidity);
	getSensorProperties_D7();	// popunim D7 da bih imao najnoviji sign. sta ako je bas malopre bila negativna vrednost?

	if (OK != H_STAT) {
		// H_STAT je vec podesen u metodu send()
		return MEASUREMENT_ERROR;
	}

	bool hdr = (reply.at(0) == 0xFF) && (reply.at(1) == 0x87);		// reply header ok?
	if (hdr) {
		float temp =  (float) ( (reply.at(8) << 8)  | (reply.at(9))  );
		float humid = (float) ( (reply.at(10) << 8) | (reply.at(11)) );
		temp = temp / 100.f;
		humid = humid / 100.f;
		rezultat = humid;
	} else {
		this->H_STAT = WRONG_RESPONSE_HEADER;
		rezultat = MEASUREMENT_ERROR;
	}
	if (rezultat < 0) {
		this->H_STAT = MEASUREMENT_OUT_OF_RANGE;
		rezultat = MEASUREMENT_ERROR;
	}
	return rezultat;
}



/////////////////////////////////////////////////
/////////////////////////////////////////////////
// low level komunikacija
/////////////////////////////////////////////////
/////////////////////////////////////////////////


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
	}
	if(DEBUG_LEVEL >= 3) {
		cout << "cmd to send:";
		for (unsigned int i = 0; i < txStruct.cmd.size(); ++i) {
			cout << " " << std::hex << static_cast<unsigned int>(txStruct.cmd[i]) << std::dec;
		}
		cout << std::dec << endl;
	}

	for (unsigned int i = 0; i < txStruct.cmd.size(); i++) {
		serialPutchar(this->uartHandle, txStruct.cmd[i]);
		// At 9600 baud, the 1 bit time is ~104 microseconds, and 1 full character is 1.04mS
		usleep(500);	// some guard time between characters. No reason, just because.
	};


	// Ako ocekujem neki odgovor od senzora
	if (txStruct.expectedReplyLen > 0) {
		unsigned int mS = 0;

		/*
		 * Na pocetku je buffer prazan tj serialDataAvail() == 0
		 * pa se POLAKO PUNI karakterima, kako pristizu od senzora
		 * Zato cekam da
		 * - available postane jednako expected
		 *  ili
		 * - istekne timeout (senzor je neispravan ili fizicki iskljucen)
		 */
		while (serialDataAvail(this->uartHandle) < (int) txStruct.expectedReplyLen) {
			usleep(1000);
			mS++;

			if ( (DEBUG_LEVEL >= 3) && (mS > (SENSOR_TIMEOUT_mS-5) ) ){
				cout << "avail sofar: " << serialDataAvail(this->uartHandle) << ", expect:" << txStruct.expectedReplyLen << ", (int)expect:" << txStruct.expectedReplyLen << endl;
			}
			if (mS >= SENSOR_TIMEOUT_mS) {
				// isteklo vreme!
				// Samo break iz ove petlje, metod send() nastavlja dalje, ako je potreban neki cleanup
				this->H_STAT = SENSOR_TIMEOUT;
				reply.clear();
				if(DEBUG_LEVEL >=1){
					cout << "ERROR!! Timeout in sensor comunication after " << mS << "mS" << endl;
				}
				if (DEBUG_LEVEL >= 3){
					std::vector<uint8_t> garbage;
					garbage.clear(); 
					while ( serialDataAvail(this->uartHandle) > 0 ) {
						uint8_t x = serialGetchar(this->uartHandle);
						garbage.push_back(x);
					};
					cout << "received garbage until timeout: " << endl;
					Logger::dhex(garbage);
				}
				break;
			}
		}

		// ovde dodjemo iskljucivo ako je:
		// - stiglo DOVOLJNO karaktera
		// ili
		// - istekao timeout
		if (mS < SENSOR_TIMEOUT_mS) {
			// stiglo je dovoljno bajtova na vreme
			while ( serialDataAvail(this->uartHandle) > 0 ) {
				uint8_t x = serialGetchar(this->uartHandle);
				reply.push_back(x);
			};

			// stiglo je DOVOLJNO bajtova
			// da vidimo da li su ispravni

			// podesimo error statuse
			// NEPOTREBNO!
			// bool sajzOk = (reply.size() == txStruct.expectedReplyLen);		// ne diraj duplo jednako!
			// if ( ! sajzOk ) { this->H_STAT = MEASUREMENT_INCOMPLETE; };		// stiao je pogresan broj bajtova


			// pa sad slicno ali za console debugging
			if (DEBUG_LEVEL >= 3) {
				bool sajzOk = (reply.size() == txStruct.expectedReplyLen);		// ne diraj duplo jednako!
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
			}
			if (DEBUG_LEVEL >= 2) {
				cout << "reply received after " << mS << "mS" << endl;
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

	//
	// FULL FREEZE!! NE FLUSHUJ RECEIVER OVDE!! Ako je expected reply bytes = 0, 
	// posle ovoga sledeca komanda ne vraca nista, nema bajtova, nema odgovora!!
	// JEDVA sam ga pronasao!
	//

	return reply;
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


/////////////////////////////////////////////////
/////////////////////////////////////////////////
// error checking, debug
/////////////////////////////////////////////////
/////////////////////////////////////////////////


/**
 * @return poslednji zabelezeni error code. Mozda ne mora da potice od poslednjeg obavljenog merenja?
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


