/*
 *  utils.h - Utility functions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef SBLIB_UTILS_H_
#define SBLIB_UTILS_H_

#include <sblib/types.h>
#include <sblib/libconfig.h> // Do not delete this line! We need it for the below debug macros, e.g. DUMP_MEM_OPS

#define HIGH_BYTE(x) ((uint8_t)(x >> 8))  ///\todo create new macro "secondByte" in bits.h

/**
 * Copy from src to dest with reversing the byte order.
 *
 * @param dest - the destination to copy to
 * @param src - the source to copy from
 * @param len - the number of bytes to copy
 */
void reverseCopy(byte* dest, const byte* src, int len);

/**
 * Call when a fatal application error happens. This function will never
 * return and the program/fatalError LED (can be changed with setFatalErrorPin) will blink rapidly to indicate the error.
 */
void fatalError();

/**
 * Set the Pin for fatalError()
 * @param newPin - the new Pin
 */
void setFatalErrorPin(int newPin);

/**
 * Set the KNX Tx-Pin for fatalError() and a hardware fault
 * @param newTxPin - the new KNX-Tx Pin
 */
void setKNX_TX_Pin(int newTxPin);

/**
 * Creates a len_hash wide hash of the uid.
 * Hash will be generated in provided hash buffer
 *
 * @param uid - LPC-serial (128bit GUID) returned by iapReadUID() which will be hashed
 * @param len_uid - size of uid  (normally 16 byte)
 * @param hash - buffer for generated hash
 * @param len_hash - size of provided hash buffer (normally 6byte/48bit for EIB)
 * @return True if hash successfully created, false if not.
 */
int hashUID(byte* uid, const int len_uid, byte* hash, const int len_hash);

/**
 * Include the C++ code snippet if DEBUG is defined, do not include the code
 * if DEBUG is not defined.
 *
 * @param code - the C++ code to include
 *
 * @note Example:  IF_DEBUG(fatalError())
 */
#ifdef DEBUG
#  define IF_DEBUG(code) { code; }
#else
#  define IF_DEBUG(code)
#endif

// Enable informational debug statements
#if defined(DUMP_MEM_OPS)
#   define DB_MEM_OPS(x) IF_DEBUG(x)
#else
#   define DB_MEM_OPS(x)
#endif

/**
 * Include the C++ code snippet if DUMP_PROPERTIES is defined, do not include the code
 * if DUMP_PROPERTIES is not defined.
 *
 * @param code - the C++ code to include
 *
 * @note Example:  DB_PROPERTIES(fatalError())
 */
#if defined(DUMP_PROPERTIES)
#   define DB_PROPERTIES(code) IF_DEBUG(code)
#else
#   define DB_PROPERTIES(code)
#endif

#if defined(DUMP_COM_OBJ)
#   define DB_COM_OBJ(x) x
#else
#   define DB_COM_OBJ(x)
#endif

#if defined(DUMP_TELEGRAMS)
#   define DB_TELEGRAM(x) x
#else
#   define DB_TELEGRAM(x)
#endif

// Informational bus debug statements
#if defined (DEBUG_BUS) || defined (DEBUG_BUS_BITLEVEL) || defined(DUMP_TELEGRAMS)
#   define DB_BUS(x) x
#else
#   define DB_BUS(x)
#endif

#if !defined(INCLUDE_SERIAL)
#   undef DB_MEM_OPS
#   define DB_MEM_OPS(x)
#   undef DB_PROPERTIES
#   define DB_PROPERTIES(x)
#   undef DB_COM_OBJ
#   define DB_COM_OBJ(x)
#   undef DB_TELEGRAM
#   define DB_TELEGRAM(x)
#   undef DB_BUS
#   define DB_BUS(x)
#endif

/**
 * Interrupt service routine to override the default HardFault exception handler in cr_startup_lpc11xx.cpp
 * This function will never return and the program/fatalError LED
 * will blink rapidly to indicate the error.
 */
extern "C" void HardFault_Handler();

#endif /* SBLIB_UTILS_H_ */
