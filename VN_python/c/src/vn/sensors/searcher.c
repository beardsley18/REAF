/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
#include "vn/bool.h"
#include "vn/sensors/searcher.h"
#include "vn/xplat/serialport.h"
#include "vn/xplat/thread.h"

#include <stdio.h>
#include <sys/types.h>
#if NOT_COMPILING
	#include <dirent.h>
	#include <unistd.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>

uint32_t SubBD[9] = {9600, 19200, 38400, 57600, 115200, 128000, 230400, 460800, 921600};

uint32_t VnSearcher_matchBaud(char inputData[])
{
	uint32_t inputBaud = 0;
	char* start = strrchr(inputData, ',') + 1;
	char* end =  strrchr(start, '*');
	size_t index = 0;
	bool integerDigit = true;

	/* Isolate the baud in the message. */

	/* Ensure this is an integer digit. */
	while((start + index) < end)
	{
		char character = start[index];
		if(0 == isdigit(character))
		{
			integerDigit = false;
			break;
		}
		++index;
	}

	/* Convert the string to an integer for comparing. */
	if(integerDigit)
	{
		char stringBaud[20];
		uint32_t length = end - start;

		memcpy(stringBaud, start, length);
		stringBaud[length] = '\0';
		inputBaud = atoi(stringBaud);
	}

	/* Return the found baud */
	return inputBaud;
}

bool VnSearcher_validateData(char inputData[])
{
	/* Assume failure */
	bool validated = false;

	/* Get the position of the terminating characters.
	 * If they are not one right after the other then this is not a valid
	 * packet.
	 */
	char* first = strchr(inputData, '\r');
	char* second = strchr(inputData, '\n');

	/* Check to ensure that the two terminating characters are one right
	 * after the other.  Also, ensure that the data is prepended with the
	 * dollar sign.
	 */
	if(((second - first) == 1) && (inputData[0] == '$'))
	{
		char* comparison1 = strstr(inputData, "$VNRRG,5");
		char* comparison2 = strstr(inputData, "$VNRRG,05");
		if((comparison1 == inputData) || (comparison2 == inputData))
		{
			validated = true;
		}
	}

	/* Return the result. */
	return validated;
}

void VnSearcher_dataReceivedHandler(void* portInfo)
{
	/* Cast the input void pointer */
	VnPortInfo* port = (VnPortInfo*)portInfo;

	/* Prepare for handling incoming data */
	char readBuffer[256] = {0};

	/* Data validation flag.  Assume failure. */
	bool validated = false;

	size_t dataLength = strlen(port->data);
	size_t bytesToRead = sizeof(port->data) - dataLength;
	size_t bytesRead = 0;

	/* Keep looping until a valid packet is found or the thread shuts down. */
	/*while(port->port->continueHandlingSerialPortEvents)*/
	{
		/* Read the data from the port. */
		VnError error = VnSerialPort_read(port->port, readBuffer, bytesToRead, &bytesRead);

		memcpy(port->data + dataLength, readBuffer, bytesRead);

		/* Check if this is valid data from the sensor. */
		/*validated = VnSearcher_validateData(readBuffer);*/
		validated = VnSearcher_validateData(port->data);

		/* If an error occurs place a value of -2 into the baud variable to
		 * indicate this and end the thread.
		 */
		if(E_NONE != error)
		{
			port->baud = -2;
			/*break;*/
		}
		/* If the recieved data validated then break out of the while loop. */
		else if(validated)
		{
			/*break;*/
		}
	}

	/* If this is valid data then extract the baud rate and put the value into
	 * the port object.
	 */
	if(validated)
	{
		uint32_t bufferBaud = VnSearcher_matchBaud(port->data);
		port->baud = bufferBaud;
	}
}

void VnSearcher_testPort(void* portInfo)
{
	/* Cast the input void pointer */
	VnPortInfo* port = (VnPortInfo*)portInfo;

	size_t index = 0;
	size_t numBaudrates = sizeof(SubBD);

	/* Loop through all of the possible baud rates. */
	while(index < numBaudrates)
	{
		/* Set up a serial port object to communicate with. */
		VnSerialPort serialPort;
		port->port = &serialPort;
		VnSerialPort_initialize(&serialPort);
		VnSerialPort_registerDataReceivedHandler(&serialPort, VnSearcher_dataReceivedHandler, port);

		/* Open the port for communications. */
		VnSerialPort_open(&serialPort, port->portName, SubBD[index]);

		/* ensure the port opened. */
		if(VnSerialPort_isOpen(&serialPort))
		{
			/* Request the baud rate from the sensor. */
			VnSerialPort_write(&serialPort, "$VNRRG,05*XX\r\n", 14);

			/* Now wait 50 miliseconds to see if the sensor answers. */
			VnThread_sleepMs(50);

			/* Close the port to tell the data handling thread to terminate. */
			VnSerialPort_close(&serialPort);

			/* Check if we have a baud.
			 * -2 - Unknown error occured.
			 * -1 - No baud rate was found.
			 *  0 - Initial state.
			 * >0 - A valid baud rate was found. */
			if(port->baud != 0)
			{
				/* Either a sensor was found or an error occurred.
				 * End the while loop. */
				break;
			}
		}

		/* We don't want an infinite loop. */
		index++;
	}

	/* If there was no change in the baud variable indicate no sensor found
	 * using a value of -1. */
	if(port->baud == 0)
	{
		port->baud = -1;
	}
}

/* Public Functions */

char* VnSearcher_getPortName(VnPortInfo* portInfo)
{
	return portInfo->portName;
}

