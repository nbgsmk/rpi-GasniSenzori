/*
 * Blinkovi.h
 *
 *  Created on: May 23, 2024
 *      Author: peca
 */

#ifndef SRC_BLINKOVI_H_
#define SRC_BLINKOVI_H_

class Blinkovi {
public:
	Blinkovi();
	virtual ~Blinkovi();

	void ledon(void);
	void ledoff(void);
	void ledtogl(void);

	void trep(uint32_t ticks_on, uint32_t ticks_off);
	void trepCnt(uint32_t count, uint32_t ticks_on, uint32_t ticks_off);
	void trepCntPer(uint32_t count, uint32_t ticks_on, uint32_t ticks_off, uint32_t period);

	uint32_t millisecondsToTicks(uint32_t);
};

#endif /* SRC_BLINKOVI_H_ */
