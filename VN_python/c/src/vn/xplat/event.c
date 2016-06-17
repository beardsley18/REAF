/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
#include "vn/xplat/event.h"

#if __linux__ || __APPLE__ || __CYGWIN__ || __QNXNTO__
	#include <time.h>
	#include <errno.h>
#endif

#if __APPLE__
	#include <sys/time.h>
	#include <pthread.h>
	#include <mach/clock.h>
	#include <mach/mach.h>
#endif

VnError VnEvent_initialize(VnEvent *e)
{
	#if _WIN32

	e->handle = CreateEvent(
		NULL,
		FALSE,
		FALSE,
		NULL);

	if (e->handle == NULL)
		return E_UNKNOWN;

	#elif __linux__ || __APPLE__ || __CYGWIN__ || __QNXNTO__
	
	if (pthread_mutex_init(&e->mutex, NULL))
		return E_UNKNOWN;

	if (pthread_cond_init(&e->condition, NULL))
		return E_UNKNOWN;

	e->isTriggered = false;

	#else
	#error "Unknown System"
	#endif

	return E_NONE;
}

VnError VnEvent_wait(VnEvent *e)
{
	#if _WIN32

	DWORD result;

	result = WaitForSingleObject(e->handle, -1);

	if (result == WAIT_OBJECT_0)
		return E_NONE;

	return E_UNKNOWN;

	#elif __linux__ || __APPLE__ || __CYGWIN__ || __QNXNTO__
	
	if (pthread_mutex_lock(&e->mutex))
		return E_UNKNOWN;

	if (pthread_cond_wait(
		&e->condition,
		&e->mutex))
		return E_UNKNOWN;

	if (pthread_mutex_unlock(&e->mutex))
		return E_UNKNOWN;
		
	return E_NONE;

	#else
	#error "Unknown System"
	#endif
}

VnError VnEvent_waitMs(VnEvent *e, uint32_t timeoutMs)
{
	#if _WIN32

	DWORD result;

	result = WaitForSingleObject(e->handle, timeoutMs);

	if (result == WAIT_OBJECT_0)
		return E_SIGNALED;

	if (result == WAIT_TIMEOUT)
		return E_TIMEOUT;

	return E_UNKNOWN;

	#elif __linux__ || __APPLE__ || __CYGWIN__ || __QNXNTO__
	
	return VnEvent_waitUs(e, timeoutMs * 1000);

	#else
	#error "Unknown System"
	#endif
}

VnError VnEvent_waitUs(VnEvent *e, uint32_t timeoutUs)
{
	#if _WIN32
	
	DWORD result;

	result = WaitForSingleObject(
		e->handle,
		timeoutUs / 1000);

	if (result == WAIT_OBJECT_0)
		return E_SIGNALED;

	if (result == WAIT_TIMEOUT)
		return E_TIMEOUT;

	#elif __linux__ || __CYGWIN__ || __QNXNTO__

	struct timespec now;
	int errorCode;
	uint32_t numOfSecs, numOfNanoseconds;
	
	if (pthread_mutex_lock(&e->mutex))
		return E_UNKNOWN;
	
	if (clock_gettime(CLOCK_REALTIME, &now))
		return E_UNKNOWN;

	numOfSecs = timeoutUs / 1000000;
	numOfNanoseconds = (timeoutUs % 1000000) * 1000;

	now.tv_sec += numOfSecs;
	now.tv_nsec += numOfNanoseconds;

	if (now.tv_nsec > 1000000000)
	{
		now.tv_nsec %= 1000000000;
		now.tv_sec++;
	}

	errorCode = pthread_cond_timedwait(
		&e->condition,
		&e->mutex,
		&now);

	if (pthread_mutex_unlock(&e->mutex))
		return E_UNKNOWN;

	if (!errorCode)
		return E_SIGNALED;

	if (errorCode == ETIMEDOUT)
		return E_TIMEOUT;
	
	#elif __APPLE__

	pthread_mutex_lock(&e->mutex);

	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	
	struct timespec now;
	now.tv_sec = mts.tv_sec;
	now.tv_nsec = mts.tv_nsec;

	uint32_t numOfSecs = timeoutUs / 1000000;
	uint32_t numOfNanoseconds = (timeoutUs % 1000000) * 1000;

	now.tv_sec += numOfSecs;
	now.tv_nsec += numOfNanoseconds;

	if (now.tv_nsec > 1000000000)
	{
		now.tv_nsec %= 1000000000;
		now.tv_sec++;
	}

	int errorCode = pthread_cond_timedwait(
		&e->condition,
		&e->mutex,
		&now);

	pthread_mutex_unlock(&e->mutex);

	if (errorCode == 0)
		return E_SIGNALED;

	if (errorCode == ETIMEDOUT)
		return E_TIMEOUT;
		
	#else
	
	#error "Unknown System"

	#endif

	return E_UNKNOWN;
}

VnError VnEvent_signal(VnEvent *e)
{
	#if _WIN32

	if (!SetEvent(e->handle))
		return E_UNKNOWN;

	#elif __linux__ || __APPLE__ || __CYGWIN__ || __QNXNTO__
	
	if (pthread_mutex_lock(&e->mutex))
		return E_UNKNOWN;

	e->isTriggered = true;

	if (pthread_cond_signal(&e->condition))
		return E_UNKNOWN;
	
	if (pthread_mutex_unlock(&e->mutex))
		return E_UNKNOWN;
	
	#else
	#error "Unknown System"
	#endif
	
	return E_NONE;
}
