

1 Raspberry BOOT
================

Kad je pripremljena SD kartica putem rpi-imagera, ubaciti je u rpi i ukljuciti ga. Prvo startovanje ce trajati oko 3-4 minuta (rpi zero) a kasnije ce biti dosta brze, oko 40-ak sekundi. Strpljenje!

A zatim:

(win) cmd prompt -> 
$ ping zero.local (ime koje smo dali prilikom "OS customization")

Ako nema odgovora:
- konektuj se na ruter i probaj da vidis koju adresu je dobio rpi. Ako je uopste dobio.
- ako ne pomaze, onda u prethodnom fajlu videti notu kako se povezuje sistemska konzola na gpio header. Odatle moras sam da se snadjes.
- ako ni to ne pomaze, u maleru si :-(

Ako ima odgvora na ping:
- najbolje ako se odmah vidi sa koje adrese dolazi odgovor
- na mojem ruteru je je ping reply ipv6 pa ne znam ipv4 adresu

		ping zero.local
		Pinging zero.local [fe80::df2d:3518:3d4:c11a%19] with 32 bytes of data:
		Reply from fe80::df2d:3518:3d4:c11a%19: time=18ms

$ ssh zero.local -> potvrdi sta god trazi prilikom ssh konekcije
-> sada smo na raspberry-ju

Ako konekcija nije prihvacena, zaboravio si da kod "OS customization" ukljucis SSH servis!

Kad se logujes na raspberry:

peca@rpi$ ip address
-> pronaci ip adresu i, po potrebi, koristiti za ubuduce konektovanje


To je sve


2 Osnovna podesavanja
=====================

https://learn.sparkfun.com/tutorials/raspberry-gpio/c-wiringpi-setup

U verziji "Legacy 32-bit Lite" koju mi koristimo, potrebno je instalirati:
1 git

		sudo apt-get install git-core
		
2 WiringPi

		Provera:
		$ gpio
		
		Ako nije instaliran, onda 
		$ git clone https://github.com/WiringPi/WiringPi.git
		$ cd WiringPi/
		$ ./build
		
		Posle ovoga bi trebalo da radi:
		$ gpio -v
		$ gpio readall
		+-----+-----+---------+------+---+-Pi ZeroW-+---+------+---------+-----+-----+
		 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
		 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
		 |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
		 |   2 |   8 |   SDA.1 |   IN | 1 |  3 || 4  |   |      | 5v      |     |     |
		 |   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
		 |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 1 | IN   | TxD     | 15  | 14  |
		 |     |     |      0v |      |   |  9 || 10 | 1 | IN   | RxD     | 16  | 15  |
		 |  17 |   0 | GPIO. 0 |   IN | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
		 |  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
		 |  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
		 |     |     |    3.3v |      |   | 17 || 18 | 0 | IN   | GPIO. 5 | 5   | 24  |
		 |  10 |  12 |    MOSI |   IN | 0 | 19 || 20 |   |      | 0v      |     |     |
		 |   9 |  13 |    MISO |   IN | 0 | 21 || 22 | 0 | IN   | GPIO. 6 | 6   | 25  |
		 |  11 |  14 |    SCLK |   IN | 0 | 23 || 24 | 1 | IN   | CE0     | 10  | 8   |
		 |     |     |      0v |      |   | 25 || 26 | 1 | IN   | CE1     | 11  | 7   |
		 |   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
		 |   5 |  21 | GPIO.21 |   IN | 1 | 29 || 30 |   |      | 0v      |     |     |
		 |   6 |  22 | GPIO.22 |   IN | 1 | 31 || 32 | 0 | IN   | GPIO.26 | 26  | 12  |
		 |  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
		 |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
		 |  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
		 |     |     |      0v |      |   | 39 || 40 | 0 | IN   | GPIO.29 | 29  | 21  |
		 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
		 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
		 +-----+-----+---------+------+---+-Pi ZeroW-+---+------+---------+-----+-----+
		
2.1 Pinovi 14 i 15 (kolona BCM) su **predvidjeni** za uart ali trenutno povezani samo kao **obican gpio**
Da bi ih omogucili, potrebno je

		$ sudo raspi-config

Pa zatim ->  3 - Interface options
	-> I6 - Serial port
	-> Would you like a login shell to be accessible over serial? -> **NO**
	-> Would you like the serial port hardware to be enabled? -> **YES**
	
Potvrditi, izaci. Ako trazi reboot -> OK


	


