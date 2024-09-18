/*
 * app.h
 *
 *  Created on: Jun 7, 2024
 *      Author: peca
 */

#ifndef APP_H_
#define APP_H_

// Error handling je neophodan
int CONSOLE_DEBUG = 2; 	// 0=silent operation; sve ostalo -> print na stdout, stderr...
enum ErrCodes_t {
	OK = 0,
	NOT_DEFINED,
	UART_NOT_AVAILABLE,
	WiringPi_NOT_AVAILABLE
};
ErrCodes_t H_STATUS = NOT_DEFINED;



int uartFileDescriptor;

/*
 *
 * UART - zvanicna dokumentacija
 * -----------------------------
 * https://www.raspberrypi.com/documentation/computers/configuration.html#configure-uarts
 * There are two types of UART available on the Raspberry Pi - PL011 and mini UART.
 * The PL011 is a capable, broadly 16550-compatible UART
 * Mini UART has a reduced feature set.
 *
 * Raspberry Pi Zero, 1, 2 and 3
 * The Raspberry Pi Zero, 1, 2, and 3 each contain two UARTs as follows:
 * Name 	Type
 * UART0	PL011
 * UART1	mini UART						<-- OVO --- !

 *
 * Raspberry Pi 4 Model B and 400 have an additional four PL011s, which are disabled by default:
 * Name 	Type
 * UART0	PL011
 * UART1	mini UART						<-- OVO --- !
 * UART2	PL011
 * UART3	PL011
 * UART4	PL011
 * UART5	PL011
 *
 * Raspberry Pi Zero W / Raspberry Pi Zero 2 W, Raspberry Pi 3, Raspberry Pi 4
 * Primary/console		= UART1					<-- OVO --- !
 * Secondary/Bluetooth	= UART0
 * ...za ostale - bas me briga
 *
 * Primary UART is present on GPIO 14 (tx) and 15 (rx)
 *
 * Linux devices on Raspberry Pi OS:
 * /dev/ttyS0		mini UART				<-- OVO --- !
 * /dev/ttyAMA0		first PL011 (UART0)
 * /dev/serial0		primary UART
 * /dev/serial1		secondary UART
 * /dev/ttyAMA10	Raspberry Pi 5 Debug UART
 *
 * /dev/serial0 and /dev/serial1 are symbolic links
 *
 * Due to changes in Bookworm, /dev/serial1 does not exist by default.
 * You can re-enable serial1 by setting the following values in config.txt:
 * dtparam=krnbt=off
 * NOTE: This option may not work on all models in the future. Only use this option if there is no other alternative for your use case.
 */

#define hwUart_ttyS0		"/dev/ttyS0"	// ala su ga zakomplikovali!! Ukratko, OVO je resenje za rpi Zero W.
#define hwUart_serial0		"/dev/serial0"	// u mom slucaju, simbolicki link na ttyS0


#endif /* APP_H_ */
