/*
 *  properties.h - BCU 2 properties of EIB objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_properties_systemb_h
#define sblib_properties_systemb_h

#include <sblib/eib/propertiesMASK0701.h>

class MASK0701;
class SYSTEMB;

class PropertiesSYSTEMB : public PropertiesMASK0701
{
public:
    explicit PropertiesSYSTEMB(SYSTEMB* bcuInstance) : PropertiesMASK0701((MASK0701*)bcuInstance), bcu(bcuInstance) {}

    LoadState handleAllocAbsDataSegment(const int objectIdx, const byte* payLoad, const int len) override;
    LoadState handleDataRelativeAllocation(const int objectIdx, const byte* payLoad, const int len) override;
    virtual uint16_t crc16(uint8_t* ptr, int len);
    int loadProperty(int objectIdx, const byte* data, int len) override;
    bool propertyValueReadTelegram(int objectIdx, PropertyID propertyId, int count, int start, uint8_t* sendBuffer) override;
    bool propertyValueWriteTelegram(int objectIdx, PropertyID propertyId, int count, int start, uint8_t* sendBuffer) override;

private:
    SYSTEMB* bcu;
};
#endif /*sblib_properties_systemb_h*/
