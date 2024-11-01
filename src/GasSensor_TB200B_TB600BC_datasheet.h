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
	// (^1) UPDATE iz Lucas Schwarz email 2024-10-16 13:11
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
	//	Baud Rate  9600 
	//	Data Bit  8 bits
	//	Stop Bit  1 bit 
	//	Parity  None 


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
	//	1ppm=1000ppb, 1%vol.=10000ppm 
	//	1mg/m3=1000ug/m3 


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
	//	0x18	2	see above the sensor type table
	//	0x00	3	Byte[4:3] Gas Sensor Rage = (Byte[1]<<8)|Byte[2], unit here corresponds to Concentration 1 in Read-Out Measured Values table below
	//	0xC8	4	Byte[4:3] Gas Sensor Rage = (Byte[1]<<8)|Byte[2], unit here corresponds to Concentration 1 in Read-Out Measured Values table below
	//	0x02	5	Unit_type: Please see above the Gas Concentration type table
	//	0x01	6	bit[7:4] number of decimal places (see Decimal Places table); bit[3:0] (see Data of Type table )
	//	0x00	7	Reserved
	//	0x46	8	Checksum see below
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
	//	byte[1:0]	Fixed command header 0xFF, 0x87
	//	byte[3:2]	Concentration 2
	//					Concentration = (float) ( (Byte[2]<<8)|Byte[3] )
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
	//	byte[9:8]	Temperature Value，there are two decimal places. 
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
	const CmdStruct_t readTempAndHumidity = { read_firmware_version, 6, false};
	//	RETURN VALUE:
	//
	// Bukvalno pise 
	//
	// 软件版本号
	//
	// Da ne verujes hahahahaha



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



	////////////////////////////////
	// Transmission mode switching
	////////////////////////////////

	//	COMMAND 1
	//	Switches to active upload
	const std::vector<uint8_t> cmd_set_active_mode = {
			//    0      |    1   |       2        |       3       |   4    |   5    |    6   |    7   |    8
			// Start bit | Retain | Switch command | Active upload | Retain | Retain | Retain | Retain | Checksum
			     0xFF,       0x01,       0x78,            0x40,       0x00,    0x00,     0x00,    0x00,    0x47
	};
	const CmdStruct_t cmdSetActiveMode = { cmd_set_active_mode, 0, false };






	//	COMMAND 2
	//	Switches to passive upload
	const std::vector<uint8_t> cmd_set_passive_mode = {
			//    0      |    1   |       2        |    3   |   4    |   5    |    6   |    7   |    8
			// Start bit | Retain | Switch command | Answer | Retain | Retain | Retain | Retain | Checksum
			     0xFF,       0x01,       0x78,         0x41,   0x00,    0x00,     0x00,    0x00,    0x46
	};
	const CmdStruct_t cmdSetPassiveMode = { cmd_set_passive_mode, 0, false };



	////////////////////////////////
	// Commands in query mode
	////////////////////////////////

	//	COMMAND 3
	//	Get the sensor type, maximum range, unit, and decimal places: 0xD1
	const std::vector<uint8_t> cmd_get_type_range_unit_decimals_0xD1 = {
			0xD1
	};
	const CmdStruct_t cmdGetTypeRangeUnitDecimals0xd1 = { cmd_get_type_range_unit_decimals_0xD1, 9, true };
	// Return value:	(valjda su to vredosti za primer - prim.prev)
	//	0x18	0	Sensor type
	//	0x00	1	Maximum range high
	//	0xC8	2	Maximum range low
	//	0x02	3	Unit
	//	0x00	4	Retain
	//	0x00	5	Retain
	//	0x00	6	Retain
	//	0x01	7	Number of decimal places(bit[4]~bit[7] | Data sign (bit[0]~bit[3])
	//	0x35	8	Parity bit
	//
	//	NOTE:
	//	Max range = (Max range high << 8) | Max range low
	//	Unit: 0x02 -> (ppm and mg/m3) or 0x04 (ppb and ug/m3)
	//	Signs: 0 (positive) or 1 (negative)
	//	Decimal places: the maximum number of decimal places is 3 (three prim. prev.)
	//
	//




	//	COMMAND 5
	//	Actively reading the gas concentration
	const std::vector<uint8_t> cmd_read_gas_concentration = {
			//    0      |    1   |    2    |    3   |   4    |   5    |    6   |    7   |    8
			// Start bit | Retain | Command | Retain | Retain | Retain | Retain | Retain | Checksum
			     0xFF,       0x01,    0x86,     0x00,   0x00,    0x00,     0x00,    0x00,    0x79
	};
	const CmdStruct_t cmdReadGasConcentration = { cmd_read_gas_concentration, 9, true };
	// RETURN VALUE:
	//
	//	0xFF	0	Start bit
	//	0x86	1	Command
	//	0x00	2	High gas concentration (mg/m3)		// STOPSHIP NOTA (1**)
	//	0x2A	3	Low gas concentration (mg/m3)		// STOPSHIP NOTA (1**)
	//	0x00	4	Full range high
	//	0x00	5	Full range low
	//	0x00	6	High gas concentration (ppm)		// STOPSHIP NOTA (1**)
	//	0x20	7	Low gas concentration (ppm)			// STOPSHIP NOTA (1**)
	//	0x30	8	Checksum
	// NOTE:
	// Checksum: Add 1 ~ 7 digits of data to generate an 8-bit data, invert each bit, add 1 at the end
	// Gas concentration = (gas concentration high bit * 256 + gas concentration bit) / 10^x		// STOPSHIP NOTA (1**)
	// where "x"= number of decimal places. The number of decimal places can be read by D1 or D7 command.
	// (The high and low concentrations need to be converted from hexadecimal to decimal and then brought into this formula to calculate)





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





	/////////////////////////////////
	// DATA ACTIVE UPLOAD MODE FORMAT (zasto se ovo ponavlja kad gore vec pise?? - prim. prev.)
	/////////////////////////////////
	//
	//	0xFF	0	Start bit
	//	0x86	1	Command
	//	0x00	2	High gas concentration (mg/m3)		// STOPSHIP NOTA (1**)
	//	0x2A	3	Low gas concentration (mg/m3)		// STOPSHIP NOTA (1**)
	//	0x00	4	Full range high
	//	0x00	5	Full range low
	//	0x00	6	High gas concentration (ppm)		// STOPSHIP NOTA (1**)
	//	0x20	7	Low gas concentration (ppm)			// STOPSHIP NOTA (1**)
	//	0x30	8	Checksum
	//
	// NOTE:
	// Checksum: 1 ~ 11 bits of data are added to generate an 8-bit data, each bit is inverted, and 1 is added at the end. (ocigledno misli "add 1~8 bits - prim. prev)
	//
	// Gas concentration = gas concentration high bit * 256 + gas concentration bit) / 10^x		// STOPSHIP NOTA (1**)
	// where "x" = The number of decimal places; The number of decimal places can be read by D1 or D7 command
	// (The high and low concentrations need to be converted from hexadecimal to decimal and then brought into this formula to calculate)



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
