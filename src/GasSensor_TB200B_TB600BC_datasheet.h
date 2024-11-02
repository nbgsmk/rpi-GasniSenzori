/*
 * Driver for gas sensor "TB600C CO 100ppm"
 *
 */

#ifndef GASSENSOR_TB200B_TB600BC_H_
#define GASSENSOR_TB200B_TB600BC_H_



	struct CmdStruct_t{
		std::vector<uint8_t> cmd;		// uint8_t konkretno komanda koja se salje
		unsigned int expectedReplyLen;	// ocekivana duzina odgovora, od senzora, u bajtovima
		bool checksumPresent;			// da li ODGOVOR sadrzi cecksum?
	};

	///////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////
	// Driver for gas sensor
	// "TB600C CO 100ppm"
	// according to manufacturer pdf
	// "TB600B&C Gas Module Communication Protocol_V2.0_20211020"
	// www.ecsense.com
	// www.ecsense.cn
	///////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////



	///////////////////////////////////////////////////////////////
	// (^1) verzija: UPDATE iz Lucas Schwarz email 2024-10-16 13:11
	//
	// Prema pdf:
	// (prva stranica)			"TB200B Gas Sensor Module UART Interface Communication Protocol Version：V1.1 [Date：05.04.2021]"
	// (poslednja stranica)		"DGM10 Gas Sensor Module_Conmunication Protocol_V1.1_20210405"
	// Jel' verujes da je ovo starije od prvobitnog?!??
	// 
	///////////////////////////////////////////////////////////////


	//	1. Communication Mode
	//	---------------------
	//	Signal Voltage  3.3 
	//	Baud Rate  		9600 
	//	Data Bit  		8 bits
	//	Stop Bit  		1 bit 
	//	Parity  		None 


	//	2. Sensor Type
	//	The sensor gas type can be identified by read out from the sensor module
	// Sensor	value
	// -------------
	// HCHO		0x17
	// VOC		0x18
	// CO		0x19
	// Cl2		0x1A
	// H2		0x1B
	// H2S		0x1C
	// HCl		0x1D
	// HCN		0x1E
	// HF		0x1F
	// NH3		0x20	// (^1): pdf za TB200B kaze 0xC4. Nelogicno! Do sada su isli rastucim redom.
	//					// (^1): U pdf-u za TB600B & C ocekivana vrednost 0x20. Za sada, bas me briga, ne koristimo ga.
	// NO2		0x21
	// O2		0x22
	// O3		0x23
	// SO2		0x24	// (^1): Ovde je kraj pfd-a za TB200, dalje postoje podaci samo u pdf-u za TB600
	// HBr		0x25
	// Br2		0x26
	// F2		0x27
	// PH3		0x28
	// AsH3		0x29
	// SiH4		0x2A
	// GeH4		0x2B
	// B2H6		0x2C
	// BF3		0x2D
	// WF6		0x2E
	// SiF4		0x2F
	// XeF2		0x30
	// TiF4		0x31
	// SMELL	0x32
	//
	// Sensor	value (continued)
	// -------------
	// IAQ		0x33
	// AQI		0x34
	// NMHC		0x35
	// SOx		0x36
	// NOx		0x37
	// NO		0x38
	// C4H8		0x39
	// C3H8O2	0x3A
	// CH4S		0x3B
	// C8H8		0x3C
	// C4H10	0x3D
	// C2H6		0x3E
	// C6H14	0x3F
	// C2H4O	0x40
	//
	// Sensor	value (continued)
	// -------------
	// C3H9N	0x41
	// C3H7N	0x42
	// C2H6O	0x43
	// CS2		0x44
	// C2H6S	0x45
	// C2H6S2	0x46
	// C2H4		0x47
	// CH3OH	0x48
	// C6H6		0x49
	// C8H10	0x4A
	// C7H8		0x4B
	// CH3COOH	0x4C
	// ClO2		0x4D
	//
	// Sensor	value (continued)
	// -------------
	// H2O2		0x4E
	// N2H4		0x4F
	// C2H8N2	0x50
	// C2HCl3	0x51
	// CHCl3	0x52
	// C2H3Cl3	0x53
	// H2Se		0x54
	// Other	0x55	// (^1): ovaj entry vise ne postoji.


	// 3. Gas Concentration type table  
	//	+---------------------------------------------+
	//	| Code            |  0x02  |  0x04  |  0x08   |
	//	+---------------------------------------------+
	//	| Concentration 1 |  ppm   |   ppb  |  %vol.  |
	//	| Concentration 2 |  mg/m3 |  ug/m3 |  10g/m3 |
	//	+---------------------------------------------+
	//	1 ppm = 1000 ppb
	//	1% vol = 10000 ppm 
	//	1 mg/m3 = 1000 ug/m3 
	//
	// TB600b/c dodatno: 
	// -----------------
	// The module is calibrated during production in the unit Concentration-1, while Concentration-2 is 
	// calculated using a concentration conversion formula. 
	// Additionally, the range is also speciﬁed in the unit of Concentration-1.
	// The concentration conversion formulas for Concentration-1 and Concentration-2 are as follows:
	// mg/m 3  = M/22.4*ppm*[273/(273+T)]*(Ba/101325)
	// M: Molecular Weight
	// T: Temperature (Celsius)
	// Ba: Atmospheric Pressure
	// In the internal module conversion, Ba is set to 1 standard atmosphere, 
	// and T is set to 25 degrees Celsius. Since VOCs (volatile organic compounds) 
	// have an unknown molecular weight, the conversion ratio 
	// between Concentration-2 and Concentration-1 is 1:1.
	// 
	// prim.prev: koristicemo samo Concentration-1. 


	// 4. Data of Type
	//	-----------------------
	//	0x00	Positive Number 
	//	0x01	Negative Number 
	//	-----------------------


	//	5. Decimal Places 
	//	-----------------
	//	0x00	none
	//	0x01	1 decimal
	//	0x02	2 decimal
	//	0x03	3 decimal
	//



	//	6.1 Command 1: 0xD7
	//	Get the sensor type, maximum range, unit, and decimal places: 0xD7
	const std::vector<uint8_t> cmd_get_type_range_unit_decimals_0xD7 = {
			0xD7
	};
	const CmdStruct_t cmdGetTypeRangeUnitDecimals0xd7 = { cmd_get_type_range_unit_decimals_0xD7, 9, true };
	// Return value (ne pise izricito u pdf-u ali valjda je to return value - prim. prev.)
	//	0xFF	0	Fixed command header
	//	0xD7	1	Fixed command header
	//	0x19	2	sensor type
	//	0x03	3	Byte[4:3] Sensor Range (uint16) = ( Byte[3]<<8) | Byte[4] , unit here corresponds to Concentration 1 in Read-Out Measured Values table below
	//	0xE8	4	Byte[4:3] Sensor Range (uint16) = ( Byte[3]<<8) | Byte[2] , unit here corresponds to Concentration 1 in Read-Out Measured Values table below
	//	0x02	5	Unit_type: Unit for Concentration-2 is mg/m³, unit for Concentration-1 is ppm. See Gas Concentration type table
	//	0x30	6	bit[7:4] number of decimal places (see Decimal Places table); bit[3:0] (see Data of Type table )
	//	0x00	7	Reserved
	//	0xF3	8	Checksum see below
	//
	//	Checksum = (Byte[1]+Byte[2]+Byte[3]+Byte[4]+Byte[5]+Byte[6]+Byte[7])
	//	Checksum = 0-checksum
	//	drugim recima: add byte 1~7 to generate an 8-bit data, invert each bit, add 1 at the end
	//	trecim recima: saberi bajtove 1~7, zatim uradi komplement dvojke



	//	6.2 Read-Out Measured Values
	//	COMMAND 2
	//	Command2 is used to read out measurement values (gas concentrations), temperature, humidity, sensor module range, sensor module gas type performance.
	const std::vector<uint8_t> cmd_read_gas_concentration_temp_and_humidity = {
			//    0      |    1   |    2    |    3   |   4    |   5    |    6   |    7   |    8
			// Start bit | Retain | Command | Retain | Retain | Retain | Retain | Retain | Checksum
			     0xFF,       0x01,    0x87,     0x00,   0x00,    0x00,     0x00,    0x00,    0x78
	};
	const CmdStruct_t cmdReadGasConcentrationTempAndHumidity = { cmd_read_gas_concentration_temp_and_humidity, 13, true };
	// RETURN VALUE:
	//
	//	byte[1:0]	0xFF, 0x87 Fixed command header
	//	byte[3:2]	Concentration 2 (uint16)
	//					Concentration2 = (float) ( (Byte[2]<<8)|Byte[3] )
	//					Gas concentration value = Value + Decimal + Unit	-> refer to Decimal Places table
	//					For example：
	//					CO gas sensor, range is 0-1000ppm, Resolution is 0.1ppm(please find this information from Order Information of Sensor datasheet)
	//					It has 1 Decimal, according this formula to calculate: Concentration = Concentration/10
	//					Unit : mg/m3 、ug/m3、10g/m3，Please see above the Gas Concentration Unit table. 
	//	byte[5:4]	Senzor range
	//					Range = (Byte[4]<<8)| Byte[5] 
	//					Unit: ppm 、ppb、%，Please see above the Gas Concentration Unit table.
	//					Gas concentration unit here corresponds to Concentration 1 in this table
	//	byte[7:6]	Concentration 1
	//					Concentration = (float)( (Byte[6]<<8) | Byte[7] )
	//					concentration value = Value + Decimal + Unit	-> refer to Decimal Places table
	//					For example：
	//					CO gas sensor, range is 0-1000ppm, Resolution is 0.1ppm(please find this information from Order Information of Sensor datasheet)
	//					It has 1 Decimal, according this formula to calculate: Concentration = Concentration/10
	//					Unit : mg/m3 、ug/m3、10g/m3，Please see above the Gas Concentration Unit table. 
	//	byte[9:8]	Temperature (int16) Value，there are two decimal places. 
	//					Unit ℃
	//					Temperature = (float) ( (Byte[8]<<8) | Byte[9] ) / 100
	//	byte[11:10]	Relative Humidity, there are two decimal places 
	//					Unit: %rh
	//					RH = (float) ( (Byte[10]<<8) | Byte[11] ) / 100
	//	byte[12]	Checksum
	//					Checksum =(Byte[1]+Byte[2]+Byte[3]+Byte[4]+Byte[5]+Byte[6]+Byte[7]+ Byte[8]+Byte[9]+Byte[10]+Byte[11])
	//					Checksum = 0-Checksum



	//	6.3 Read-Out Temperature and Humidity value 0xD6
	//	COMMAND 3: 0xD6
	const std::vector<uint8_t> read_temp_and_humidity = {
			0xD6
	};
	const CmdStruct_t readTempAndHumidity = { read_temp_and_humidity, 5, true};
	// RETURN VALUE:
	//
	//	byte[1:0]	Temperature，there are two decimal places.
	//					Unit: ℃
	//					Temperature = (float) ( (Byte[0]<<8) | Byte[1] ) / 100
	//	byte[3:2]	Relative Humidity，there are two decimal places
	//					Unit: %rh
	//					RH = (float) ( (Byte[2]<<8) | Byte[3] ) / 100
	//	byte[4]		Checksum = (Byte[0]+Byte[1]+Byte[2]+Byte[3]) 
	//				Checksum = 0-Checksum



	//	6.4 Read-Out Firmware Version
	//	The firmware of the sensor module can be updated. If you want to get information, please follow the number in the Read-Out 
	//	Command 4: 0xD3
	const std::vector<uint8_t> read_firmware_version = {
			0xD3
	};
	const CmdStruct_t readFirmwareVersion = { read_firmware_version, 6, false};
	//	RETURN VALUE:
	//
	// Bukvalno pise 
	//
	// 软件版本号
	//
	// Da ne verujes hahahahaha
	// TB600: Example: 0x20 0x23 0x11 0x08 0x14 0x54: 0x202311081454, After conversion to BCD code, 202311081454



	//////////////////////////////////////////////////
	//
	//	OVO CEMO DA IMPLEMENTIRAMO MOZDA JEDNOG DANA
	//
	//////////////////////////////////////////////////
	//	6.5 Sleeping Mode
	// For less power consumption, please follow the table below to control the sensor module's sleep or awake mode.
	// After entering the sleep mode, other commands will not be executed. 
	// For sending other commands you must exit the sleeping mode.
	//
	//	Command for Enter Sleeping Mode :
	//	0xA1  0x53  0x6C  0x65  0x65  0x70  0x32 
	//	RETURN VALUE
	//	0xFF  0xA1  0x00  0x00  0x00  0x00  0x00  0x00  0x5F 
	//
	//	Exit sleeping Mode
	//	Command:
	//	0xA2  0x45  0x78  0x69  0x74  0x32 
	//	RETURN VALUE
	//	0xFF  0xA2  0x00  0x00  0x00  0x00  0x00  0x00  0x5E 
	//////////////////////////////////////////////////
	//////////////////////////////////////////////////


	//	6.6 Control LED
	//////////////////////////////////
	// RUNNING LIGHTS ON SENSOR MODULE  (led koji trepce dok senzor radi - prim. prev.)
	//////////////////////////////////

	// LED OFF
	// Turn off the running light
	const std::vector<uint8_t> cmd_running_light_off = {
			//    0      |    1   |    2    |    3   |   4    |   5    |    6   |    7   |    8
			// Start bit | Retain | Command | Retain | Retain | Retain | Retain | Retain | Checksum
			     0xFF,       0x01,    0x88,     0x00,   0x00,    0x00,     0x00,    0x00,    0x77
	};
	 const CmdStruct_t cmdRunningLightOff = { cmd_running_light_off, 2, false };
	// RETURN VALUE:
	//
	//	0x4F	0
	//	0x4B	1


	// LED ON
	// Turn on the running light (led koji trepce dok senzor radi - prim. prev.)
	const std::vector<uint8_t> cmd_running_light_on = {
			//    0      |    1   |    2    |    3   |   4    |   5    |    6   |    7   |    8
			// Start bit | Retain | Command | Retain | Retain | Retain | Retain | Retain | Checksum
			     0xFF,       0x01,    0x89,     0x00,   0x00,    0x00,     0x00,    0x00,    0x76
	};
	const CmdStruct_t cmdRunningLightOn = { cmd_running_light_on, 2, false };
	// RETURN VALUE:
	//
	//	0x4F	0
	//	0x4B	1


	// LED STATUS
	// Query the running light status
	const std::vector<uint8_t> cmd_running_light_get_status = {
			// Start bit | Retain | Command | Retain | Retain | Retain | Retain | Retain | Checksum
			//    0      |    1   |    2    |    3   |   4    |   5    |    6   |    7   |    8
			     0xFF,       0x01,    0x8A,     0x00,   0x00,    0x00,     0x00,    0x00,    0x75
	};
	const CmdStruct_t cmdRunningLightGetStatus = { cmd_running_light_get_status, 9, true };
	// RETURN VALUE:
	//
	//	0xFF	0	Start bit
	//	0x8A	1	Command
	//	0x01	2	State value (1:light on), (0:light off)
	//	0x00	3	Reserved
	//	0x00	4	Reserved
	//	0x00	5	Reserved
	//	0x00	6	Reserved
	//	0x00	7	Reserved
	//	0x75	8	Checksum =(Byte[1]+Byte[2]+Byte[3]+Byte[4]+Byte[5]+Byte[6]+Byte[7])
	//				Checksum = 0-Checksum 





