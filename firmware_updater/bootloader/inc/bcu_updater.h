/**************************************************************************//**
 * @addtogroup SBLIB_BOOTLOADER Selfbus Bootloader
 * @defgroup SBLIB_BOOTLOADER_BCU Bus coupling unit (BCU)
 * @ingroup SBLIB_BOOTLOADER
 * @brief    Bus coupling unit (BCU)
 * @details
 *
 * @{
 *
 * @file   bcu_updater.h
 * @author Martin Glueck <martin@mangari.org> Copyright (c) 2015
 * @author Stefan Haller Copyright (c) 2021
 * @author Darthyson <darth@maptrack.de> Copyright (c) 2022
 * @bug No known bugs.
 ******************************************************************************/

/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 -----------------------------------------------------------------------------*/

#ifndef BCU_UPDATER_H_
#define BCU_UPDATER_H_

#include <sblib/eib/bcu_base.h>
#include "update.h"

class BcuUpdate: public BcuBase
{
public:
    BcuUpdate();
    ~BcuUpdate() = default;
    using BcuBase::setProgrammingMode; // make it public so we can use it in bootloader.cpp
    void begin();
    bool applicationRunning() const override {return (enabled);}

protected:
    bool processApci(ApciCommand apciCmd, unsigned char * telegram, uint8_t telLength, uint8_t * sendBuffer) override;
    bool processGroupAddressTelegram(ApciCommand apciCmd, uint16_t groupAddress, unsigned char *telegram, uint8_t telLength) override;
    bool processBroadCastTelegram(ApciCommand apciCmd, unsigned char *telegram, uint8_t telLength) override;

    uint8_t& layerStatus() override;
private:
    uint8_t bcuStatus = BCU_STATUS_LINK_LAYER | BCU_STATUS_TRANSPORT_LAYER | BCU_STATUS_APPLICATION_LAYER | BCU_STATUS_USER_MODE;
};

#endif /* BCU_UPDATER_H_ */

/** @}*/
