/*
 * Logger.cpp
 *
 *  Created on: Sep 26, 2024
 *      Author: peca
 */

using namespace std;

// C++
#include <bitset>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

#include "Logger.h"


Logger::Logger(int log_level) {
	this->log_level = log_level;
}

Logger::~Logger() {

}

////////////// HEX variable
void Logger::dhex(uint8_t x){	cout << "   0x" << setfill('0') << setw(2) << hex << static_cast<int>( x ) << "  " << std::dec; }
void Logger::dhex(uint16_t x){	cout << "   0x" << setfill('0') << setw(4) << hex << static_cast<int>( x ) << "  " << std::dec; }
void Logger::dhex(uint32_t x){	cout << "   0x" << setfill('0') << setw(8) << hex << static_cast<int>( x ) << "  " << std::dec; }


////////////// HEX vector
void Logger::dhex(vector<uint8_t> v){
    for (int i = 0; i < v.size(); i++) {
		dhex(v.at(i));
	}
}
void Logger::dhex(vector<uint16_t> v){
    for (int i = 0; i < v.size(); i++) {
		dhex(v.at(i));
	}
}
void Logger::dhex(vector<uint32_t> v){
    for (int i = 0; i < v.size(); i++) {
		dhex(v.at(i));
	}
}




////////////////// BIN variable
void Logger::dbin(uint8_t x) { std::cout << " " << bitset<8>( x ); }
void Logger::dbin(uint16_t x){ std::cout << " " << bitset<16>( x ); }
void Logger::dbin(uint32_t x){ std::cout << " " << bitset<32>( x ); }


////////////////// BIN vector
void Logger::dbin(vector<uint8_t> v){
	for (int i = 0; i < v.size(); i++) {
		dbin(v.at(i));
	}
	cout << endl;	
}
void Logger::dbin(vector<uint16_t> v){
	for (int i = 0; i < v.size(); i++) {
		dbin(v.at(i));
	}
	cout << endl;	
}
void Logger::dbin(vector<uint32_t> v){
	for (int i = 0; i < v.size(); i++) {
		dbin(v.at(i));
	}
	cout << endl;	
}






