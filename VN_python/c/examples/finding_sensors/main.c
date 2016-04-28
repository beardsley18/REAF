/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
#include "vn/sensors/searcher.h"
#include "stdio.h"

int main(int argv, char** argc)
{
	printf("VnSearcher:\n");
	printf("\n");
	printf("Finding Ports:\n");
	printf("\n");
	printf("Before you can use a sensor you need to know what ports\n");
	printf("are currently recognized by the system as being active.\n");
	printf("Using VnSearcher_findPorts(char***, int32_t*) we get\n");
	printf("\n");

	char** portList = NULL;
	int32_t index = 0;
	int32_t numPorts = 0;

	/* Call the find ports function to retrieve all port names. */
	/* NOTE: This call does not identify sensors; only port names. */
	VnSearcher_findPorts(&portList, &numPorts);

	if(numPorts > 0)
	{
		while(index < numPorts)
		{
			printf("  %2d: %s\n", index, portList[index]);
			index++;
		}
	}
	else
	{
		printf("  There do not appear to be any open ports on your system.\n");
	}

	printf("\n");
	printf("Now that we have a list of port names we can tell the system\n");
	printf("to search for an available sensor.  This means a sensor that\n");
	printf("is both connected to the computer and not currently being\n");
	printf("accessed by anything else.\n");
	printf("Using VnSearcher_SearchInputPorts(char***, uint32_t, VnPortInfo***) we get:\n");
	printf("\n");

	VnPortInfo** portInfo;
	index = 0;

	/* The call here will take in the information from the last call */
	/* and return a list of VnPortInfo structs.  These will be explained */
	/* later. */
	VnSearcher_searchInputPorts(&portList, numPorts, &portInfo);

	while(index < numPorts)
	{
		printf("  %2d: %s ", index, VnSearcher_getPortName(portInfo[index]));
		int32_t baud = VnSearcher_getPortBaud(portInfo[index]);

		if(baud > 0)
		{
			printf("has a baud rate of %d\n", baud);
		}
		else if(baud == -1)
		{
			printf("does not have a sensor attached.\n");
		}
		else
		{
			printf("encountered an error while searching for a sensor.\n");
		}

		index++;
	}

	printf("\n");
	printf("The name and baud are now available for you to connect to the\n");
	printf("sensor.\n");
	printf("\n");
	printf("Finding Sensor Baudrate:\n");
	printf("\n");
	printf("Sometimes the only thing you need to do is find the baud of the sensor\n");
	printf("in order to communicate with it.\n");
	printf("VnSearcher_findPortBaud(char*, int32_t*) will find the baud of the\n");
	printf("port and populate the uint32_t variable.\n");
	printf("A return value of -1 means that no sensor was connected to the port.\n");
	printf("A return value of -2 means an error occured during the search.\n");
	printf("\n");
	if(numPorts > 0)
	{
		printf("To demonstrate let's use port %s from above.\n", portList[0]);
		printf("\n");
		int32_t portBaud = 0;
		VnSearcher_findPortBaud(portList[0], &portBaud);
		printf("The returned baud rate is %d which indicates ", portBaud);

		if(portBaud > 0)
		{
			printf("a sensor is connected.\n");
		}
		else if(portBaud == -1)
		{
			printf("no sensor is connected.\n");
		}
		else
		{
			printf("an error occurred searching for the baud rate.\n");
		}
	}
	else
	{
		printf("Unable to run demonstration.  Please plug in a sensor and\n");
		printf("run this demo again.\n");
	}

	return 0;
}
