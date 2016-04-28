/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
#ifndef _VNERROR_H_
#define _VNERROR_H_
/** \file
* {COMMON_HEADER}
*
* \section Description
* This header file defines the types and error codes used within the VectorNav
* C Library.
*/

/** Enumeration of the various error messages used by the library. */
typedef enum
{
	/** Indicates there were no errors encountered. */
	E_NONE,

	/** Indicates an unknown error occurred. */
	E_UNKNOWN,

	/** Indicates a provided buffer was too small to complete the action. */
	E_BUFFER_TOO_SMALL,

	/** Indicates a provided value is not valid. */
	E_INVALID_VALUE,

	/** Indicates the requested functionality is currently not implemented. */
	E_NOT_IMPLEMENTED,

	/** Indicates the requested functionality is not supported. */
	E_NOT_SUPPORTED,

	/** Indicates the requested item was not found. */
	E_NOT_FOUND,

	/** Indicates the operation timed out. */
	E_TIMEOUT,

	/** Indicates insufficient permission to perform the operation. */
	E_PERMISSION_DENIED,

	/** Indicates an invalid operation was attempted. */
	E_INVALID_OPERATION,

	/** Indicates an event was signaled. */
	E_SIGNALED,

    /** Indicates either not enough memory is available or no memory was allocated */
    E_MEMORY_NOT_ALLOCATED,

	/** VectorNav sensor hard fault (Code 1). */
	E_SENSOR_HARD_FAULT = 1001,

	/** VectorNav sensor serial buffer overflow (Code 2). */
	E_SENSOR_SERIAL_BUFFER_OVERFLOW = 1002,

	/** VectorNav sensor invalid checksum (Code 3). */
	E_SENSOR_INVALID_CHECKSUM = 1003,

	/** VectorNav sensor invalid command (Code 4). */
	E_SENSOR_INVALID_COMMAND = 1004,

	/** VectorNav sensor not enough parameters (Code 5). */
	E_SENSOR_NOT_ENOUGH_PARAMETERS = 1005,

	/** VectorNav sensor too many parameters (Code 6). */
	E_SENSOR_TOO_MANY_PARAMETERS = 1006,

	/** VectorNav sensor invalid parameter (Code 7). */
	E_SENSOR_INVALID_PARAMETER = 1007,

	/** VectorNav sensor invalid register (Code 8). */
	E_SENSOR_INVALID_REGISTER = 1008,

	/** VectorNav sensor unauthorized access (Code 9). */
	E_SENSOR_UNAUTHORIZED_ACCESS = 1009,

	/** VectorNav sensor watchdog reset (Code 10). */
	E_SENSOR_WATCHDOG_RESET = 1010,

	/** VectorNav sensor output buffer overflow (Code 11). */
	E_SENSOR_OUTPUT_BUFFER_OVERFLOW = 1011,

	/** VectorNav sensor insufficient baud rate (Code 12). */
	E_SENSOR_INSUFFICIENT_BAUD_RATE = 1012,

	/** VectorNav sensor error buffer overflow (Code 13). */
	E_SENSOR_ERROR_BUFFER_OVERFLOW = 1013

} VnError;

/** \brief Converts a VnError enum into a string.
 *
 * \param[out] out The buffer to place the string in.
 * \param[in] val The VnError value to convert to string. */
void strFromVnError(char* out, VnError val);

#endif
