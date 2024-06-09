/*
 * UartMux.h
 *
 *  Created on: May 23, 2024
 *      Author: peca
 */

#ifndef SRC_UARTMUX_H_
#define SRC_UARTMUX_H_


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
// Adrese senzora
//      moj_naziv	S[1..8] (broj na plocici uart mux-a)
#define adr_CO 			3
#define adr_H2S			4
#define adr_O2			5
#define adr_itd			7
#define adr_DONOTUSE_8	8	// pocetno, izgleda da su svi pinovi floating high, tj adresa 0x111 iliti, po njegovoj numeraciji port 8


// rpi specific
/*
 * NAZIVI GPIO PINOVA
 * proveriti na rpi: $ gpio readall
 *
 +-----+-----+---------+------+---+---Pi 3---+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
 |   2 |   8 |   SDA.1 | ALT0 | 1 |  3 || 4  |   |      | 5v      |     |     |
 |   3 |   9 |   SCL.1 | ALT0 | 1 |  5 || 6  |   |      | 0v      |     |     |
 |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 0 | IN   | TxD     | 15  | 14  |
 |     |     |      0v |      |   |  9 || 10 | 1 | IN   | RxD     | 16  | 15  |
 |  17 |   0 | GPIO. 0 |   IN | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
 |  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
 |  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
 |     |     |    3.3v |      |   | 17 || 18 | 0 | IN   | GPIO. 5 | 5   | 24  |
 |  10 |  12 |    MOSI | ALT0 | 0 | 19 || 20 |   |      | 0v      |     |     |
 |   9 |  13 |    MISO | ALT0 | 0 | 21 || 22 | 0 | IN   | GPIO. 6 | 6   | 25  |
 |  11 |  14 |    SCLK | ALT0 | 0 | 23 || 24 | 1 | OUT  | CE0     | 10  | 8   |
 |     |     |      0v |      |   | 25 || 26 | 1 | OUT  | CE1     | 11  | 7   |
 |   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
 |   5 |  21 | GPIO.21 |   IN | 1 | 29 || 30 |   |      | 0v      |     |     |
 |   6 |  22 | GPIO.22 |   IN | 1 | 31 || 32 | 0 | IN   | GPIO.26 | 26  | 12  |
 |  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
 |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
 |  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
 |     |     |      0v |      |   | 39 || 40 | 0 | IN   | GPIO.29 | 29  | 21  |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+---Pi 3---+---+------+---------+-----+-----+
 *
 * wiringPiSetupGpio(); // Initializes wiringPi using the Broadcom GPIO pin numbers
 * wiringPiSetup();		// Initializes wiringPi using wiringPi's simlified number system.
 */
#define UartMuX_pinS1	 4	// gpio4
#define UartMuX_pinS2	 3	// gpio3
#define UartMuX_pinS3	 2	// gpio2

typedef unsigned int MuxAdr_t;

class UartMux {
public:
	UartMux();
	virtual ~UartMux();

	void setAddr(MuxAdr_t adresa);		// prozovi uart mux adresu
	MuxAdr_t getAddr();					// koja je trenutna adresa?

private:
	MuxAdr_t currentAddress;
};

#endif /* SRC_UARTMUX_H_ */
