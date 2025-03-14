/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include <sblib/eib/callback_bcu.h>

CallbackBcu::CallbackBcu(BcuBase* aBcu)
: bcu(aBcu)
{

}

void CallbackBcu::finishedSendingTelegram(bool successful)
{
    bcu->finishedSendingTelegram(successful);
}

const uint8_t CallbackBcu::getLayerStatus() const
{
    return bcu->layerStatus();
}