/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// KRAJ DATASHEET-a TB200B
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Odavde do kraja su samo specificnosti iz datasheet-a za TB600b&c
//
/////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////
	//	4.1 Communication Mode Switching Command
	//////////////////////////////////////////////
	//
	//	4.1.1 Switch to Auto Reporting Mode
	const std::vector<uint8_t> cmd_set_active_mode = {
			//    0      |    1   |       2        |       3       |   4    |   5    |    6   |    7   |    8
			// Start bit | Retain | Switch command | Active upload | Retain | Retain | Retain | Retain | Checksum
			     0xFF,       0x01,       0x78,            0x40,       0x00,    0x00,     0x00,    0x00,    0x47
	};
	const CmdStruct_t cmdSetActiveMode = { cmd_set_active_mode, 0, false };
	// RETURN VALUE:
	/////////////////////////////////////
	//	Rx (uplink data) format (every 1s)
	//
	//	0xFF, 0x86	byte[0:1]	Command header
	//	0x25, 0xBC	byte[2:3]	Concentration-2 (uint16). 0x25BC=9660 -> the result is 9.66 mg/m³. See section Query Sensor Module Parameters
	//	0x03, 0xE8	byte[4:5]	Full Range (uint16). 0x03E8=1000 -> the result is 1000 ppm. See section Query Sensor Module Parameters
	//	0x20, 0xD0	byte[6:7]	Concentration-1 (uint16). 0x20D0=8400 -> the result is 8.4 ppm. See section Query Sensor Module Parameters
	//	0xBE		byte[8]		Checksum ( -(0x86+0x25+0xBC+0x03+0xE8+0x20+0xD0) ) & 0xFF = 0xBE


	//
	//	4.1.2 Switch to Query Mode
	const std::vector<uint8_t> cmd_set_passive_mode = {
			//    0      |    1   |       2        |    3   |   4    |   5    |    6   |    7   |    8
			// Start bit | Retain | Switch command | Answer | Retain | Retain | Retain | Retain | Checksum
			     0xFF,       0x01,       0x78,         0x41,   0x00,    0x00,     0x00,    0x00,    0x46
	};
	const CmdStruct_t cmdSetPassiveMode = { cmd_set_passive_mode, 0, false };
	//	RETURN VALUE:
	//	After sending the downlink command, there will be data reply within the next 2 seconds, which means the switch is successful.




	//////////////////////////////////////////////
	//	4.2 Query Sensor Module Parameters
	//////////////////////////////////////////////
	//
	//	4.2.1 Query Sensor Module Parameters (Method 1)
	// 0xD1
	const std::vector<uint8_t> cmd_get_type_range_unit_decimals_0xD1 = {
			0xD1
	};
	const CmdStruct_t cmdGetTypeRangeUnitDecimals0xd1 = { cmd_get_type_range_unit_decimals_0xD1, 9, true };
	// RETURN VALUE:
	//	0x19			byte[0]		Sensor type (see table)
	//	0x03, 0xE8		byte[1:2]	Full range (uint16). (see table). 0x03E8=1000 -> the result is 1000 ppm
	//	0x02			byte[3]		Unit for  Concentration-2 is mg/m³. Unit for Concentration-1 is ppm. (see table)
	//	0x00 0x00 0x00	byte[4:6]	Reserved
	//	0x30			byte[7]		Decimal places: 0x30 >> 4 = 0x03 represent 3 decimal places. Divide Concentration-2 and Concentration-1 by 1000
	//	0xE3			byte[8]		Checksum ( -(0x03+0xE8+0x02+0x00+0x00+0x00+0x30) ) & 0xFF = 0xE3

	//
	//	4.2.2 Query Sensor Module Parameters (Method 2)
	//	0xD7
	//	Same as [4.2.1 Query Sensor Module Parameters (Method 1)], with a diﬀerent instruction format. 
	//	Users can choose to use either method.
	//////////////////////////////////////
	// ISTO KAO TB200: 6.1 Command 1: 0xD7
	//////////////////////////////////////


	//	4.3 Query Current Gas Concentration
	const std::vector<uint8_t> cmd_read_gas_concentration = {
			//    0      |    1   |    2    |    3   |   4    |   5    |    6   |    7   |    8
			// Start bit | Retain | Command | Retain | Retain | Retain | Retain | Retain | Checksum
			     0xFF,       0x01,    0x86,     0x00,   0x00,    0x00,     0x00,    0x00,    0x79
	};
	const CmdStruct_t cmdReadGasConcentration = { cmd_read_gas_concentration, 9, true };
	// RETURN VALUE:
	//
	//	0xFF, 0x86	byte[0:1]	Fixed command header
	//	0x25, 0xBC	byte[2:3]	Concentration-2 (uint16).	0x25BC = 9660 -> the result is 9.66 mg/m³. See 4.2 Query Sensor Module Parameters
	//	0x03, 0xE8	byte[4:5]	Full range (uint16). 		0x03E8=1000 -> the result is 1000ppm. See 4.2 Query Sensor Module Parameters
	//	0x20, 0xD0	byte[6:7]	Concentration-1 (uint16). 	0x20D0=8400 -> the result is 8.4 ppm. See 4.2 Query Sensor Module Parameters
	//	0xBE		byte[8]		Checksum = ( -(0x86+0x25+0xBC+0x03+0xE8+0x20+0xD0) ) & 0xFF = 0xBE


	// 4.4 Query Gas Concentration, Temperature and Relative Humidity
	//
	// ISTO KAO COMMAND 2 za TB200




	//////////////////////////////////////////////
	// 4.5 Query Temperature and Relative Humidity
	// 4.6 Query Software Version
	// 4.7 Enter and Exit Sleep Mode (ima Metod 1 i Metod 2. Za TB200 metod 2 je isti)
	// 4.8 LED Control
	//////////////////////////////////////////////
	// ISTO KAO za TB200




	/////////////////////////
	/////////////////////////
	// 4.9 Sensor Calibration
	/////////////////////////
	/////////////////////////
	//
	//	A harsh environment or a long-term measurement will causes the sensor signal to diminish and measurement drift. 
	//	A baseline calibration (using zero gas, e.g. 0ppm) and a sensitivity calibration (using span gas, e.g. 10ppm) 
	//	are suggested to guarantee the measurement precision.
	//
	// NEMAMO USLOVE ZA OVO. NE DIRATI


	////////////////////////////////////////////
	//	4.10 Restore Factory Calibration Setting
	////////////////////////////////////////////
	//	TODO
	const std::vector<uint8_t> cmd_restore_factory_calibration = {
			//    0      |    1   |    2    |    3   |   4    |   5    |    6   |    7   |    8
			     0xFF,       0x01,    0x8E,     0x00,   0x00,    0x00,     0x00,    0x00,    0x71
	};
	const CmdStruct_t cmdRestoreFactoryCalibration_ONLY_TB600 = { cmd_restore_factory_calibration, 2, false };
	// RETURN VALUE:
	//
	//	0xFF, 0x86	byte[0:1]	Fixed format (ascii OK)

































	//	COMMAND 8
	//	Get the current temperature and humidity with calibration
	//	const std::vector<uint8_t> read_temp_and_humidity_with_calibration = {
	//			 // NE PISE KOMANDA?! (u dva razlicita datasheeta ne pise! Neko je zezno' copy-paste - prim. prev.)
	//	};
	// const CmdStruct_t readTempAndHumidityWithCalibration = { read_temp_and_humidity_with_calibration, 5};
	// RETURN VALUE:
	//
	//	0x0A	0	Temperature high 8 bit
	//	0x09	1	Temperature low 8 bit
	//	0x11	2	Humidity high 8 bit
	//	0xF4	3	Humidity low 8 bit
	//	0xE8	4	Checksum
	//
	// NOTE:
	// Checksum: 0 ~ 3 bits of data are added to generate an 8-bit data. Each bit is inverted, plus 1 at the end
	//
	// Temperature is signed data with two decimal places, the unit is (℃ -Celsius). Pseudo code calculation formula:
	// T = (float) ( (int)((0x0A<<8) | 0x09)) / 100
	//
	// Humidity is data without sign and two decimal places, the unit is (rh%). Pseudo code calculation formula:
	// Rh = (float) ( (uint)((0x0A<<8) | 0x09)) / 100		(valjda misli ((bit[2]<<8) | bit[3]) - prim. prev.)


	//	COMMAND 9
	//	Get the current version number
	//	const std::vector<uint8_t> read_version = {
	//			 // NE PISE KOMANDA?! (u dva razlicita datasheeta ne pise! Neko je zezno' copy-paste - prim. prev.)
	//	};
	// const CmdStruct_t readVersion = { read_version, 6};
	// RETURN VALUE:
	//
	//	0x19	0
	//	0x05	1
	//	0x27	2
	//	0x00	3
	//	0x10	4
	//	0x01	5
	// (Ne pise sta su, valjda ih treba concatenate kao stringove i to je kao fol verzija - prim. prev.)






	//////////////////////
	// LOW POWER SWITCHING	// TODO	- ovo cemo da implementiramo samo ako bude potrebno
	//////////////////////
	//
	// Enter sleep mode
	// ...
	// Exit sleep mode
	// ...
	// It takes 5 seconds to recover after exiting sleep mode, no data within 5 seconds








#endif /* GASSENSOR_TB200B_TB600BC_H_ */
