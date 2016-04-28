/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
/** \file
* {COMMON_HEADER}
*
* \section Description
* This header file provides common integer types compatible with the C99
* stdint.h header.
*/
#ifndef _VNINT_H_
#define _VNINT_H_

/* Visual Studio 2008 and earlier do not include the stdint.h header file. */
#if defined(_MSC_VER) && _MSC_VER <= 1500

typedef signed __int8		int8_t;
typedef signed __int16		int16_t;
typedef signed __int32		int32_t;
typedef signed __int64		int64_t;
typedef unsigned __int8		uint8_t;
typedef unsigned __int16	uint16_t;
typedef unsigned __int32	uint32_t;
typedef unsigned __int64	uint64_t;

#else

	/* Just include the standard header file for integer types. */
	#include <stdint.h>

#endif

#endif
