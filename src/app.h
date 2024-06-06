/*
 * app.h
 *
 *  Created on: Jun 6, 2024
 *      Author: peca
 */

#ifndef APP_H_
#define APP_H_

#include <iostream>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <vector>

void HAL_Delay(unsigned int delay_mS){
	usleep(delay_mS * 1000);
};

#endif /* APP_H_ */
