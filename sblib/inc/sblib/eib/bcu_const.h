/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef SBLIB_KNX_BCU_CONST_H_
#define SBLIB_KNX_BCU_CONST_H_

#include <cstdint>

const uint16_t TelegramBufferSize = 23; //!> TL4, Tx/Rx KNX telegram buffer size in bytes to allocate

/**
 * BCU status bits for @ref status()
 * See BCU1 / BCU2 help for detailed description.
 */
enum BcuStatus
{
    BCU_STATUS_PROGRAMMING_MODE  = 0x01, //!< Programming mode: 0=normal mode, 1=programming mode
    BCU_STATUS_LINK_LAYER        = 0x02, //!< Link layer mode (1), or bus monitor mode (0)
    BCU_STATUS_TRANSPORT_LAYER   = 0x04, //!< If bit is set, transport layer is enabled
    BCU_STATUS_APPLICATION_LAYER = 0x08, //!< If bit is set, application layer is enabled
    BCU_STATUS_SERIAL_PEI        = 0x10, //!< If bit is set, serial PEI is enabled
    BCU_STATUS_USER_MODE         = 0x20, //!< If bit is set, application layer/program is enabled
    BCU_STATUS_DOWNLOAD_MODE     = 0x40, //!< If bit is set, BCU is in download mode, else normal operation
    BCU_STATUS_PARITY            = 0x80, //!< Parity bit: even parity for bits 0..6
};

#endif /* SBLIB_KNX_BCU_CONST_H_ */

