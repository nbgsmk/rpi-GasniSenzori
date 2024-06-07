

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