int32_t VnSearcher_getPortBaud(VnPortInfo* portInfo)
{
	return portInfo->baud;
}

void VnSearcher_findPorts(char*** portNames, int32_t* numPortsFound)
{
	#if NOT_COMPILING
	/* Linux path that stores names of existing ports */
	const char portDir[] = "/dev/serial/by-path/";
	const uint32_t portDirSize = sizeof(portDir);

	/* Directory object to search */
	DIR *dp;
	/* Struct that contains folder information */
	struct dirent *ep;

	int32_t linkCount = 0;

	/* attempt to open the port folder */
	dp = opendir(portDir);
	if(dp != NULL)
	{
		/* count the number of links */
		while((ep = readdir(dp)) != NULL)
		{
			/* Only count those that are links */
			if(ep->d_type == DT_LNK)
			{
				linkCount++;
			}
		}

		/* create an array of linkCount link names */
		*portNames = (char**)malloc(sizeof(char*) * linkCount);

		/* Reset the link count and directory search */
		linkCount = 0;
		rewinddir(dp);

		/* Now actually get the link names */
		while((ep = readdir(dp)) != NULL)
		{
			if(ep->d_type == DT_LNK)
			{
				/* Get the real path to the port */
				uint32_t sizeOfName = strlen(ep->d_name);
				char* fullPath = (char*)malloc(sizeof(char) * (portDirSize + sizeOfName));
				memcpy(fullPath, portDir, portDirSize);
				memcpy(fullPath + portDirSize - 1, ep->d_name, sizeOfName);

				char* canFile = (char*)canonicalize_file_name(fullPath);

				/* Copy the name into the return array */
				size_t stringLength = strlen(canFile);
				(*portNames)[linkCount] = (char*)malloc(sizeof(char) * stringLength + 1);
				memcpy((*portNames)[linkCount], canFile, stringLength + 1);

				/* Keep tabs on how large the array is */
				++linkCount;
			}
		}

		/* Load up the return value */
		*numPortsFound = linkCount;
	}
	else
	{
		/* There was an error.  Indicate this with -1 */
		*numPortsFound = -1;
	}
	#endif
}

void VnSearcher_findPortBaud(char* portName, int32_t* foundBaudrate)
{
	/* These will handle the return data. */
	VnPortInfo** portInfo;

	/* These will handle the input data. */
	char** portNames = &portName;
	int32_t numPorts = 1;

	/* This function will search the port and, if found, return a VnPortInfo */
	/* struct containing the baud rate. */
	VnSearcher_searchInputPorts(&portNames, numPorts, &portInfo);

	/* The number in baud will indicate whether the function succeeded
	 * -2 - error
	 * -1 - no sensor found
	 * >0 - sensor's baud rate
	 */
	*foundBaudrate = portInfo[0]->baud;
}

void VnSearcher_findAllSensors(VnPortInfo*** returnSensors, int32_t* numSensors)
{
	int32_t numSystemPorts = 0;
	char** systemPorts = NULL;

	/* First, find all of the ports on the system */
	VnSearcher_findPorts(&systemPorts, &numSystemPorts);

	/* Check if a sensor is accessible on the port */
	/* Only do this if the port search succeeded and found available system ports */
	if(numSystemPorts > 0)
	{
		VnSearcher_searchInputPorts(&systemPorts, numSystemPorts, returnSensors);
		*numSensors = numSystemPorts;
	}
}

void VnSearcher_searchInputPorts(char*** portsToCheck, int32_t numPortsToCheck, VnPortInfo*** sensorsFound)
{
	int32_t linkCount = 0;

	/* Initialize the sensorsFound array with space for any future pointers. */
	*sensorsFound = (VnPortInfo**) malloc(sizeof(VnPortInfo*) * numPortsToCheck);

	while(linkCount < numPortsToCheck)
	{
		VnPortInfo* newPort = (*sensorsFound)[linkCount];
		size_t nameSize = strlen((*portsToCheck)[linkCount]);

		/* Create the port info */
		(*sensorsFound)[linkCount] = (VnPortInfo*)malloc(sizeof(VnPortInfo));

		/* Give it the port name */
		(*sensorsFound)[linkCount]->portName = (char*)malloc(sizeof(char) * nameSize + 1);
		memcpy((*sensorsFound)[linkCount]->portName, (*portsToCheck)[linkCount], nameSize + 1);

		/* Make sure the data field is zeroed out. */
		memset(&(*sensorsFound)[linkCount]->data, 0, sizeof((*sensorsFound)[linkCount]->data));

		/* Set baud to 0.  A baud of 0 will indicate that the test is not finished
		 * running just yet.  A baud of -1 will indicate that no sensor has been
		 * found.  A baud of -2 will indicate an error.  Any baud over 0 will be
		 * the actual baud rate. */
		(*sensorsFound)[linkCount]->baud = 0;

		/* Set up the thread */

		/* Now, start the thread to check the port.*/
		VnThread_startNew(&(*sensorsFound)[linkCount]->thread, VnSearcher_testPort, (*sensorsFound)[linkCount]);

		/* And increment the link counter */
		++linkCount;
	}

	/* Reset the link counter. */
	linkCount = 0;

	/* Loop until all of the threads have completed. */
	while(linkCount < numPortsToCheck)
	{
		/* Sleep until the baud rate changes */
		while((*sensorsFound)[linkCount]->baud == 0)
		{
			VnThread_sleepMs(1);
		}

		/* The thread has finished.  The thread handles setting any pertinent
		 * data.  Simply increment the linkCount variable
		 */
		++linkCount;
	}
}
