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

#define CO_ADR 		3
#define H2S_ADR		5
#define O2_ADR		7
#define itd_adr		8

typedef uint8_t MuxAdr_t;

class UartMux {
public:
	UartMux();
	virtual ~UartMux();

	MuxAdr_t currentAddress;

	void setAdr(MuxAdr_t adresa);		// prozovi uart mux adresu
	MuxAdr_t getAdr();					// koja je trenutna adresa?

private:


};

#endif /* SRC_UARTMUX_H_ */
