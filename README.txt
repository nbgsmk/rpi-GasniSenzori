

UART
====

general uart on linux, sync and async handling etc
http://www.faqs.org/docs/Linux-HOWTO/Serial-Programming-HOWTO.html

uart rx / tx with separate threads
https://raspberrypi.stackexchange.com/questions/57906/uart-interrupt-in-raspberry-pi-with-c-or-c

wiringPi on github
https://github.com/WiringPi/WiringPi/blob/master/wiringPi/wiringSerial.c


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



CROSS COMPILATION TOOLCHAIN
---------------------------

++++++++++++++++++++
TODO add external library
https://www.eclipse.org/forums/index.php/t/238028/
https://www.eclipse.org/forums/index.php?t=msg&th=238028&goto=721572&#msg_721572
++++++++++++++++++++

pogledati ovo:
https://www.gurucoding.com/eclipse_as_ide_for_raspberry_pi/04_cross_compilation_setup/

3rd party libraries!!
https://www.gurucoding.com/eclipse_as_ide_for_raspberry_pi/08_adding_third_party_libraries/

=====================
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

https://gnutoolchains.com/raspberry/



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





-----------

temperatura i napajanje

$ vcgencmd get_throttled
  https://www.raspberrypi.com/documentation/computers/os.html#vcgencmd
  
 ALL OK
throttled=0x0 
  

Bit Hex value   Meaning
0          1    Under-voltage detected
1          2    Arm frequency capped
2          4    Currently throttled
3          8    Soft temperature limit active
16     10000    Under-voltage has occurred
17     20000    Arm frequency capping has occurred
18     40000    Throttling has occurred
19     80000    Soft temperature limit has occurred  
  
  
  
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

unpack a .deb package and use its contents for linking in Eclipse CDT

dpkg-deb -x <package_name>.deb <output_directory>

Typically, libraries are located in 	usr/lib 	or 	usr/local/lib, 
and headers are in 						usr/include or 	usr/local/include

Eclipse:

Include Paths:
	Project Properties > C/C++ General > Paths and Symbols -> add path to the header files extracted from the .deb package

Library Paths:
	Libraries tab -> add the path to the libraries extracted from the .deb package.

Library Names:
	Names of the libraries to link against (without the lib prefix and .so suffix)

EXAMPLE:
	Extracted 
	Libraries: example_contents/usr/lib
	Headers: example_contents/usr/include
	
	Configure eclipse:
	Include Paths: example_contents/usr/include.
	Library Paths: example_contents/usr/lib.
	Library Names: If you have libexample.so, add example










  
