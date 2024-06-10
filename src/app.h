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
#define hwUart_serial0		"/dev/serial0"	// Postoji na Rpi Zero W. NE POSTOJI na rpi4
#define hwUart_ttyS0		"/dev/ttyS0"	// Postoji na Rpi Zero W. NE POSTOJI na rpi4
#define hwUart_AMA0			"/dev/ttyAMA0"	// NE KORISTITI! Izgleda da je za bluetooth. Postoji na Rpi Zero W, kao i na rpi4.


#endif /* APP_H_ */
