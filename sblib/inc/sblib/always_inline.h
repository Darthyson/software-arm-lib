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

/**
 * @def FORCE_INLINE
 * @brief Force a function to be inlined, even if the compiler would not do so by default.
 * This is more aggressive than ALWAYS_INLINE and may lead to larger code size.
 * source: https://meghprkh.github.io/blog/posts/c++-force-inline/
 */
#if defined(__clang__)
#   define FORCE_INLINE [[gnu::always_inline]] [[gnu::gnu_inline]] extern inline
#elif defined(__GNUC__)
#   define FORCE_INLINE [[gnu::always_inline]] inline
#elif defined(_MSC_VER)
#pragma warning(error: 4714)
#   define FORCE_INLINE __forceinline
#else
#   error Unsupported compiler
#endif

#endif /* SBLIB_ALWAYS_INLINE_H */
