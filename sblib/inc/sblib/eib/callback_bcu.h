/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef SBLIB_EIB_CALLBACK_BCU_H_
#define SBLIB_EIB_CALLBACK_BCU_H_

#include <cstdint>
#include <sblib/eib/callback_bus.h>
#include <sblib/eib/bcu_base.h>

class BcuBase;

class CallbackBcu : public CallbackBus
{
public:
    CallbackBcu(BcuBase* bcu);
    CallbackBcu() = delete;

    void finishedSendingTelegram(bool successful) override;
    const virtual uint8_t getLayerStatus() const override;

private:
    BcuBase* bcu;
};

#endif /* SBLIB_EIB_CALLBACK_BCU_H_ */
