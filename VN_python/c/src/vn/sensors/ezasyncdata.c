/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
#include "vn/sensors.h"
#include "vn/sensors/compositedata.h"
#include "vn/sensors/ezasyncdata.h"
#include "vn/xplat/criticalsection.h"

#include <math.h>
#include <string.h>

void VnEzAsyncData_processReceivedAsyncPacket(void *userData, VnUartPacket *packet, size_t runningIndex);
VnError VnEzAsyncData_initializeAndConnect(VnEzAsyncData* ezAsyncData, const char* portName, uint32_t baudrate)
{
	VnError error;

	/** \brief The associated connected sensor. */
    ezAsyncData->sensor = (VnSensor*)malloc(sizeof(VnSensor));
    ezAsyncData->curData = (VnCompositeData*)malloc(sizeof(VnCompositeData));
    ezAsyncData->curDataCS = (VnCriticalSection*)malloc(sizeof(VnCompositeData));

    if((ezAsyncData->sensor == NULL) ||
       (ezAsyncData->curData == NULL) ||
       (ezAsyncData->curDataCS == NULL))
    {
        error = E_MEMORY_NOT_ALLOCATED;
        return error;
    }

	memset(ezAsyncData->curData, 0, sizeof(VnCompositeData));

	if ((error = VnCriticalSection_initialize(ezAsyncData->curDataCS)) != E_NONE)
		return error;

	if ((error = VnSensor_initialize(ezAsyncData->sensor)) != E_NONE)
		return error;

	if ((error = VnSensor_connect(ezAsyncData->sensor, portName, baudrate)) != E_NONE)
		return error;

	ezAsyncData->curData->velocityType = CDVEL_None;

	return VnSensor_registerAsyncPacketReceivedHandler(ezAsyncData->sensor, VnEzAsyncData_processReceivedAsyncPacket, ezAsyncData);
}

VnError VnEzAsyncData_disconnect(VnEzAsyncData* ezAsyncData)
{
	VnError error;

	if ((error = VnSensor_unregisterAsyncPacketReceivedHandler(ezAsyncData->sensor)) != E_NONE)
		return error;

	if ((error = VnCriticalSection_deinitialize(ezAsyncData->curDataCS)) != E_NONE)
		return error;

	return VnSensor_disconnect(ezAsyncData->sensor);
}

VnCompositeData VnEzAsyncData_currentData(VnEzAsyncData* ez)
{
	VnCompositeData cd;

	VnCriticalSection_enter(ez->curDataCS);

	cd = *(ez->curData);

	VnCriticalSection_leave(ez->curDataCS);

	return cd;
}

VnSensor* VnEzAsyncData_sensor(VnEzAsyncData* ez)
{
	return ez->sensor;
}

void VnEzAsyncData_processReceivedAsyncPacket(void *userData, VnUartPacket *packet, size_t runningIndex)
{
	VnEzAsyncData* ez = (VnEzAsyncData*) userData;

	if (VnUartPacket_isAsciiAsync(packet))
        VnCompositeData_processAsciiAsyncPacket(ez->curData, packet, ez->curDataCS);
	else
        VnCompositeData_processBinaryPacket(ez->curData, packet, ez->curDataCS);
}
