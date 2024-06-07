/*
 * Mux.h
 *
 *  Created on: May 23, 2024
 *      Author: peca
 */

#ifndef SRC_UARTMUX_H_
#define SRC_UARTMUX_H_

#include <stdint.h>

///////////////////////
// UART MUX
// AtlasScientific
// https://files.atlas-scientific.com/serial_port_expander_datasheet.pdf
//
// s3-s1	port
// 0 0 0	1		adresa(000) => port(1) Grr!!
// 0 0 1	2
// 0 1 0	3
// 0 1 1	4
// 1 0 0	5
// 1 0 1	6
// 1 1 0	7
// 1 1 1	8
//
///////////////////////
#define adr_CO 		2
#define adr_H2S		4
#define adr_O2		5
#define adr_itd		7
#define adr_NOUSE_8	8

// rpi specific
#define UartMuX_pinS1	 4	// gpio23 = wiringPi 4
#define UartMuX_pinS2	 5	// gpio24 = wiringPi 4
#define UartMuX_pinS3	 6	// gpio25 = wiringPi 4
#define hwUart			"/dev/ttyAMA0"

typedef uint8_t MuxAdr_t;

class UartMux {
public:
	UartMux();
	virtual ~UartMux();

	MuxAdr_t currentAddress;
	int fileDescriptor;

	void setAdr(MuxAdr_t adresa);		// prozovi uart mux adresu
	MuxAdr_t getAdr();					// koja je trenutna adresa?

	void uartSend(int fileDescriptor, char chars[]);


private:


};

#endif /* SRC_UARTMUX_H_ */
