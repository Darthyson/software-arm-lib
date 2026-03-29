/*
 *  The ALWAYS_INLINE macro to ensure that GNU gcc always inlines.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef SBLIB_ALWAYS_INLINE_H
#define SBLIB_ALWAYS_INLINE_H

/**
 * Declare a function as always inline
 */
#if defined ( __GNUC__ )
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#define ALWAYS_INLINE inline
#endif

#endif /* SBLIB_ALWAYS_INLINE_H */
