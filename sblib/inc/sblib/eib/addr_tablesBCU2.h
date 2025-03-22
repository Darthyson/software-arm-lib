/*
 *  addr_tables.h - BCU communication address tables.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_addr_tables_BCU2_h
#define sblib_addr_tables_BCU2_h

#include <sblib/eib/addr_tables.h>

class BCU2;

class AddrTablesBCU2 : public AddrTables ///\todo derive from AddrTablesBCU1, to get rid of indexOfAddr(int addr)
{
public:
    AddrTablesBCU2(BCU2* bcuInstance) : bcu(bcuInstance) {};
    ~AddrTablesBCU2() = default;

    /**
     * Get the index of a group address in the address table.
     *
     * @param addr - the address to find.
     * @return The index of the address, -1 if not found.
     *
     * @brief The address table contains the configured group addresses and our
     * own physical address. This function skips the own physical address and
     * only scans the group addresses.
     */
    int indexOfAddr(int addr) override;

    /**
     * Get the address table. The address table contains the configured group addresses
     * and our own physical address.
     *
     * @return The pointer to the address table.
     *
     * @brief The first byte of the table contains the number of entries. The rest of
     * the table consists of the addresses: 2 bytes per address.
     */
    byte* addrTable() override;

    /**
     * Get the association table. The association table connects group addresses
     * with communication objects.
     *
     * @return The pointer to the association table.
     *
     * @brief The first byte of the table contains the number of entries. The rest of
     * the table consists of the associations - 2 bytes per association:
     * 1 byte addr-table index, 1 byte com-object number.
     */
    byte* assocTable() override;

    /**
     * Get total number of address entries
     * @return Total number of address entries including own address
     */
    uint16_t addrCount() override;

private:
    BCU2* bcu;
};

#endif /*sblib_addr_tables_BCU2_h*/
