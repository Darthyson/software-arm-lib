/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef SBLIB_EIB_CALLBACK_BUS_H_
#define SBLIB_EIB_CALLBACK_BUS_H_

#include <cstdint>

class CallbackBus
{
public:
    virtual void finishedSendingTelegram(bool successful) = 0;
    const virtual uint8_t getLayerStatus() const = 0;
};


#endif /* SBLIB_EIB_CALLBACK_BUS_H_ */
