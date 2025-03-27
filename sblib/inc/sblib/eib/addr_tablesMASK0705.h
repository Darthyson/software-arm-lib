/*
 *  addr_tables.h - BCU communication address tables.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef SBLIB_KNX_ADDR_TABLES_MASK0705_H
#define SBLIB_KNX_ADDR_TABLES_MASK0705_H

#include <sblib/eib/addr_tablesMASK0701.h>

class MASK0705;
class MASK0701;

class AddrTablesMASK0705 : public AddrTablesMASK0701
{
public:
    explicit AddrTablesMASK0705(MASK0705* bcu) : AddrTablesMASK0701((MASK0701*)bcu) {}
};

#endif /* SBLIB_KNX_ADDR_TABLES_MASK0705_H */
