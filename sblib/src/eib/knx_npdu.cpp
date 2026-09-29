/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#include "sblib/eib/knx_npdu.h"
#include "sblib/eib/knx_lpdu.h"

constexpr uint8_t NPDU_EXTENDED_LENGTH_BYTE = 6;  //!< Length byte of an extended frame

constexpr uint8_t NPDU_STANDARD_HEADER_SIZE = 7;  //!< Protocol overhead of a standard frame, excluding the checksum
constexpr uint8_t NPDU_EXTENDED_HEADER_SIZE = 8;  //!< Protocol overhead of an extended frame, excluding the checksum

constexpr uint8_t MASK_STANDARD_LENGTH = 0x0F;

uint16_t telegramSize(const uint8_t* telegram)
{
    if (frameType(telegram) == FRAME_EXTENDED)
    {
        return NPDU_EXTENDED_HEADER_SIZE + telegram[NPDU_EXTENDED_LENGTH_BYTE];
    }

    return NPDU_STANDARD_HEADER_SIZE + (telegram[NPDU_CONTROL_BYTE] & MASK_STANDARD_LENGTH);
}
