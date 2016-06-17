/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
#include "vn/xplat/time.h"

#if __linux__ || __CYGWIN__ || __QNXNTO__
	#include <time.h>
#elif __APPLE__
	#include <mach/clock.h>
	#include <mach/mach.h>
#endif

VnError VnStopwatch_initializeAndStart(VnStopwatch *sw)
{
	#if _WIN32
	
	sw->pcFrequency = 0;
	sw->counterStart = -1;
	
	#elif __linux__ || __APPLE__ || __CYGWIN__ || __QNXNTO__
	
	sw->clockStart = -1;
	
	#else
	#error "Unknown System"
	#endif

	/* Start the stopwatch. */
	return VnStopwatch_reset(sw);
}

VnError VnStopwatch_reset(VnStopwatch *sw)
{
	#if _WIN32

	LARGE_INTEGER li;
	if(!QueryPerformanceFrequency(&li))
		/* The hardware must not support a high-resolution performance counter. */
		return E_NOT_SUPPORTED;

	sw->pcFrequency = li.QuadPart / 1000.0;

	QueryPerformanceCounter(&li);

	sw->counterStart = li.QuadPart;

	#elif __linux__ || __CYGWIN__ || __QNXNTO__

	struct timespec time;
	int error;

	error = clock_gettime(CLOCK_MONOTONIC, &time);

	if (error)
		return E_UNKNOWN;

	sw->clockStart = (time.tv_sec * 1000.0) + (time.tv_nsec / 1000000.0);

	#elif __APPLE__

	clock_serv_t cclock;
	mach_timespec_t mts;

	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);

	sw->clockStart = (mts.tv_sec * 1000.0) + (mts.tv_nsec / 1000000.0);

	#else
	#error "Unknown System"
	#endif

	return E_NONE;
}

VnError VnStopwatch_elapsedMs(VnStopwatch *sw, float *elapsedMs)
{
	#if _WIN32

	LARGE_INTEGER li;

	if (sw->counterStart == -1)
		return E_UNKNOWN;

	QueryPerformanceCounter(&li);

	*elapsedMs = (float) ((li.QuadPart - sw->counterStart) / sw->pcFrequency);

	#elif __linux__ || __CYGWIN__ || __QNXNTO__
	
	struct timespec time;
	int error;

	if (sw->clockStart < 0)
		/* Clock not started. */
		return E_INVALID_OPERATION;

	error = clock_gettime(CLOCK_MONOTONIC, &time);

	if (error)
		return E_UNKNOWN;

	*elapsedMs = (time.tv_sec * 1000.0) + (time.tv_nsec / 1000000.0) - sw->clockStart;

	#elif __APPLE__
	
	clock_serv_t cclock;
	mach_timespec_t mts;
	
	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	
	return (mts.tv_sec * 1000.0) + (mts.tv_nsec / 1000000.0) - sw->clockStart;
		
	#else
	#error "Unknown System"
	#endif

	return E_NONE;
}
