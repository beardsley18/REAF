/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
/** \file
 * {COMMON_HEADER}
 *
 * \section DESCRIPTION
 * This header file contains structures and functions useful for cross-platform
 * development.
 */
#ifndef _VNXPLAT_H_
#define _VNXPLAT_H_

#include "vn/int.h"
#include "vn/error.h"
#include "vn/bool.h"

#ifdef _WIN32

	/* Disable some warnings for Visual Studio with -Wall. */
	#if defined(_MSC_VER)
		#pragma warning(push)
		#pragma warning(disable:4668)
		#pragma warning(disable:4820)
		#pragma warning(disable:4255)
	#endif

	#include <Windows.h>

	#if defined(_MSC_VER)
		#pragma warning(pop)
	#endif

#endif

#if defined(__linux__) || defined(__QNXNTO__)
	#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#elif defined(__linux__) || defined(__QNXNTO__)
	typedef	union
	{
		pthread_mutex_t		mutexHandle;
		void*				conditionAndMutexStruct;
	} VN_HANDLE;
#endif

/** \brief Determines if the COM port needs to be optimized. Currently only
 *      USB virtual COM ports on Windows need to be optimized for their Latency
 *      Timer value.
 *
 * \param[in] portName The COM port name.
 * \param[out] isOptimized <c>true</c> if the COM port does not need
 *     optimization; <c>false</c> if the COM port needs to be optimized.
 * \return \ref VnError "VectorNav Error Code".
 */
VnError xplat_comport_isOptimized(
	char const* portName,
	bool* isOptimized);

/** \brief Attempts to optimize the COM port. Currently only USB virtual COM
 *      ports on Windows need to be optimized for their Latency Timer value.
 *
 * \param[in] portName The COM port name.
 * \return \ref VnError "VectorNav Error Code".
 */
VnError xplat_comport_optimize(
	char const* portName);

/** \brief Sleeps the current thread the specified number of milliseconds.
 *
 * \param[in] numOfMillisecondsToSleep The number of milliseconds to pause the
 *     current thread.
 * \return \ref VnError "VectorNav Error Code".
 */
VnError xplat_sleepMs(
	uint32_t numOfMillisecondsToSleep);

#ifdef __cplusplus
}
#endif

#endif
