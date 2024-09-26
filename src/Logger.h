/*
 * Logger.h
 *
 *  Created on: Sep 26, 2024
 *      Author: peca
 */

#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_


class Logger {

public:
    Logger(int log_level);
    virtual ~Logger();

    static void setLogLevel();

    static void dhex(uint8_t i);
    static void dhex(uint16_t i);
    static void dhex(uint32_t i);

    static void dhex(std::vector<uint8_t> v);
    static void dhex(std::vector<uint16_t> v);
    static void dhex(std::vector<uint32_t> v);


    static void dbin(uint8_t i);
    static void dbin(uint16_t i);
    static void dbin(uint32_t i);

    static void dbin(std::vector<uint8_t> v);
    static void dbin(std::vector<uint16_t> v);
    static void dbin(std::vector<uint32_t> v);


private:
    int log_level;
    int msg_level;

};

#endif /* SRC_LOGGER_H_ */
