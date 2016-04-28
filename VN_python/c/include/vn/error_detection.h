/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
/** \file
* {COMMON_HEADER}
*
* \section Description
* This header file defines types and functions for error-detection capabilities
* within the VectorNav C Library.
*/
#ifndef _VNERROR_DETECTION_H_
#define _VNERROR_DETECTION_H_

#include "vn/int.h"
#include "vn/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Enumeration of the various error-detection algorithms used by the
 * library
 */
typedef enum
{
	/** Signifies no error-detection should be performed. */
	VNERRORDETECTIONMODE_NONE,

	/** Signifies to use 8-bit XOR checksum. */
	VNERRORDETECTIONMODE_CHECKSUM,

	/** Signifies to use CRC16-CCITT algorithm. */
	VNERRORDETECTIONMODE_CRC

} VnErrorDetectionMode;

/** \brief Computes the 8-bit XOR checksum of the provided data.
 *
 * \param[in] data Pointer to the start of data to perform the checksum of.
 * \param[in] length The number of bytes to include in the checksum.
 * \return The computed checksum.
 */
uint8_t VnChecksum8_compute(char const *data, size_t length);

/** \brief Computes the 16-bit CRC16-CCITT of the provided data.
 *
 * \param[in] data Pointer to the start of data to perform the CRC of.
 * \param[in] length The number of bytes to include in the CRC.
 * \return The computed CRC.
 */
uint16_t VnCrc16_compute(char const *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif
