/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
/** \file
* {COMMON_HEADER}
*
* \section Description
* This header file provides common types for the VectorNav library.
*/
#ifndef _VNTYPES_H_
#define _VNTYPES_H_

#if !defined(__cplusplus)

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || defined(__GNUC__)

    #include <stddef.h>

#else

	/* Must not have C99. */

	/** Backup definition of size_t. */
	typedef unsigned int size_t;

#endif

#endif

#endif
