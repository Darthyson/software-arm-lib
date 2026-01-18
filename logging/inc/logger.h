//
// Created by Mario Theodoridis on 21.05.20.
//

#ifndef SBLIB_LOGGER_H
#define SBLIB_LOGGER_H


#include <sblib/ioports.h>

void initLogger(uint32_t txPin = PIO1_7, uint32_t rxPin = PIO1_6);
void serPrintf(const char *fmt, ...);

#endif /* SBLIB_LOGGER_H */
