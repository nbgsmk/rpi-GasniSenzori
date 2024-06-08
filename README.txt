

http://www.faqs.org/docs/Linux-HOWTO/Serial-Programming-HOWTO.html

https://raspberrypi.stackexchange.com/questions/57906/uart-interrupt-in-raspberry-pi-with-c-or-c

https://github.com/WiringPi/WiringPi/blob/master/wiringPi/wiringSerial.c

https://man7.org/linux/man-pages/man2/select.2.html

-----------------

https://www.youtube.com/results?search_query=cross+compile+wiringpi+on+laptop

https://www.youtube.com/watch?v=1Bs-bD4YSIU

https://www.youtube.com/watch?v=ykbr6YVww8I

https://www.youtube.com/watch?v=HxNHlhv74tA


--------------------------------------

WiringPI 

SETUP, INSTALL, GPIO
--------------------

https://learn.sparkfun.com/tutorials/raspberry-gpio/gpio-pinout
https://roboticsbackend.com/introduction-to-wiringpi-for-raspberry-pi/

REPO: https://github.com/WiringPi/WiringPi


--- Install WiringPi on Linux (general)
Terminal -> folder gde zelimo da instaliramo WirinPi
cd ___x___
sudo apt-get purge wiringpi
git clone https://github.com/WiringPi/WiringPi.git
cd WiringPi
./build

This will get the source code from git and build WiringPi

Check if WiringPi is correctly installed
$ gpio -v

On generic linux, the output will be
------------------------------------
Oops: Unable to determine Raspberry Pi board revision from /proc/device-tree/system/linux,revision and from /proc/cpuinfo
      WiringPi    : 3.6
      system name : Linux
      release     : 6.5.0-35-generic
      version     : #35-Ubuntu SMP PREEMPT_DYNAMIC Fri Apr 26 11:23:57 UTC 2024
      machine     : x86_64
 -> This is not an ARM architecture; it cannot be a Raspberry Pi.
...

On raspberry
$ gpio readall
+-----+-----+---------+------+---+---Pi 3---+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
 |   2 |   8 |   SDA.1 | ALT0 | 1 |  3 || 4  |   |      | 5v      |     |     |
 |   3 |   9 |   SCL.1 | ALT0 | 1 |  5 || 6  |   |      | 0v      |     |     |
 |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 0 | IN   | TxD     | 15  | 14  |
...

Finally add the libratry reference to your project's:
- compiler
- linker

This step is IDE dependent.
If compiling from command line add flag -lWiringPi
$ g++ myapp.cpp -lWiringPi



CROSS COMPILATION TOOLCHAIN
---------------------------

development machine:

check sudo privileges
$ visudo
...tbd


update sources list
$ sudo su -
# cd /etc/apt/sources.list.d
# nano crostools.list
-> dodati ovo
deb http://emdebian.org/tools/debian jessie main
-> snimiti

# apt-get install curl
# apt-get install build-essential
# curl http://emdebian.org/tools/debian/embedded-toolchain-archive.key | sudo apt-key add -
# exit
-> back to regular user

Na rpi proverimo koja je cpu arhitektura
$ ssh rpi.local
...password
$ uname -a
$ uname -m
aarch64
$ dpkg --print-architecture
arm64


Vratimo se nazad na development masinu: 
koji native toolchain imamo? 
$ dpkg --print-architecture
amd64 	- to je intel/amd pc

koje cross compile toolchainove imamo?
$ dpkg --print-foreign-architectures
i386	- valjda intel/amd
armhf	- arm hard float

$ sudo apt-get update

Dodati toolchain za arhitekturu koja je prikazana na rpi
(u nasem slucaju arm64 - ono sto je prikazao rpi)
$ sudo apt-get install crossbuild-essential-arm64 - 
ili 
$ sudo apt-get install crossbuild-essential-armhf

dpkg --add-architecture armhf

To je to!
Test

napraviti test.cpp

kompajlirati za intel/amd
$ g++ test.cpp -o amd.out
$ ./amd.out
-treba da radi

kompajlirati za arm
$ arm-linux-gnueabihf-g++ test.cpp -o aeabihf.out
$ arm-linux-gnueabihf-g++ test.cpp -o aeabihf_stat.out -static

dobiju se dinamicki i staticki linkovani exec fajlovi

ako se pokrene bilo koji na amd/intel masini, treba da odgovori 
-bash: ./aeabihf.out: cannot execute binary file: Exec format error

prebaciti oba na rpi i pokrenuti, ali sada, na rpi:
amd.out - daje exec format error
ali zato
aeabihf... - treba da radi

Ako mu se ne dopada dinamicki linkovan fajl, odgovor je npr 
-bash: ./aeabihf_stat.out: cannot execute: required file not found









Ko je procitao do ovde, svaka mu cast. 
Za nagradu, neka pogleda https://www.youtube.com/watch?v=T9yFyWsyyGk
Tamo sve lepo pise.

REMOTE DEBUGGING
----------------

Pratiti pomenuti klip od oko 20:30 pa do kraja!

rpi:
$ sudo apt-get install gdbserver

developer:
$ sudo apt-get install gdb-multiarch
$ cd __gde je eclipse workspace__

Napraviti fajl .gdbinit i upisati set architecture arm
$ echo "set architecture arm" >> .gdbinit



