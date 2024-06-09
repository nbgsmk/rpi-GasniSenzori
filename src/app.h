/*
 * app.h
 *
 *  Created on: Jun 7, 2024
 *      Author: peca
 */

#ifndef APP_H_
#define APP_H_


int uartFileDescriptor;

/*
 * rpi 4:
 * zovu se /dev/tty0, tty1 ...
 * /dev/serial0		ne postoji
 * /dev/ttyS0		ne postoji
 *
 * rpi zero:
 * /dev/serial0	symlink-> /dev/ttyS0
 * /dev/serial1	symlink-> /dev/ttyAMA0
 */
#define hwUart			"/dev/serial0"	// rpi4 = NE POSTOJI


#endif /* APP_H_ */
