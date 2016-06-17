/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
#include "vn/protocol/uart.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "vn/util.h"

#define MAXIMUM_REGISTER_ID		255
#define ASCII_START_CHAR		'$'
#define ASCII_END_CHAR1			'\r'
#define ASCII_END_CHAR2			'\n'
#define BINARY_START_CHAR		0xFA
#define MAX_BINARY_PACKET_SIZE	256

const uint8_t BinaryPacketGroupLengths[6][16] = {
	{ 8, 8, 8, 12, 16, 12, 24, 12, 12, 24, 20, 28, 2, 4, 8, 0 },
	{ 8, 8, 8, 2, 8, 8, 8, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 12, 12, 12, 4, 4, 16, 12, 12, 12, 12, 2, 40, 0, 0, 0 },
	{ 8, 8, 2, 1, 1, 24, 24, 12, 12, 12, 4, 4, 32, 0, 0, 0 },
	{ 2, 12, 16, 36, 12, 12, 12, 12, 12, 12, 28, 24, 0, 0, 0, 0 },
	{ 2, 24, 24, 12, 12, 12, 12, 12, 12, 4, 4, 68, 64, 0, 0, 0 },
};

#define NEXT result = VnUartPacket_getNextData(packet->data, &packetIndex); if (result == NULL) return;

#define NEXTRAW result = VnUartPacket_getNextData(packet, &packetIndex); if (result == NULL) return;

#define ATOFF ((float)atof(result))
#define ATOFD atof(result)
#define ATOU32 ((uint32_t) atoi(result))
#define ATOU16X ((uint16_t) strtol(result, NULL, 16))
#define ATOU16 ((uint16_t) atoi(result))
#define ATOU8 ((uint8_t) atoi(result))

/* Function declarations */
void VnUartPacketFinder_resetAsciiStatus(VnUartPacketFinder* finder);
void VnUartPacketFinder_resetBinaryStatus(VnUartPacketFinder* finder);
void VnUartPacketFinder_dispatchPacket(VnUartPacketFinder* finder, VnUartPacket* packet, size_t runningIndexOfPacketStart);
void VnUartPacketFinder_processPacket(VnUartPacketFinder* finder, char* packetStart, size_t packetLength, size_t runningIndexOfPacketStart);
char* VnUartPacket_startAsciiPacketParse(char* packetStart, size_t* index);
char* VnUartPacket_startAsciiResponsePacketParse(char* packetStart, size_t* index);
char* VnUartPacket_getNextData(char* str, size_t* startIndex);
char* vnstrtok(char* str, size_t* startIndex);
VnError VnUartPacket_genWriteBinaryOutput(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint8_t binaryOutputNumber,
	uint16_t asyncMode,
	uint16_t rateDivisor,
	uint16_t commonField,
	uint16_t timeField,
	uint16_t imuField,
	uint16_t gpsField,
	uint16_t attitudeField,
	uint16_t insField);
void VnUartPacket_startExtractionIfNeeded(VnUartPacket *packet);

void VnUartPacket_initialize(VnUartPacket *packet, char *data, size_t length)
{
	packet->curExtractLoc = 0;
	packet->length = length;
	packet->data = data;
}

void VnUartPacket_initializeFromStr(VnUartPacket* packet, char* data)
{
	VnUartPacket_initialize(packet, data, strlen(data));
}

bool VnUartPacket_isValid(VnUartPacket *packet)
{
	PacketType t;

	if (packet->length == 0)
		return false;

	t = VnUartPacket_type(packet);

	if (t == PACKETTYPE_ASCII)
	{
		/* First determine if this packet does not have a checksum or CRC. */
		if (packet->data[packet->length - 3] == 'X' && packet->data[packet->length - 4] == 'X')
			return true;

		/* First determine if this packet has an 8-bit checksum or a 16-bit CRC. */
		if (packet->data[packet->length - 5] == '*')
		{
			/* Appears we have an 8-bit checksum packet. */
			uint8_t expectedChecksum = VnUtil_toUint8FromHexStr(packet->data + packet->length - 4);

			uint8_t computedChecksum = VnChecksum8_compute(packet->data + 1, packet->length - 6);

			return (bool) (expectedChecksum == computedChecksum);
		}
		else if (packet->data[packet->length - 7] == '*')
		{
			/* Appears we have a 16-bit CRC packet. */
			uint16_t packetCrc = VnUtil_toUint16FromHexStr(packet->data + packet->length - 6);

			uint16_t computedCrc = VnCrc16_compute(packet->data + 1, packet->length - 8);

			return (bool) (packetCrc == computedCrc);
		}
		else
		{
			/* Don't know what we have. */
			return false;
		}
	}
	else if (t == PACKETTYPE_BINARY)
	{
		uint16_t computedCrc = VnCrc16_compute(packet->data + 1, packet->length - 1);

		return computedCrc == 0;
	}
	else
	{
		/* Unknown packet type. */
		return false;
	}
}

bool VnUartPacket_isAsciiAsync(VnUartPacket *packet)
{
	/* Pointer to the unique asynchronous data type identifier. */
	char* pAT = packet->data + 3;

	if (strncmp(pAT, "YPR", 3) == 0)
		return true;
	if (strncmp(pAT, "QTN", 3) == 0)
		return true;
	#ifdef EXTRA
	if (strncmp(pAT, "QTM", 3) == 0)
		return true;
	if (strncmp(pAT, "QTA", 3) == 0)
		return true;
	if (strncmp(pAT, "QTR", 3) == 0)
		return true;
	if (strncmp(pAT, "QMA", 3) == 0)
		return true;
	if (strncmp(pAT, "QAR", 3) == 0)
		return true;
	#endif
	if (strncmp(pAT, "QMR", 3) == 0)
		return true;
	#ifdef EXTRA
	if (strncmp(pAT, "DCM", 3) == 0)
		return true;
	#endif
	if (strncmp(pAT, "MAG", 3) == 0)
		return true;
	if (strncmp(pAT, "ACC", 3) == 0)
		return true;
	if (strncmp(pAT, "GYR", 3) == 0)
		return true;
	if (strncmp(pAT, "MAR", 3) == 0)
		return true;
	if (strncmp(pAT, "YMR", 3) == 0)
		return true;
	#ifdef EXTRA
	if (strncmp(pAT, "YCM", 3) == 0)
		return true;
	#endif
	if (strncmp(pAT, "YBA", 3) == 0)
		return true;
	if (strncmp(pAT, "YIA", 3) == 0)
		return true;
	#ifdef EXTRA
	if (strncmp(pAT, "ICM", 3) == 0)
		return true;
	#endif
	if (strncmp(pAT, "IMU", 3) == 0)
		return true;
	if (strncmp(pAT, "GPS", 3) == 0)
		return true;
	if (strncmp(pAT, "GPE", 3) == 0)
		return true;
	if (strncmp(pAT, "INS", 3) == 0)
		return true;
	if (strncmp(pAT, "INE", 3) == 0)
		return true;
	if (strncmp(pAT, "ISL", 3) == 0)
		return true;
	if (strncmp(pAT, "ISE", 3) == 0)
		return true;
	if (strncmp(pAT, "DTV", 3) == 0)
		return true;
	#ifdef EXTRA
	if (strncmp(pAT, "RAW", 3) == 0)
		return true;
	if (strncmp(pAT, "CMV", 3) == 0)
		return true;
	if (strncmp(pAT, "STV", 3) == 0)
		return true;
	if (strncmp(pAT, "COV", 3) == 0)
		return true;
	#endif
	else
		return false;
}

bool VnUartPacket_isResponse(VnUartPacket *packet)
{
	if (strncmp(packet->data + 3, "WRG", 3) == 0)
		return true;
	if (strncmp(packet->data + 3, "RRG", 3) == 0)
		return true;
	if (strncmp(packet->data + 3, "WNV", 3) == 0)
		return true;
	if (strncmp(packet->data + 3, "RFS", 3) == 0)
		return true;
	if (strncmp(packet->data + 3, "RST", 3) == 0)
		return true;
	if (strncmp(packet->data + 3, "FWU", 3) == 0)
		return true;
	if (strncmp(packet->data + 3, "CMD", 3) == 0)
		return true;
	if (strncmp(packet->data + 3, "ASY", 3) == 0)
		return true;
	if (strncmp(packet->data + 3, "TAR", 3) == 0)
		return true;
	if (strncmp(packet->data + 3, "KMD", 3) == 0)
		return true;
	if (strncmp(packet->data + 3, "KAD", 3) == 0)
		return true;
	if (strncmp(packet->data + 3, "SGB", 3) == 0)
		return true;

	return false;
}

bool VnUartPacket_isError(VnUartPacket *packet)
{
	return VnUartPacket_isErrorRaw(packet->data);
}

bool VnUartPacket_isErrorRaw(char *packet)
{
	return strncmp(packet + 3, "ERR", 3) == 0;
}

PacketType VnUartPacket_type(VnUartPacket *packet)
{
	if (packet->length < 1)
		/* Is really and invalid packet. */
		return PACKETTYPE_UNKNOWN;

	if (packet->data[0] == ASCII_START_CHAR)
		return PACKETTYPE_ASCII;
	if ((uint8_t) packet->data[0] == BINARY_START_CHAR)
		return PACKETTYPE_BINARY;

	return PACKETTYPE_UNKNOWN;
}

uint8_t VnUartPacket_groups(VnUartPacket* packet)
{
	return packet->data[1];
}

uint16_t VnUartPacket_groupField(VnUartPacket* packet, size_t groupIndex)
{
	return stoh16(*((uint16_t*) (packet->data + groupIndex * sizeof(uint16_t) + 2)));
}

VnAsciiAsync VnUartPacket_determineAsciiAsyncType(VnUartPacket *packet)
{
	/* Pointer to the unique asynchronous data type identifier. */
	char* pAT = packet->data + 3;

	if (strncmp(pAT, "YPR", 3) == 0)
		return VNYPR;
	if (strncmp(pAT, "QTN", 3) == 0)
		return VNQTN;
	#ifdef EXTRA
	if (strncmp(pAT, "QTM", 3) == 0)
		return VNQTM;
	if (strncmp(pAT, "QTA", 3) == 0)
		return VNQTA;
	if (strncmp(pAT, "QTR", 3) == 0)
		return VNQTR;
	if (strncmp(pAT, "QMA", 3) == 0)
		return VNQMA;
	if (strncmp(pAT, "QAR", 3) == 0)
		return VNQAR;
	#endif
	if (strncmp(pAT, "QMR", 3) == 0)
		return VNQMR;
	#ifdef EXTRA
	if (strncmp(pAT, "DCM", 3) == 0)
		return VNDCM;
	#endif
	if (strncmp(pAT, "MAG", 3) == 0)
		return VNMAG;
	if (strncmp(pAT, "ACC", 3) == 0)
		return VNACC;
	if (strncmp(pAT, "GYR", 3) == 0)
		return VNGYR;
	if (strncmp(pAT, "MAR", 3) == 0)
		return VNMAR;
	if (strncmp(pAT, "YMR", 3) == 0)
		return VNYMR;
	#ifdef EXTRA
	if (strncmp(pAT, "YCM", 3) == 0)
		return VNYCM;
	#endif
	if (strncmp(pAT, "YBA", 3) == 0)
		return VNYBA;
	if (strncmp(pAT, "YIA", 3) == 0)
		return VNYIA;
	#ifdef EXTRA
	if (strncmp(pAT, "ICM", 3) == 0)
		return VNICM;
	#endif
	if (strncmp(pAT, "IMU", 3) == 0)
		return VNIMU;
	if (strncmp(pAT, "GPS", 3) == 0)
		return VNGPS;
	if (strncmp(pAT, "GPE", 3) == 0)
		return VNGPE;
	if (strncmp(pAT, "INS", 3) == 0)
		return VNINS;
	if (strncmp(pAT, "INE", 3) == 0)
		return VNINE;
	if (strncmp(pAT, "ISL", 3) == 0)
		return VNISL;
	if (strncmp(pAT, "ISE", 3) == 0)
		return VNISE;
	if (strncmp(pAT, "DTV", 3) == 0)
		return VNDTV;
	#ifdef EXTRA
	if (strncmp(pAT, "RAW", 3) == 0)
		return VNRAW;
	if (strncmp(pAT, "CMV", 3) == 0)
		return VNCMV;
	if (strncmp(pAT, "STV", 3) == 0)
		return VNSTV;
	if (strncmp(pAT, "COV", 3) == 0)
		return VNCOV;
	#endif
	else
		/* Can't determine the packet type. */
		return VNOFF;
}

void VnUartPacketFinder_resetAsciiStatus(VnUartPacketFinder* finder)
{
	finder->asciiCurrentlyBuildingPacket = false;
	finder->asciiPossibleStartOfPacketIndex = 0;
	finder->asciiEndChar1Found = false;
	finder->asciiRunningDataIndexOfStart = 0;
}

void VnUartPacketFinder_resetBinaryStatus(VnUartPacketFinder* finder)
{
	finder->binaryCurrentlyBuildingBinaryPacket = false;
	finder->binaryGroupsPresentFound = false;
	finder->binaryGroupsPresent = 0;
	finder->binaryNumOfBytesRemainingToHaveAllGroupFields = 0;
	finder->binaryPossibleStartIndex = 0;
	finder->binaryNumberOfBytesRemainingForCompletePacket = 0;
	finder->binaryRunningDataIndexOfStart = 0;
}

void VnUartPacketFinder_resetAllTrackingStatus(VnUartPacketFinder *finder)
{
	if (finder->asciiCurrentlyBuildingPacket)
		VnUartPacketFinder_resetAsciiStatus(finder);

	if (finder->binaryCurrentlyBuildingBinaryPacket)
		VnUartPacketFinder_resetBinaryStatus(finder);

	finder->bufferAppendLocation = 0;
}

void VnUartPacketFinder_dispatchPacket(VnUartPacketFinder* finder, VnUartPacket* packet, size_t runningIndexOfPacketStart)
{
	if (finder->packetFoundHandler == NULL)
		return;

	finder->packetFoundHandler(finder->packetFoundHandlerUserData, packet, runningIndexOfPacketStart);
}

void VnUartPacketFinder_processPacket(VnUartPacketFinder* finder, char* packetStart, size_t packetLength, size_t runningIndexOfPacketStart)
{
	VnUartPacket p;
	PacketType t;

	VnUartPacket_initialize(&p, packetStart, packetLength);

	t = VnUartPacket_type(&p);

	if (VnUartPacket_isValid(&p))
	{
		VnUartPacketFinder_dispatchPacket(finder, &p, runningIndexOfPacketStart);

		/* Reset tracking for both packet types since this packet was valid. */
		VnUartPacketFinder_resetAsciiStatus(finder);
		VnUartPacketFinder_resetBinaryStatus(finder);
		finder->bufferAppendLocation = 0;
	}
	else
	{
		/* Invalid packet! Reset tracking. */

		if (t == PACKETTYPE_ASCII)
		{
			VnUartPacketFinder_resetAsciiStatus(finder);

			if (!finder->binaryCurrentlyBuildingBinaryPacket)
				finder->bufferAppendLocation = 0;
		}
		else
		{
			VnUartPacketFinder_resetBinaryStatus(finder);

			if (!finder->asciiCurrentlyBuildingPacket)
				finder->bufferAppendLocation = 0;
		}
	}
}

size_t VnUartPacket_computeNumOfBytesForBinaryGroupPayload(BinaryGroupType groupType, uint16_t groupField)
{
	size_t runningLength = 0;
	size_t i;

	/* Determine which group is present. */
	size_t groupIndex = 0;
	for (i = 0; i < 8; i++, groupIndex++)
	{
		if (((size_t) groupType >> i) & 0x01)
			break;
	}

	for (i = 0; i < sizeof(uint16_t) * 8; i++)
	{
		if ((groupField >> i) & 1)
		{
			runningLength += BinaryPacketGroupLengths[groupIndex][i];
		}
	}

	return runningLength;
}

bool VnUartPacket_isCompatible(
	VnUartPacket *packet,
	CommonGroup commonGroup,
	TimeGroup timeGroup,
	ImuGroup imuGroup,
	GpsGroup gpsGroup,
	AttitudeGroup attitudeGroup,
	InsGroup insGroup)
{
	/* First make sure the appropriate groups are specified. */
	uint8_t groups = packet->data[1];
	char *curField = packet->data + 2;

	if (commonGroup)
	{
		if (stoh16(*(uint16_t*) curField) != commonGroup)
			/* Not the expected collection of field data types. */
			return false;

		curField += 2;
	}
	else if (groups & 0x01)
	{
		/* There is unexpected Common Group data. */
		return false;
	}

	if (timeGroup)
	{
		if (stoh16(*(uint16_t*) curField) != timeGroup)
			/* Not the expected collection of field data types. */
			return false;

		curField += 2;
	}
	else if (groups & 0x02)
	{
		/* There is unexpected Time Group data. */
		return false;
	}

	if (imuGroup)
	{
		if (stoh16(*(uint16_t*) curField) != imuGroup)
			/* Not the expected collection of field data types. */
			return false;

		curField += 2;
	}
	else if (groups & 0x04)
	{
		/* There is unexpected IMU Group data. */
		return false;
	}

	if (gpsGroup)
	{
		if (stoh16(*(uint16_t*) curField) != gpsGroup)
			/* Not the expected collection of field data types. */
			return false;

		curField += 2;
	}
	else if (groups & 0x08)
	{
		/* There is unexpected GPS Group data. */
		return false;
	}

	if (attitudeGroup)
	{
		if (stoh16(*(uint16_t*) curField) != attitudeGroup)
			/* Not the expected collection of field data types. */
			return false;

		curField += 2;
	}
	else if (groups & 0x10)
	{
		/* There is unexpected Attitude Group data. */
		return false;
	}

	if (insGroup)
	{
		if (stoh16(*(uint16_t*) curField) != insGroup)
			/* Not the expected collection of field data types. */
			return false;

		curField += 2;
	}
	else if (groups & 0x20)
	{
		/* There is unexpected INS Group data. */
		return false;
	}

	/* Everything checks out. */
	return true;
}

void VnUartPacket_startExtractionIfNeeded(VnUartPacket *packet)
{
	if (packet->curExtractLoc == 0)
		/* Determine the location to start extracting. */
		packet->curExtractLoc = VnUtil_countSetBitsUint8(packet->data[1]) * 2 + 2;
}

uint8_t VnUartPacket_extractUint8(VnUartPacket *packet)
{
	uint8_t d;

	VnUartPacket_startExtractionIfNeeded(packet);

	d =  *(uint8_t*) (packet->data + packet->curExtractLoc);

	packet->curExtractLoc += sizeof(uint8_t);

	return d;
}

int8_t VnUartPacket_extractInt8(VnUartPacket *packet)
{
	int8_t d;

	VnUartPacket_startExtractionIfNeeded(packet);

	d =  *(int8_t*) (packet->data + packet->curExtractLoc);

	packet->curExtractLoc += sizeof(int8_t);

	return d;
}

uint16_t VnUartPacket_extractUint16(VnUartPacket *packet)
{
	char* extractLocation;

	VnUartPacket_startExtractionIfNeeded(packet);

	extractLocation = packet->data + packet->curExtractLoc;

	packet->curExtractLoc += sizeof(uint16_t);

	return VnUtil_extractUint16(extractLocation);
}

uint32_t VnUartPacket_extractUint32(VnUartPacket *packet)
{
	char* extractLocation;

	VnUartPacket_startExtractionIfNeeded(packet);
	
	extractLocation = packet->data + packet->curExtractLoc;
	
	packet->curExtractLoc += sizeof(uint32_t);

	return VnUtil_extractUint32(extractLocation);
}

uint64_t VnUartPacket_extractUint64(VnUartPacket *packet)
{
	uint64_t d;

	VnUartPacket_startExtractionIfNeeded(packet);

	memcpy(&d, packet->data + packet->curExtractLoc, sizeof(uint64_t));

	packet->curExtractLoc += sizeof(uint64_t);

	return stoh64(d);
}

float VnUartPacket_extractFloat(VnUartPacket* packet)
{
	char* extractLocation;

	VnUartPacket_startExtractionIfNeeded(packet);

	extractLocation = packet->data + packet->curExtractLoc;

	packet->curExtractLoc += sizeof(float);

	return VnUtil_extractFloat(extractLocation);
}

vec3f VnUartPacket_extractVec3f(VnUartPacket *packet)
{
	char* extractLocation;

	VnUartPacket_startExtractionIfNeeded(packet);

	extractLocation = packet->data + packet->curExtractLoc;

	packet->curExtractLoc += 3 * sizeof(float);

	return VnUtil_extractVec3f(extractLocation);
}

vec3d VnUartPacket_extractVec3d(VnUartPacket *packet)
{
	char* extractLocation;

	VnUartPacket_startExtractionIfNeeded(packet);

	extractLocation = packet->data + packet->curExtractLoc;

	packet->curExtractLoc += 3 * sizeof(double);

	return VnUtil_extractVec3d(extractLocation);
}

vec4f VnUartPacket_extractVec4f(VnUartPacket *packet)
{
	char* extractLocation;

	VnUartPacket_startExtractionIfNeeded(packet);

	extractLocation = packet->data + packet->curExtractLoc;

	packet->curExtractLoc += 4 * sizeof(float);

	return VnUtil_extractVec4f(extractLocation);
}

mat3f VnUartPacket_extractMat3f(VnUartPacket *packet)
{
	char* extractLocation;

	VnUartPacket_startExtractionIfNeeded(packet);

	extractLocation = packet->data + packet->curExtractLoc;

	packet->curExtractLoc += 9 * sizeof(float);

	return VnUtil_extractMat3f(extractLocation);
}

void VnUartPacketFinder_initialize(VnUartPacketFinder* toInitialize)
{
	toInitialize->packetFoundHandler = NULL;
	toInitialize->packetFoundHandlerUserData = NULL;
	toInitialize->runningDataIndex = 0;
	toInitialize->asciiCurrentlyBuildingPacket = false;
	toInitialize->asciiPossibleStartOfPacketIndex = 0;
	toInitialize->asciiRunningDataIndexOfStart = 0;
	toInitialize->asciiEndChar1Found = false;
	toInitialize->binaryCurrentlyBuildingBinaryPacket = false;
	toInitialize->binaryRunningDataIndexOfStart = 0;
	toInitialize->bufferSize = VNUART_PROTOCOL_BUFFER_SIZE;
	toInitialize->binaryGroupsPresentFound = false;
	toInitialize->binaryGroupsPresent = 0;
	toInitialize->binaryNumOfBytesRemainingToHaveAllGroupFields = 0;
	toInitialize->binaryPossibleStartIndex = 0;
	toInitialize->binaryNumberOfBytesRemainingForCompletePacket = 0;
	toInitialize->bufferAppendLocation = 0;

}

VnError VnUartPacketFinder_registerPacketFoundHandler(VnUartPacketFinder* finder, VnUartPacketFinder_PacketFoundHandler handler, void *userData)
{
	if (finder->packetFoundHandler != NULL)
		return E_INVALID_OPERATION;

	finder->packetFoundHandler = handler;
	finder->packetFoundHandlerUserData = userData;

	return E_NONE;
}

void VnUartPacketFinder_processReceivedData(VnUartPacketFinder* finder, char* data, size_t length)
{
	bool asciiStartFoundInProvidedBuffer = false;
	bool binaryStartFoundInProvidedDataBuffer = false;
	size_t i, dataIndexToStartCopyingFrom, binaryDataMoveOverIndexAdjustment;
	bool binaryDataToCopyOver;

	/* Assume that since the _runningDataIndex is unsigned, any overflows
	 * will naturally go to zero, which is the behavior that we want. */
	for (i = 0; i < length; i++, finder->runningDataIndex++)
	{
		if (data[i] == ASCII_START_CHAR)
		{
			VnUartPacketFinder_resetAsciiStatus(finder);
			finder->asciiCurrentlyBuildingPacket = true;
			finder->asciiPossibleStartOfPacketIndex = i;
			finder->asciiRunningDataIndexOfStart = finder->runningDataIndex;

			asciiStartFoundInProvidedBuffer = true;
		}
		else if (finder->asciiCurrentlyBuildingPacket && data[i] == ASCII_END_CHAR1)
		{
			finder->asciiEndChar1Found = true;
		}
		else if (finder->asciiEndChar1Found)
		{
			if (data[i] == ASCII_END_CHAR2)
			{
				/* We have a possible data packet. */

				if (asciiStartFoundInProvidedBuffer)
				{
					char* startOfAsciiPacket;
					size_t packetLength;

					/* All the packet was in this data buffer so we don't
					 * need to do any copying. */

					startOfAsciiPacket = data + finder->asciiPossibleStartOfPacketIndex;
					packetLength = i - finder->asciiPossibleStartOfPacketIndex + 1;

					VnUartPacketFinder_processPacket(finder, startOfAsciiPacket, packetLength, finder->asciiRunningDataIndexOfStart);

					asciiStartFoundInProvidedBuffer = false;
				}
				else
				{
					/* The packet was split between the running data buffer
					 * the current data buffer. We need to copy the data
					 * over before further processing. */

					if (finder->bufferAppendLocation + i < finder->bufferSize)
					{
						char* startOfAsciiPacket;
						size_t packetLength;

						memcpy(finder->receiveBuffer + finder->bufferAppendLocation, data, i + 1);

						startOfAsciiPacket = finder->receiveBuffer + finder->asciiPossibleStartOfPacketIndex;
						packetLength = finder->bufferAppendLocation + i + 1 - finder->asciiPossibleStartOfPacketIndex;

						VnUartPacketFinder_processPacket(finder, startOfAsciiPacket, packetLength, finder->asciiRunningDataIndexOfStart);

						asciiStartFoundInProvidedBuffer = false;
					}
					else
					{
						/* We are about to overflow our buffer. */
						VnUartPacketFinder_resetAllTrackingStatus(finder);
						asciiStartFoundInProvidedBuffer = false;
						binaryStartFoundInProvidedDataBuffer = false;
					}
				}
			}
			else
			{
				/* Must not be a valid ASCII packet. */
				VnUartPacketFinder_resetAsciiStatus(finder);
				asciiStartFoundInProvidedBuffer = false;
				if (!finder->binaryCurrentlyBuildingBinaryPacket)
					finder->bufferAppendLocation = 0;
			}
		}

		/* Update our binary packet in processing. */
		if (finder->binaryCurrentlyBuildingBinaryPacket)
		{
			if (!finder->binaryGroupsPresentFound)
			{
				/* This byte must be the groups present. */
				finder->binaryGroupsPresentFound = true;
				finder->binaryGroupsPresent = (uint8_t) data[i];
				finder->binaryNumOfBytesRemainingToHaveAllGroupFields = (uint8_t) (2 * VnUtil_countSetBitsUint8((uint8_t) data[i]));
			}
			else if (finder->binaryNumOfBytesRemainingToHaveAllGroupFields != 0)
			{
				/* We found another byte belonging to this possible binary packet. */
				finder->binaryNumOfBytesRemainingToHaveAllGroupFields--;

				if (finder->binaryNumOfBytesRemainingToHaveAllGroupFields == 0)
				{
					/* We have all of the group fields now. */

					size_t remainingBytesForCompletePacket;
					if (binaryStartFoundInProvidedDataBuffer)
					{
						size_t headerLength = i - finder->binaryPossibleStartIndex + 1;
						remainingBytesForCompletePacket = VnUartPacket_computeBinaryPacketLength(data + finder->binaryPossibleStartIndex) - headerLength;
					}
					else
					{
						/* Not all of the packet's group is inside the caller's provided buffer. */

						/* Temporarily copy the rest of the packet to the receive buffer
						 * for computing the size of the packet.
						 */

						size_t numOfBytesToCopyIntoReceiveBuffer = i + 1;
						size_t headerLength = finder->bufferAppendLocation - finder->binaryPossibleStartIndex + numOfBytesToCopyIntoReceiveBuffer;

						if (finder->bufferAppendLocation + numOfBytesToCopyIntoReceiveBuffer < finder->bufferSize)
						{
							memcpy(
								finder->receiveBuffer + finder->bufferAppendLocation,
								data,
								numOfBytesToCopyIntoReceiveBuffer);

							remainingBytesForCompletePacket = VnUartPacket_computeBinaryPacketLength(finder->receiveBuffer + finder->binaryPossibleStartIndex) - headerLength;
						}
						else
						{
							/* About to overrun our receive buffer! */
							VnUartPacketFinder_resetAllTrackingStatus(finder);
							binaryStartFoundInProvidedDataBuffer = false;
							asciiStartFoundInProvidedBuffer = false;
						}
					}

					if (finder->binaryCurrentlyBuildingBinaryPacket)
					{
						if (remainingBytesForCompletePacket > MAX_BINARY_PACKET_SIZE)
						{
							/* Must be a bad possible binary packet. */
							VnUartPacketFinder_resetBinaryStatus(finder);
							binaryStartFoundInProvidedDataBuffer = false;

							if (!finder->asciiCurrentlyBuildingPacket)
								finder->bufferAppendLocation = 0;
						}
						else
						{
							finder->binaryNumberOfBytesRemainingForCompletePacket = remainingBytesForCompletePacket;
						}
					}
				}
			}
			else
			{
				/* We are currently collecting data for our packet. */

				finder->binaryNumberOfBytesRemainingForCompletePacket--;

				if (finder->binaryNumberOfBytesRemainingForCompletePacket == 0)
				{
					/* We have a possible binary packet! */

					if (binaryStartFoundInProvidedDataBuffer)
					{
						char *packetStart;
						size_t packetLength;

						/* The binary packet exists completely in the user's provided buffer. */
						packetStart = data + finder->binaryPossibleStartIndex;
						packetLength = i - finder->binaryPossibleStartIndex + 1;

						VnUartPacketFinder_processPacket(finder, packetStart, packetLength, finder->binaryRunningDataIndexOfStart);
					}
					else
					{
						/* The packet is split between our receive buffer and the user's buffer. */
						size_t numOfBytesToCopyIntoReceiveBuffer = i + 1;

						if (finder->bufferAppendLocation + numOfBytesToCopyIntoReceiveBuffer < finder->bufferSize)
						{
							char *packetStart;
							size_t packetLength;

							memcpy(
								finder->receiveBuffer + finder->bufferAppendLocation,
								data,
								numOfBytesToCopyIntoReceiveBuffer);

							packetStart = finder->receiveBuffer + finder->binaryPossibleStartIndex;
							packetLength = finder->bufferAppendLocation - finder->binaryPossibleStartIndex + i + 1;

							VnUartPacketFinder_processPacket(finder, packetStart, packetLength, finder->binaryRunningDataIndexOfStart);
						}
						else
						{
							/* About to overrun our receive buffer! */
							VnUartPacketFinder_resetAllTrackingStatus(finder);
							binaryStartFoundInProvidedDataBuffer = false;
							asciiStartFoundInProvidedBuffer = false;
						}
					}
				}
			}
		}

		if (((uint8_t) data[i]) == BINARY_START_CHAR)
		{
			/* Possible start of a binary packet. */
			binaryStartFoundInProvidedDataBuffer = true;
			finder->binaryCurrentlyBuildingBinaryPacket = true;
			finder->binaryPossibleStartIndex = i;
			finder->binaryGroupsPresentFound = false;
			finder->binaryNumOfBytesRemainingToHaveAllGroupFields = 0;
			finder->binaryNumberOfBytesRemainingForCompletePacket = 0;
			finder->binaryRunningDataIndexOfStart = finder->runningDataIndex;
		}
	}

	/* Perform any data copying to our receive buffer. */

	dataIndexToStartCopyingFrom = 0;
	binaryDataToCopyOver = false;
	binaryDataMoveOverIndexAdjustment = 0;

	if (finder->binaryCurrentlyBuildingBinaryPacket)
	{
		binaryDataToCopyOver = true;

		if (binaryStartFoundInProvidedDataBuffer)
		{
			dataIndexToStartCopyingFrom = finder->binaryPossibleStartIndex;
			binaryDataMoveOverIndexAdjustment = dataIndexToStartCopyingFrom;
		}
	}

	if (finder->asciiCurrentlyBuildingPacket && asciiStartFoundInProvidedBuffer)
	{
		if (finder->asciiPossibleStartOfPacketIndex < dataIndexToStartCopyingFrom)
		{
			binaryDataMoveOverIndexAdjustment -= binaryDataMoveOverIndexAdjustment - finder->asciiPossibleStartOfPacketIndex;
			dataIndexToStartCopyingFrom = finder->asciiPossibleStartOfPacketIndex;
		}
		else if (!binaryDataToCopyOver)
		{
			dataIndexToStartCopyingFrom = finder->asciiPossibleStartOfPacketIndex;
		}

		/* Adjust our ASCII index to be based on the receive buffer. */
		finder->asciiPossibleStartOfPacketIndex = finder->bufferAppendLocation + finder->asciiPossibleStartOfPacketIndex - dataIndexToStartCopyingFrom;
	}

	/* Adjust any binary packet indexes we are currently building. */
	if (finder->binaryCurrentlyBuildingBinaryPacket)
	{
		if (binaryStartFoundInProvidedDataBuffer)
		{
			finder->binaryPossibleStartIndex = finder->binaryPossibleStartIndex - binaryDataMoveOverIndexAdjustment + finder->bufferAppendLocation;
		}
	}

	if (finder->bufferAppendLocation + length - dataIndexToStartCopyingFrom < finder->bufferSize)
	{
		/* Safe to copy over the data. */

		size_t numOfBytesToCopyOver = length - dataIndexToStartCopyingFrom;
		char *copyFromStart = data + dataIndexToStartCopyingFrom;

		memcpy(
			finder->receiveBuffer + finder->bufferAppendLocation,
			copyFromStart,
			numOfBytesToCopyOver);

		finder->bufferAppendLocation += numOfBytesToCopyOver;
	}
	else
	{
		/* We are about to overflow our buffer. */
		VnUartPacketFinder_resetAsciiStatus(finder);
		VnUartPacketFinder_resetBinaryStatus(finder);
		finder->bufferAppendLocation = 0;
	}
}

size_t VnUartPacket_computeBinaryPacketLength(char const* startOfPossibleBinaryPacket)
{
	uint8_t groupsPresent = startOfPossibleBinaryPacket[1];
	size_t runningPayloadLength = 2;	/* Start of packet character plus groups present field. */
	char const* pCurrentGroupField = startOfPossibleBinaryPacket + 2;

	if (groupsPresent & 0x01)
	{
		runningPayloadLength += 2 + VnUartPacket_computeNumOfBytesForBinaryGroupPayload(BINARYGROUPTYPE_COMMON, stoh16(*(uint16_t*)pCurrentGroupField));
		pCurrentGroupField += 2;
	}

	if (groupsPresent & 0x02)
	{
		runningPayloadLength += 2 + VnUartPacket_computeNumOfBytesForBinaryGroupPayload(BINARYGROUPTYPE_TIME, stoh16(*(uint16_t*)pCurrentGroupField));
		pCurrentGroupField += 2;
	}

	if (groupsPresent & 0x04)
	{
		runningPayloadLength += 2 + VnUartPacket_computeNumOfBytesForBinaryGroupPayload(BINARYGROUPTYPE_IMU, stoh16(*(uint16_t*)pCurrentGroupField));
		pCurrentGroupField += 2;
	}

	if (groupsPresent & 0x08)
	{
		runningPayloadLength += 2 + VnUartPacket_computeNumOfBytesForBinaryGroupPayload(BINARYGROUPTYPE_GPS, stoh16(*(uint16_t*)pCurrentGroupField));
		pCurrentGroupField += 2;
	}

	if (groupsPresent & 0x10)
	{
		runningPayloadLength += 2 + VnUartPacket_computeNumOfBytesForBinaryGroupPayload(BINARYGROUPTYPE_ATTITUDE, stoh16(*(uint16_t*)pCurrentGroupField));
		pCurrentGroupField += 2;
	}

	if (groupsPresent & 0x20)
	{
		runningPayloadLength += 2 + VnUartPacket_computeNumOfBytesForBinaryGroupPayload(BINARYGROUPTYPE_INS, stoh16(*(uint16_t*)pCurrentGroupField));
		pCurrentGroupField += 2;
	}

	return runningPayloadLength + 2;	/* Add 2 bytes for CRC. */
}

char* VnUartPacket_startAsciiPacketParse(char* packetStart, size_t* index)
{
	*index = 7;

	return vnstrtok(packetStart, index);
}

char* VnUartPacket_startAsciiResponsePacketParse(char* packetStart, size_t* index)
{
	VnUartPacket_startAsciiPacketParse(packetStart, index);

	return vnstrtok(packetStart, index);
}

char* VnUartPacket_getNextData(char* str, size_t* startIndex)
{
	return vnstrtok(str, startIndex);
}

char* vnstrtok(char* str, size_t* startIndex)
{
	size_t origIndex = *startIndex;

	while (str[*startIndex] != ',' && str[*startIndex] != '*')
		(*startIndex)++;

	str[(*startIndex)++] = '\0';

	return str + origIndex;
}

void VnUartPacket_parseVNYPR(VnUartPacket* packet, vec3f* yawPitchRoll)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	yawPitchRoll->x = ATOFF; NEXT
	yawPitchRoll->y = ATOFF; NEXT
	yawPitchRoll->z = ATOFF;
}

void VnUartPacket_parseVNQTN(VnUartPacket* packet, vec4f* quaternion)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	quaternion->x = ATOFF; NEXT
	quaternion->y = ATOFF; NEXT
	quaternion->z = ATOFF; NEXT
	quaternion->w = ATOFF;
}

#ifdef EXTRA

void VnUartPacket_parseVNQTM(VnUartPacket* packet, vec4f *quaternion, vec3f *magnetic)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	quaternion->x = ATOFF; NEXT
	quaternion->y = ATOFF; NEXT
	quaternion->z = ATOFF; NEXT
	quaternion->w = ATOFF; NEXT
	magnetic->x = ATOFF; NEXT
	magnetic->y = ATOFF; NEXT
	magnetic->z = ATOFF;
}

void VnUartPacket_parseVNQTA(VnUartPacket* packet, vec4f* quaternion, vec3f* acceleration)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	quaternion->x = ATOFF; NEXT
	quaternion->y = ATOFF; NEXT
	quaternion->z = ATOFF; NEXT
	quaternion->w = ATOFF; NEXT
	acceleration->x = ATOFF; NEXT
	acceleration->y = ATOFF; NEXT
	acceleration->z = ATOFF;
}

void VnUartPacket_parseVNQTR(VnUartPacket* packet, vec4f* quaternion, vec3f* angularRate)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	quaternion->x = ATOFF; NEXT
	quaternion->y = ATOFF; NEXT
	quaternion->z = ATOFF; NEXT
	quaternion->w = ATOFF; NEXT
	angularRate->x = ATOFF; NEXT
	angularRate->y = ATOFF; NEXT
	angularRate->z = ATOFF;
}

void VnUartPacket_parseVNQMA(VnUartPacket* packet, vec4f* quaternion, vec3f* magnetic, vec3f* acceleration)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	quaternion->x = ATOFF; NEXT
	quaternion->y = ATOFF; NEXT
	quaternion->z = ATOFF; NEXT
	quaternion->w = ATOFF; NEXT
	magnetic->x = ATOFF; NEXT
	magnetic->y = ATOFF; NEXT
	magnetic->z = ATOFF; NEXT
	acceleration->x = ATOFF; NEXT
	acceleration->y = ATOFF; NEXT
	acceleration->z = ATOFF;
}

void VnUartPacket_parseVNQAR(VnUartPacket* packet, vec4f* quaternion, vec3f* acceleration, vec3f* angularRate)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	quaternion->x = ATOFF; NEXT
	quaternion->y = ATOFF; NEXT
	quaternion->z = ATOFF; NEXT
	quaternion->w = ATOFF; NEXT
	acceleration->x = ATOFF; NEXT
	acceleration->y = ATOFF; NEXT
	acceleration->z = ATOFF; NEXT
	angularRate->x = ATOFF; NEXT
	angularRate->y = ATOFF; NEXT
	angularRate->z = ATOFF;
}

#endif

void VnUartPacket_parseVNQMR(VnUartPacket* packet, vec4f* quaternion, vec3f* magnetic, vec3f* acceleration, vec3f* angularRate)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	quaternion->x = ATOFF; NEXT
	quaternion->y = ATOFF; NEXT
	quaternion->z = ATOFF; NEXT
	quaternion->w = ATOFF; NEXT
	magnetic->x = ATOFF; NEXT
	magnetic->y = ATOFF; NEXT
	magnetic->z = ATOFF; NEXT
	acceleration->x = ATOFF; NEXT
	acceleration->y = ATOFF; NEXT
	acceleration->z = ATOFF; NEXT
	angularRate->x = ATOFF; NEXT
	angularRate->y = ATOFF; NEXT
	angularRate->z = ATOFF;
}

#ifdef EXTRA

void VnUartPacket_parseVNDCM(VnUartPacket* packet, mat3f* dcm)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	dcm->e00 = ATOFF; NEXT
	dcm->e01 = ATOFF; NEXT
	dcm->e02 = ATOFF; NEXT
	dcm->e10 = ATOFF; NEXT
	dcm->e11 = ATOFF; NEXT
	dcm->e12 = ATOFF; NEXT
	dcm->e20 = ATOFF; NEXT
	dcm->e21 = ATOFF; NEXT
	dcm->e22 = ATOFF;
}

#endif

void VnUartPacket_parseVNMAG(VnUartPacket* packet, vec3f* magnetic)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	magnetic->x = ATOFF; NEXT
	magnetic->y = ATOFF; NEXT
	magnetic->z = ATOFF;
}

void VnUartPacket_parseVNACC(VnUartPacket* packet, vec3f* acceleration)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	acceleration->x = ATOFF; NEXT
	acceleration->y = ATOFF; NEXT
	acceleration->z = ATOFF;
}

void VnUartPacket_parseVNGYR(VnUartPacket* packet, vec3f* angularRate)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	angularRate->x = ATOFF; NEXT
	angularRate->y = ATOFF; NEXT
	angularRate->z = ATOFF;
}

void VnUartPacket_parseVNMAR(VnUartPacket* packet, vec3f* magnetic, vec3f* acceleration, vec3f* angularRate)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	magnetic->x = ATOFF; NEXT
	magnetic->y = ATOFF; NEXT
	magnetic->z = ATOFF; NEXT
	acceleration->x = ATOFF; NEXT
	acceleration->y = ATOFF; NEXT
	acceleration->z = ATOFF; NEXT
	angularRate->x = ATOFF; NEXT
	angularRate->y = ATOFF; NEXT
	angularRate->z = ATOFF;
}

void VnUartPacket_parseVNYMR(VnUartPacket* packet, vec3f* yawPitchRoll, vec3f* magnetic, vec3f* acceleration, vec3f* angularRate)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	yawPitchRoll->x = ATOFF; NEXT
	yawPitchRoll->y = ATOFF; NEXT
	yawPitchRoll->z = ATOFF; NEXT
	magnetic->x = ATOFF; NEXT
	magnetic->y = ATOFF; NEXT
	magnetic->z = ATOFF; NEXT
	acceleration->x = ATOFF; NEXT
	acceleration->y = ATOFF; NEXT
	acceleration->z = ATOFF; NEXT
	angularRate->x = ATOFF; NEXT
	angularRate->y = ATOFF; NEXT
	angularRate->z = ATOFF;
}

#ifdef EXTRA

void VnUartPacket_parseVNYCM(VnUartPacket* packet, vec3f* yawPitchRoll, vec3f* magnetic, vec3f* acceleration, vec3f* angularRate, float* temperature)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	yawPitchRoll->x = ATOFF; NEXT
	yawPitchRoll->y = ATOFF; NEXT
	yawPitchRoll->z = ATOFF; NEXT
	magnetic->x = ATOFF; NEXT
	magnetic->y = ATOFF; NEXT
	magnetic->z = ATOFF; NEXT
	acceleration->x = ATOFF; NEXT
	acceleration->y = ATOFF; NEXT
	acceleration->z = ATOFF; NEXT
	angularRate->x = ATOFF; NEXT
	angularRate->y = ATOFF; NEXT
	angularRate->z = ATOFF; NEXT
	*temperature = ATOFF;
}

#endif

void VnUartPacket_parseVNYBA(VnUartPacket* packet, vec3f* yawPitchRoll, vec3f* accelerationBody, vec3f* angularRate)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	yawPitchRoll->x = ATOFF; NEXT
	yawPitchRoll->y = ATOFF; NEXT
	yawPitchRoll->z = ATOFF; NEXT
	accelerationBody->x = ATOFF; NEXT
	accelerationBody->y = ATOFF; NEXT
	accelerationBody->z = ATOFF; NEXT
	angularRate->x = ATOFF; NEXT
	angularRate->y = ATOFF; NEXT
	angularRate->z = ATOFF;
}

void VnUartPacket_parseVNYIA(VnUartPacket* packet, vec3f* yawPitchRoll, vec3f* accelerationInertial, vec3f* angularRate)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	yawPitchRoll->x = ATOFF; NEXT
	yawPitchRoll->y = ATOFF; NEXT
	yawPitchRoll->z = ATOFF; NEXT
	accelerationInertial->x = ATOFF; NEXT
	accelerationInertial->y = ATOFF; NEXT
	accelerationInertial->z = ATOFF; NEXT
	angularRate->x = ATOFF; NEXT
	angularRate->y = ATOFF; NEXT
	angularRate->z = ATOFF;
}

#ifdef EXTRA

void VnUartPacket_parseVNICM(VnUartPacket* packet, vec3f* yawPitchRoll, vec3f* magnetic, vec3f* accelerationInertial, vec3f* angularRate)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	yawPitchRoll->x = ATOFF; NEXT
	yawPitchRoll->y = ATOFF; NEXT
	yawPitchRoll->z = ATOFF; NEXT
	magnetic->x = ATOFF; NEXT
	magnetic->y = ATOFF; NEXT
	magnetic->z = ATOFF; NEXT
	accelerationInertial->x = ATOFF; NEXT
	accelerationInertial->y = ATOFF; NEXT
	accelerationInertial->z = ATOFF; NEXT
	angularRate->x = ATOFF; NEXT
	angularRate->y = ATOFF; NEXT
	angularRate->z = ATOFF;
}

#endif

void VnUartPacket_parseVNIMU(VnUartPacket* packet, vec3f* magneticUncompensated, vec3f* accelerationUncompensated, vec3f* angularRateUncompensated, float* temperature, float* pressure)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	magneticUncompensated->x = ATOFF; NEXT
	magneticUncompensated->y = ATOFF; NEXT
	magneticUncompensated->z = ATOFF; NEXT
	accelerationUncompensated->x = ATOFF; NEXT
	accelerationUncompensated->y = ATOFF; NEXT
	accelerationUncompensated->z = ATOFF; NEXT
	angularRateUncompensated->x = ATOFF; NEXT
	angularRateUncompensated->y = ATOFF; NEXT
	angularRateUncompensated->z = ATOFF; NEXT
	*temperature = ATOFF; NEXT
	*pressure = ATOFF;
}

void VnUartPacket_parseVNGPS(VnUartPacket* packet, double* time, uint16_t* week, uint8_t* gpsFix, uint8_t* numSats, vec3d* lla, vec3f* nedVel, vec3f* nedAcc, float* speedAcc, float* timeAcc)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	*time = ATOFD; NEXT
	*week = (uint16_t) atoi(result); NEXT
	*gpsFix = (uint8_t) atoi(result); NEXT
	*numSats = (uint8_t) atoi(result); NEXT
	lla->x = ATOFD; NEXT
	lla->y = ATOFD; NEXT
	lla->z = ATOFD; NEXT
	nedVel->x = ATOFF; NEXT
	nedVel->y = ATOFF; NEXT
	nedVel->z = ATOFF; NEXT
	nedAcc->x = ATOFF; NEXT
	nedAcc->y = ATOFF; NEXT
	nedAcc->z = ATOFF; NEXT
	*speedAcc = ATOFF; NEXT
	*timeAcc = ATOFF;
}

void VnUartPacket_parseVNINS(VnUartPacket* packet, double* time, uint16_t* week, uint16_t* status, vec3f* yawPitchRoll, vec3d* lla, vec3f* nedVel, float* attUncertainty, float* posUncertainty, float* velUncertainty)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	*time = ATOFD; NEXT
	*week = (uint16_t) atoi(result); NEXT
	*status = (uint16_t) atoi(result); NEXT
	yawPitchRoll->x = ATOFF; NEXT
	yawPitchRoll->y = ATOFF; NEXT
	yawPitchRoll->z = ATOFF; NEXT
	lla->x = ATOFD; NEXT
	lla->y = ATOFD; NEXT
	lla->z = ATOFD; NEXT
	nedVel->x = ATOFF; NEXT
	nedVel->y = ATOFF; NEXT
	nedVel->z = ATOFF; NEXT
	*attUncertainty = ATOFF; NEXT
	*posUncertainty = ATOFF; NEXT
	*velUncertainty = ATOFF;
}

void VnUartPacket_parseVNINE(VnUartPacket* packet, double* time, uint16_t* week, uint16_t* status, vec3f* yawPitchRoll, vec3d* position, vec3f* velocity, float* attUncertainty, float* posUncertainty, float* velUncertainty)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	*time = ATOFD; NEXT
	*week = (uint16_t) atoi(result); NEXT
	*status = (uint16_t) atoi(result); NEXT
	yawPitchRoll->x = ATOFF; NEXT
	yawPitchRoll->y = ATOFF; NEXT
	yawPitchRoll->z = ATOFF; NEXT
	position->x = ATOFD; NEXT
	position->y = ATOFD; NEXT
	position->z = ATOFD; NEXT
	velocity->x = ATOFF; NEXT
	velocity->y = ATOFF; NEXT
	velocity->z = ATOFF; NEXT
	*attUncertainty = ATOFF; NEXT
	*posUncertainty = ATOFF; NEXT
	*velUncertainty = ATOFF;
}

void VnUartPacket_parseVNISL(VnUartPacket* packet, vec3f* ypr, vec3d* lla, vec3f* velocity, vec3f* acceleration, vec3f* angularRate)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	ypr->x = ATOFF; NEXT
	ypr->y = ATOFF; NEXT
	ypr->z = ATOFF; NEXT
	lla->x = ATOFD; NEXT
	lla->y = ATOFD; NEXT
	lla->z = ATOFD; NEXT
	velocity->x = ATOFF; NEXT
	velocity->y = ATOFF; NEXT
	velocity->z = ATOFF; NEXT
	acceleration->x = ATOFF; NEXT
	acceleration->y = ATOFF; NEXT
	acceleration->z = ATOFF; NEXT
	angularRate->x = ATOFF; NEXT
	angularRate->y = ATOFF; NEXT
	angularRate->z = ATOFF;
}

void VnUartPacket_parseVNISE(VnUartPacket* packet, vec3f* ypr, vec3d* position, vec3f* velocity, vec3f* acceleration, vec3f* angularRate)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	ypr->x = ATOFF; NEXT
	ypr->y = ATOFF; NEXT
	ypr->z = ATOFF; NEXT
	position->x = ATOFD; NEXT
	position->y = ATOFD; NEXT
	position->z = ATOFD; NEXT
	velocity->x = ATOFF; NEXT
	velocity->y = ATOFF; NEXT
	velocity->z = ATOFF; NEXT
	acceleration->x = ATOFF; NEXT
	acceleration->y = ATOFF; NEXT
	acceleration->z = ATOFF; NEXT
	angularRate->x = ATOFF; NEXT
	angularRate->y = ATOFF; NEXT
	angularRate->z = ATOFF;
}

#ifdef EXTRA

void VnUartPacket_parseVNRAW(VnUartPacket* packet, vec3f *magneticVoltage, vec3f *accelerationVoltage, vec3f *angularRateVoltage, float* temperatureVoltage)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	magneticVoltage->x = ATOFF; NEXT
	magneticVoltage->y = ATOFF; NEXT
	magneticVoltage->z = ATOFF; NEXT
	accelerationVoltage->x = ATOFF; NEXT
	accelerationVoltage->y = ATOFF; NEXT
	accelerationVoltage->z = ATOFF; NEXT
	angularRateVoltage->x = ATOFF; NEXT
	angularRateVoltage->y = ATOFF; NEXT
	angularRateVoltage->z = ATOFF; NEXT
	*temperatureVoltage = ATOFF;
}

void VnUartPacket_parseVNCMV(VnUartPacket* packet, vec3f* magneticUncompensated, vec3f* accelerationUncompensated, vec3f* angularRateUncompensated, float* temperature)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	magneticUncompensated->x = ATOFF; NEXT
	magneticUncompensated->y = ATOFF; NEXT
	magneticUncompensated->z = ATOFF; NEXT
	accelerationUncompensated->x = ATOFF; NEXT
	accelerationUncompensated->y = ATOFF; NEXT
	accelerationUncompensated->z = ATOFF; NEXT
	angularRateUncompensated->x = ATOFF; NEXT
	angularRateUncompensated->y = ATOFF; NEXT
	angularRateUncompensated->z = ATOFF; NEXT
	*temperature = ATOFF;
}

void VnUartPacket_parseVNSTV(VnUartPacket* packet, vec4f* quaternion, vec3f* angularRateBias)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	quaternion->x = ATOFF; NEXT
	quaternion->y = ATOFF; NEXT
	quaternion->z = ATOFF; NEXT
	quaternion->w = ATOFF; NEXT
	angularRateBias->x = ATOFF; NEXT
	angularRateBias->y = ATOFF; NEXT
	angularRateBias->z = ATOFF;
}

void VnUartPacket_parseVNCOV(VnUartPacket* packet, vec3f* attitudeVariance, vec3f* angularRateBiasVariance)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	attitudeVariance->x = ATOFF; NEXT
	attitudeVariance->y = ATOFF; NEXT
	attitudeVariance->z = ATOFF; NEXT
	angularRateBiasVariance->x = ATOFF; NEXT
	angularRateBiasVariance->y = ATOFF; NEXT
	angularRateBiasVariance->z = ATOFF;
}

#endif

void VnUartPacket_parseVNGPE(VnUartPacket* packet, double* tow, uint16_t* week, uint8_t* gpsFix, uint8_t* numSats, vec3d* position, vec3f* velocity, vec3f* posAcc, float* speedAcc, float* timeAcc)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	*tow = ATOFD; NEXT
	*week = (uint16_t) atoi(result); NEXT
	*gpsFix = (uint8_t) atoi(result); NEXT
	*numSats = (uint8_t) atoi(result); NEXT
	position->x = ATOFD; NEXT
	position->y = ATOFD; NEXT
	position->z = ATOFD; NEXT
	velocity->x = ATOFF; NEXT
	velocity->y = ATOFF; NEXT
	velocity->z = ATOFF; NEXT
	posAcc->x = ATOFF; NEXT
	posAcc->y = ATOFF; NEXT
	posAcc->z = ATOFF; NEXT
	*speedAcc = ATOFF; NEXT
	*timeAcc = ATOFF;
}

void VnUartPacket_parseVNDTV(VnUartPacket* packet, float* deltaTime, vec3f* deltaTheta, vec3f* deltaVelocity)
{
	size_t packetIndex;

	char* result = VnUartPacket_startAsciiPacketParse(packet->data, &packetIndex);

	*deltaTime = ATOFF; NEXT
	deltaTheta->x = ATOFF; NEXT
	deltaTheta->y = ATOFF; NEXT
	deltaTheta->z = ATOFF; NEXT
	deltaVelocity->x = ATOFF; NEXT
	deltaVelocity->y = ATOFF; NEXT
	deltaVelocity->z = ATOFF;
}

VnError VnUartPacket_genWrite(
    char *buffer,
    size_t bufferSize,
    VnErrorDetectionMode errorDetectionMode,
    uint16_t registerId,
    size_t *cmdSize,
	char const *format,
    ...)
{
    va_list ap;
	char *curOutputLoc = buffer;
	char const *curFormatPos = format;

	#if defined(_MSC_VER)
		/* Disable warnings regarding using strcpy_s since this
		 * function's signature does not provide us with information
		 * about the length of 'out'. */
		#pragma warning(push)
		#pragma warning(disable:4996)
	#endif

	/* Add the message header and register number. */
	curOutputLoc += sprintf(curOutputLoc, "$VNWRG,%d", registerId);

	va_start(ap, format);

	/* Now cycle through the provided format specifier. */
	while (*curFormatPos != '\0')
	{
		switch (*curFormatPos++)
		{
		case 'U':

			switch (*curFormatPos++)
			{
			case '1':
				/* 'uint8_t' is promoted to 'int' when passed through '...'. */
				curOutputLoc += sprintf(curOutputLoc, ",%d", va_arg(ap, int));
				break;

			case '2':
				/* 'uint16_t' is promoted to 'int' when passed through '...'. */
				curOutputLoc += sprintf(curOutputLoc, ",%d", va_arg(ap, int));
				break;

			case '4':
				curOutputLoc += sprintf(curOutputLoc, ",%d", va_arg(ap, uint32_t));
				break;
			}

			break;

		case 'F':

			switch (*curFormatPos++)
			{
			case '4':
				/* 'float' is promoted to 'double' when passed through '...'. */
				curOutputLoc += sprintf(curOutputLoc, ",%f", va_arg(ap, double));
				break;

			case '8':
				curOutputLoc += sprintf(curOutputLoc, ",%f", va_arg(ap, double));
				break;
			}

			break;
		}
	}

	#if defined(_MSC_VER)
		#pragma warning(pop)
	#endif

	va_end(ap);

	/* Compute and append the checksum. */
	*curOutputLoc++ = '*';

	if (errorDetectionMode == VNERRORDETECTIONMODE_NONE)
	{
		*curOutputLoc++ = 'X';
		*curOutputLoc++ = 'X';
	}
	else if (errorDetectionMode == VNERRORDETECTIONMODE_CHECKSUM)
	{
		uint8_t checksum = VnChecksum8_compute(buffer + 1, curOutputLoc - buffer - 2);
		VnUtil_toHexStrFromUint8(checksum, curOutputLoc);

		curOutputLoc += 2;
	}
	else if (errorDetectionMode == VNERRORDETECTIONMODE_CRC)
	{
		uint16_t crc = VnCrc16_compute(buffer + 1, curOutputLoc - buffer - 2);
		VnUtil_toHexStrFromUint16(crc, curOutputLoc);

		curOutputLoc += 4;
	}
	else
	{
		return E_NOT_SUPPORTED;
	}

	*curOutputLoc++ = '\r';
	*curOutputLoc++ = '\n';

	*cmdSize = curOutputLoc - buffer;

	return E_NONE;
}

VnError VnUartPacket_genReadBinaryOutput1(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,75");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,75");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadBinaryOutput2(
		char *buffer,
		size_t bufferSize,
		VnErrorDetectionMode errorDetectionMode,
		size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,76");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,76");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadBinaryOutput3(
		char *buffer,
		size_t bufferSize,
		VnErrorDetectionMode errorDetectionMode,
		size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,77");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,77");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

#ifdef EXTRA

VnError VnUartPacket_genReadBinaryOutput4(
		char *buffer,
		size_t bufferSize,
		VnErrorDetectionMode errorDetectionMode,
		size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,78");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,78");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadBinaryOutput5(
		char *buffer,
		size_t bufferSize,
		VnErrorDetectionMode errorDetectionMode,
		size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,79");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,79");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

#endif

VnError VnUartPacket_finalizeCommand(VnErrorDetectionMode errorDetectionMode, char *packet, size_t *length)
{
	#if defined(_MSC_VER)
		/* Disable warnings regarding using sprintf_s. */
		#pragma warning(push)
		#pragma warning(disable:4996)
	#endif

	if (errorDetectionMode == VNERRORDETECTIONMODE_CHECKSUM)
	{
		*length += sprintf(packet + *length, "*%02X\r\n", VnChecksum8_compute(packet + 1, *length - 1));
	}
	else if (errorDetectionMode == VNERRORDETECTIONMODE_CRC)
	{
		*length += sprintf(packet + *length, "*%04X\r\n", VnCrc16_compute(packet + 1, *length - 1));
	}
	else
	{
		*length += sprintf(packet + *length, "*XX\r\n");
	}

	#if defined(_MSC_VER)
		#pragma warning(pop)
	#endif

	return E_NONE;
}

VnError VnUartPacket_genCmdWriteSettings(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNWNV");
	#else
	*cmdSize = sprintf(buffer, "$VNWNV");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genCmdRestoreFactorySettings(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRFS");
	#else
	*cmdSize = sprintf(buffer, "$VNRFS");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genCmdReset(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRST");
	#else
	*cmdSize = sprintf(buffer, "$VNRST");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genCmdTare(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNTAR");
	#else
	*cmdSize = sprintf(buffer, "$VNTAR");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genCmdSetGyroBias(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNSGB");
	#else
	*cmdSize = sprintf(buffer, "$VNSGB");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genCmdKnownMagneticDisturbance(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	bool disturbancePresent,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNKMD,%d", disturbancePresent ? 1 : 0);
	#else
	*cmdSize = sprintf(buffer, "$VNKMD,%d", disturbancePresent ? 1 : 0);
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genCmdKnownAccelerationDisturbance(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	bool disturbancePresent,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNKAD,%d", disturbancePresent ? 1 : 0);
	#else
	*cmdSize = sprintf(buffer, "$VNKAD,%d", disturbancePresent ? 1 : 0);
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadUserTag(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,00");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,00");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadModelNumber(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,01");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,01");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadHardwareRevision(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,02");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,02");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadSerialNumber(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,03");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,03");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadFirmwareVersion(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,04");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,04");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadSerialBaudRate(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,05");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,05");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadAsyncDataOutputType(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,06");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,06");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadAsyncDataOutputFrequency(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,07");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,07");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadYawPitchRoll(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,08");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,08");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadAttitudeQuaternion(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,09");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,09");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadQuaternionMagneticAccelerationAndAngularRates(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,15");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,15");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadMagneticMeasurements(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,17");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,17");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadAccelerationMeasurements(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,18");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,18");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadAngularRateMeasurements(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,19");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,19");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadMagneticAccelerationAndAngularRates(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,20");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,20");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadMagneticAndGravityReferenceVectors(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,21");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,21");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadMagnetometerCompensation(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,23");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,23");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadAccelerationCompensation(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,25");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,25");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadReferenceFrameRotation(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,26");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,26");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadYawPitchRollMagneticAccelerationAndAngularRates(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,27");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,27");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadCommunicationProtocolControl(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,30");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,30");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadSynchronizationControl(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,32");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,32");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadSynchronizationStatus(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,33");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,33");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadVpeBasicControl(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,35");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,35");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadVpeMagnetometerBasicTuning(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,36");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,36");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadVpeAccelerometerBasicTuning(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,38");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,38");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadMagnetometerCalibrationControl(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,44");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,44");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadCalculatedMagnetometerCalibration(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,47");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,47");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadVelocityCompensationMeasurement(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,50");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,50");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadVelocityCompensationControl(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,51");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,51");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadImuMeasurements(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,54");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,54");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadGpsConfiguration(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,55");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,55");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadGpsAntennaOffset(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,57");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,57");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadGpsSolutionLla(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,58");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,58");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadGpsSolutionEcef(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,59");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,59");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadInsSolutionLla(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,63");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,63");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadInsSolutionEcef(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,64");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,64");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadInsBasicConfiguration(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,67");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,67");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadInsStateLla(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,72");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,72");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadInsStateEcef(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,73");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,73");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadStartupFilterBiasEstimate(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,74");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,74");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadDeltaThetaAndDeltaVelocity(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,80");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,80");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadDeltaThetaAndDeltaVelocityConfiguration(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,82");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,82");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadReferenceVectorConfiguration(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,83");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,83");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadGyroCompensation(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,84");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,84");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadImuFilteringConfiguration(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,85");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,85");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadGpsCompassBaseline(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,93");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,93");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadGpsCompassEstimatedBaseline(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,97");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,97");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadYawPitchRollTrueBodyAccelerationAndAngularRates(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,239");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,239");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genReadYawPitchRollTrueInertialAccelerationAndAngularRates(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize)
{
	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNRRG,240");
	#else
	*cmdSize = sprintf(buffer, "$VNRRG,240");
	#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genWriteBinaryOutput(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint8_t binaryOutputNumber,
	uint16_t asyncMode,
	uint16_t rateDivisor,
	uint16_t commonField,
	uint16_t timeField,
	uint16_t imuField,
	uint16_t gpsField,
	uint16_t attitudeField,
	uint16_t insField)
{
	uint16_t groups = 0;

	/* First determine which groups are present. */
	if (commonField)
		groups |= 0x0001;
	if (timeField)
		groups |= 0x0002;
	if (imuField)
		groups |= 0x0004;
	if (gpsField)
		groups |= 0x0008;
	if (attitudeField)
		groups |= 0x0010;
	if (insField)
		groups |= 0x0020;

	#if VN_HAVE_SECURE_CRT
	*cmdSize = sprintf_s(buffer, bufferSize, "$VNWRG,%u,%u,%u,%X", 74 + binaryOutputNumber, asyncMode, rateDivisor, groups);
	#else
	*cmdSize = sprintf(buffer, "$VNWRG,%u,%u,%u,%X", 74 + binaryOutputNumber, asyncMode, rateDivisor, groups);
	#endif

	if (commonField)
		#if VN_HAVE_SECURE_CRT
		*cmdSize += sprintf_s(buffer + *cmdSize, bufferSize - *cmdSize, ",%X", commonField);
		#else
		*cmdSize += sprintf(buffer + *cmdSize, ",%X", commonField);
		#endif
	if (timeField)
		#if VN_HAVE_SECURE_CRT
		*cmdSize += sprintf_s(buffer + *cmdSize, bufferSize - *cmdSize, ",%X", timeField);
		#else
		*cmdSize += sprintf(buffer + *cmdSize, ",%X", timeField);
		#endif
	if (imuField)
		#if VN_HAVE_SECURE_CRT
		*cmdSize += sprintf_s(buffer + *cmdSize, bufferSize - *cmdSize, ",%X", imuField);
		#else
		*cmdSize += sprintf(buffer + *cmdSize, ",%X", imuField);
		#endif
	if (gpsField)
		#if VN_HAVE_SECURE_CRT
		*cmdSize += sprintf_s(buffer + *cmdSize, bufferSize - *cmdSize, ",%X", gpsField);
		#else
		*cmdSize += sprintf(buffer + *cmdSize, ",%X", gpsField);
		#endif
	if (attitudeField)
		#if VN_HAVE_SECURE_CRT
		*cmdSize += sprintf_s(buffer + *cmdSize, bufferSize - *cmdSize, ",%X", attitudeField);
		#else
		*cmdSize += sprintf(buffer + *cmdSize, ",%X", attitudeField);
		#endif
	if (insField)
		#if VN_HAVE_SECURE_CRT
		*cmdSize += sprintf_s(buffer + *cmdSize, bufferSize - *cmdSize, ",%X", insField);
		#else
		*cmdSize += sprintf(buffer + *cmdSize, ",%X", insField);
		#endif

	return VnUartPacket_finalizeCommand(errorDetectionMode, buffer, cmdSize);
}

VnError VnUartPacket_genWriteBinaryOutput1(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint16_t asyncMode,
	uint16_t rateDivisor,
	uint16_t commonField,
	uint16_t timeField,
	uint16_t imuField,
	uint16_t gpsField,
	uint16_t attitudeField,
	uint16_t insField)
{
	return VnUartPacket_genWriteBinaryOutput(
		buffer,
		bufferSize,
		errorDetectionMode,
		cmdSize,
		1,
		asyncMode,
		rateDivisor,
		commonField,
		timeField,
		imuField,
		gpsField,
		attitudeField,
		insField);
}

VnError VnUartPacket_genWriteBinaryOutput2(
		char *buffer,
		size_t bufferSize,
		VnErrorDetectionMode errorDetectionMode,
		size_t *cmdSize,
		uint16_t asyncMode,
		uint16_t rateDivisor,
		uint16_t commonField,
		uint16_t timeField,
		uint16_t imuField,
		uint16_t gpsField,
		uint16_t attitudeField,
		uint16_t insField)
{
	return VnUartPacket_genWriteBinaryOutput(
			buffer,
			bufferSize,
			errorDetectionMode,
			cmdSize,
			2,
			asyncMode,
			rateDivisor,
			commonField,
			timeField,
			imuField,
			gpsField,
			attitudeField,
			insField);
}

VnError VnUartPacket_genWriteBinaryOutput3(
		char *buffer,
		size_t bufferSize,
		VnErrorDetectionMode errorDetectionMode,
		size_t *cmdSize,
		uint16_t asyncMode,
		uint16_t rateDivisor,
		uint16_t commonField,
		uint16_t timeField,
		uint16_t imuField,
		uint16_t gpsField,
		uint16_t attitudeField,
		uint16_t insField)
{
	return VnUartPacket_genWriteBinaryOutput(
			buffer,
			bufferSize,
			errorDetectionMode,
			cmdSize,
			3,
			asyncMode,
			rateDivisor,
			commonField,
			timeField,
			imuField,
			gpsField,
			attitudeField,
			insField);
}

#ifdef EXTRA

VnError VnUartPacket_genWriteBinaryOutput4(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint16_t asyncMode,
	uint16_t rateDivisor,
	uint16_t commonField,
	uint16_t timeField,
	uint16_t imuField,
	uint16_t gpsField,
	uint16_t attitudeField,
	uint16_t insField)
{
	return VnUartPacket_genWriteBinaryOutput(
		buffer,
		bufferSize,
		errorDetectionMode,
		cmdSize,
		4,
		asyncMode,
		rateDivisor,
		commonField,
		timeField,
		imuField,
		gpsField,
		attitudeField,
		insField);
}

VnError VnUartPacket_genWriteBinaryOutput5(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint16_t asyncMode,
	uint16_t rateDivisor,
	uint16_t commonField,
	uint16_t timeField,
	uint16_t imuField,
	uint16_t gpsField,
	uint16_t attitudeField,
	uint16_t insField)
{
	return VnUartPacket_genWriteBinaryOutput(
		buffer,
		bufferSize,
		errorDetectionMode,
		cmdSize,
		5,
		asyncMode,
		rateDivisor,
		commonField,
		timeField,
		imuField,
		gpsField,
		attitudeField,
		insField);
}

#endif

VnError VnUartPacket_genWriteUserTag(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	char* tag)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		0,
		cmdSize,
		"ST",
		tag);
}

VnError VnUartPacket_genWriteSerialBaudRate(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint32_t baudrate)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		5,
		cmdSize,
		"U4",
		baudrate);
}

VnError VnUartPacket_genWriteSerialBaudRate_with_options(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint32_t baudrate)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		5,
		cmdSize,
		"U4",
		baudrate);
}

VnError VnUartPacket_genWriteAsyncDataOutputType(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint32_t ador)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		6,
		cmdSize,
		"U4",
		ador);
}

VnError VnUartPacket_genWriteAsyncDataOutputType_with_options(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint32_t ador)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		6,
		cmdSize,
		"U4",
		ador);
}

VnError VnUartPacket_genWriteAsyncDataOutputFrequency(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint32_t adof)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		7,
		cmdSize,
		"U4",
		adof);
}

VnError VnUartPacket_genWriteAsyncDataOutputFrequency_with_options(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint32_t adof)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		7,
		cmdSize,
		"U4",
		adof);
}

VnError VnUartPacket_genWriteMagneticAndGravityReferenceVectors(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	vec3f magRef,
	vec3f accRef)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		21,
		cmdSize,
		"F4F4F4F4F4F4",
		magRef.x,
		magRef.y,
		magRef.z,
		accRef.x,
		accRef.y,
		accRef.z);
}

VnError VnUartPacket_genWriteMagnetometerCompensation(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	mat3f c,
	vec3f b)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		23,
		cmdSize,
		"F4F4F4F4F4F4F4F4F4F4F4F4",
		c.e00,
		c.e01,
		c.e02,
		c.e10,
		c.e11,
		c.e12,
		c.e20,
		c.e21,
		c.e22,
		b.x,
		b.y,
		b.z);
}

VnError VnUartPacket_genWriteAccelerationCompensation(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	mat3f c,
	vec3f b)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		25,
		cmdSize,
		"F4F4F4F4F4F4F4F4F4F4F4F4",
		c.e00,
		c.e01,
		c.e02,
		c.e10,
		c.e11,
		c.e12,
		c.e20,
		c.e21,
		c.e22,
		b.x,
		b.y,
		b.z);
}

VnError VnUartPacket_genWriteReferenceFrameRotation(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	mat3f c)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		26,
		cmdSize,
		"F4F4F4F4F4F4F4F4F4",
		c.e00,
		c.e01,
		c.e02,
		c.e10,
		c.e11,
		c.e12,
		c.e20,
		c.e21,
		c.e22);
}

VnError VnUartPacket_genWriteCommunicationProtocolControl(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint8_t serialCount,
	uint8_t serialStatus,
	uint8_t spiCount,
	uint8_t spiStatus,
	uint8_t serialChecksum,
	uint8_t spiChecksum,
	uint8_t errorMode)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		30,
		cmdSize,
		"U1U1U1U1U1U1U1",
		serialCount,
		serialStatus,
		spiCount,
		spiStatus,
		serialChecksum,
		spiChecksum,
		errorMode);
}

VnError VnUartPacket_genWriteSynchronizationControl(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint8_t syncInMode,
	uint8_t syncInEdge,
	uint16_t syncInSkipFactor,
	uint32_t reserved1,
	uint8_t syncOutMode,
	uint8_t syncOutPolarity,
	uint16_t syncOutSkipFactor,
	uint32_t syncOutPulseWidth,
	uint32_t reserved2)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		32,
		cmdSize,
		"U1U1U2U4U1U1U2U4U4",
		syncInMode,
		syncInEdge,
		syncInSkipFactor,
		reserved1,
		syncOutMode,
		syncOutPolarity,
		syncOutSkipFactor,
		syncOutPulseWidth,
		reserved2);
}

VnError VnUartPacket_genWriteSynchronizationStatus(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint32_t syncInCount,
	uint32_t syncInTime,
	uint32_t syncOutCount)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		33,
		cmdSize,
		"U4U4U4",
		syncInCount,
		syncInTime,
		syncOutCount);
}

VnError VnUartPacket_genWriteVpeBasicControl(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint8_t enable,
	uint8_t headingMode,
	uint8_t filteringMode,
	uint8_t tuningMode)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		35,
		cmdSize,
		"U1U1U1U1",
		enable,
		headingMode,
		filteringMode,
		tuningMode);
}

VnError VnUartPacket_genWriteVpeMagnetometerBasicTuning(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	vec3f baseTuning,
	vec3f adaptiveTuning,
	vec3f adaptiveFiltering)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		36,
		cmdSize,
		"F4F4F4F4F4F4F4F4F4",
		baseTuning.x,
		baseTuning.y,
		baseTuning.z,
		adaptiveTuning.x,
		adaptiveTuning.y,
		adaptiveTuning.z,
		adaptiveFiltering.x,
		adaptiveFiltering.y,
		adaptiveFiltering.z);
}

VnError VnUartPacket_genWriteVpeAccelerometerBasicTuning(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	vec3f baseTuning,
	vec3f adaptiveTuning,
	vec3f adaptiveFiltering)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		38,
		cmdSize,
		"F4F4F4F4F4F4F4F4F4",
		baseTuning.x,
		baseTuning.y,
		baseTuning.z,
		adaptiveTuning.x,
		adaptiveTuning.y,
		adaptiveTuning.z,
		adaptiveFiltering.x,
		adaptiveFiltering.y,
		adaptiveFiltering.z);
}

VnError VnUartPacket_genWriteMagnetometerCalibrationControl(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint8_t hsiMode,
	uint8_t hsiOutput,
	uint8_t convergeRate)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		44,
		cmdSize,
		"U1U1U1",
		hsiMode,
		hsiOutput,
		convergeRate);
}

VnError VnUartPacket_genWriteVelocityCompensationMeasurement(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	vec3f velocity)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		50,
		cmdSize,
		"F4F4F4",
		velocity.x,
		velocity.y,
		velocity.z);
}

VnError VnUartPacket_genWriteVelocityCompensationControl(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint8_t mode,
	float velocityTuning,
	float rateTuning)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		51,
		cmdSize,
		"U1F4F4",
		mode,
		velocityTuning,
		rateTuning);
}

VnError VnUartPacket_genWriteGpsConfiguration(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint8_t mode,
	uint8_t ppsSource,
	uint8_t reserved1,
	uint8_t reserved2,
	uint8_t reserved3)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		55,
		cmdSize,
		"U1U1U1U1U1",
		mode,
		ppsSource,
		reserved1,
		reserved2,
		reserved3);
}

VnError VnUartPacket_genWriteGpsAntennaOffset(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	vec3f position)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		57,
		cmdSize,
		"F4F4F4",
		position.x,
		position.y,
		position.z);
}

VnError VnUartPacket_genWriteInsBasicConfiguration(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint8_t scenario,
	uint8_t ahrsAiding,
	uint8_t estBaseline,
	uint8_t resv2)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		67,
		cmdSize,
		"U1U1U1U1",
		scenario,
		ahrsAiding,
		estBaseline,
		resv2);
}

VnError VnUartPacket_genWriteStartupFilterBiasEstimate(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	vec3f gyroBias,
	vec3f accelBias,
	float pressureBias)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		74,
		cmdSize,
		"F4F4F4F4F4F4F4",
		gyroBias.x,
		gyroBias.y,
		gyroBias.z,
		accelBias.x,
		accelBias.y,
		accelBias.z,
		pressureBias);
}

VnError VnUartPacket_genWriteDeltaThetaAndDeltaVelocityConfiguration(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint8_t integrationFrame,
	uint8_t gyroCompensation,
	uint8_t accelCompensation,
	uint8_t reserved1,
	uint16_t reserved2)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		82,
		cmdSize,
		"U1U1U1U1U2",
		integrationFrame,
		gyroCompensation,
		accelCompensation,
		reserved1,
		reserved2);
}

VnError VnUartPacket_genWriteReferenceVectorConfiguration(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint8_t useMagModel,
	uint8_t useGravityModel,
	uint8_t resv1,
	uint8_t resv2,
	uint32_t recalcThreshold,
	float year,
	vec3d position)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		83,
		cmdSize,
		"U1U1U1U1U4F4F8F8F8",
		useMagModel,
		useGravityModel,
		resv1,
		resv2,
		recalcThreshold,
		year,
		position.x,
		position.y,
		position.z);
}

VnError VnUartPacket_genWriteGyroCompensation(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	mat3f c,
	vec3f b)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		84,
		cmdSize,
		"F4F4F4F4F4F4F4F4F4F4F4F4",
		c.e00,
		c.e01,
		c.e02,
		c.e10,
		c.e11,
		c.e12,
		c.e20,
		c.e21,
		c.e22,
		b.x,
		b.y,
		b.z);
}

VnError VnUartPacket_genWriteImuFilteringConfiguration(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	uint16_t magWindowSize,
	uint16_t accelWindowSize,
	uint16_t gyroWindowSize,
	uint16_t tempWindowSize,
	uint16_t presWindowSize,
	uint8_t magFilterMode,
	uint8_t accelFilterMode,
	uint8_t gyroFilterMode,
	uint8_t tempFilterMode,
	uint8_t presFilterMode)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		85,
		cmdSize,
		"U2U2U2U2U2U1U1U1U1U1",
		magWindowSize,
		accelWindowSize,
		gyroWindowSize,
		tempWindowSize,
		presWindowSize,
		magFilterMode,
		accelFilterMode,
		gyroFilterMode,
		tempFilterMode,
		presFilterMode);
}

VnError VnUartPacket_genWriteGpsCompassBaseline(
	char *buffer,
	size_t bufferSize,
	VnErrorDetectionMode errorDetectionMode,
	size_t *cmdSize,
	vec3f position,
	vec3f uncertainty)
{
	return VnUartPacket_genWrite(
		buffer,
		bufferSize,
		errorDetectionMode,
		93,
		cmdSize,
		"F4F4F4F4F4F4",
		position.x,
		position.y,
		position.z,
		uncertainty.x,
		uncertainty.y,
		uncertainty.z);
}

void VnUartPacket_parseError(VnUartPacket *packet, uint8_t *error)
{
	VnUartPacket_parseErrorRaw(packet->data, error);
}

void VnUartPacket_parseErrorRaw(char *packet, uint8_t *error)
{
	*error = VnUtil_toUint8FromHexStr(packet + 7);
}

void VnUartPacket_parseBinaryOutput(
	VnUartPacket *packet,
	uint16_t* asyncMode,
	uint16_t* rateDivisor,
	uint16_t* outputGroup,
	uint16_t* commonField,
	uint16_t* timeField,
	uint16_t* imuField,
	uint16_t* gpsField,
	uint16_t* attitudeField,
	uint16_t* insField)
{
	VnUartPacket_parseBinaryOutputRaw(
		packet->data,
		asyncMode,
		rateDivisor,
		outputGroup,
		commonField,
		timeField,
		imuField,
		gpsField,
		attitudeField,
		insField);
}

void VnUartPacket_parseBinaryOutputRaw(
	char *packet,
	uint16_t* asyncMode,
	uint16_t* rateDivisor,
	uint16_t* outputGroup,
	uint16_t* commonField,
	uint16_t* timeField,
	uint16_t* imuField,
	uint16_t* gpsField,
	uint16_t* attitudeField,
	uint16_t* insField)
{
	size_t packetIndex;

	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*commonField = 0;
	*timeField = 0;
	*imuField = 0;
	*gpsField = 0;
	*attitudeField = 0;
	*insField = 0;

	*asyncMode = ATOU16;
	NEXTRAW
	*rateDivisor = ATOU16;
	NEXTRAW
	*outputGroup = ATOU16;
	if (*outputGroup & 0x0001)
	{
		NEXTRAW
		*commonField = ATOU16;
	}
	if (*outputGroup & 0x0002)
	{
		NEXTRAW
		*timeField = ATOU16;
	}
	if (*outputGroup & 0x0004)
	{
		NEXTRAW
		*imuField = ATOU16;
	}
	if (*outputGroup & 0x0008)
	{
		NEXTRAW
		*gpsField = ATOU16;
	}
	if (*outputGroup & 0x0010)
	{
		NEXTRAW
		*attitudeField = ATOU16;
	}
	if (*outputGroup & 0x0020)
	{
		NEXTRAW
		*insField = ATOU16;
	}
}

void VnUartPacket_parseUserTag(VnUartPacket *packet, char *tag)
{
	VnUartPacket_parseUserTagRaw(packet->data, tag);
}

void VnUartPacket_parseUserTagRaw(char *packet, char* tag)
{
	size_t packetIndex;
	char* next;

	next = VnUartPacket_startAsciiPacketParse(packet, &packetIndex);

	if (*(next + strlen(next) + 1) == '*')
	{
		tag[0] = '\0';
		return;
	}

	next = VnUartPacket_getNextData(packet, &packetIndex);

	#if defined(_MSC_VER)
		/* Disable warnings regarding using strcpy_s since this
		 * function's signature does not provide us with information
		 * about the length of 'out'. */
		#pragma warning(push)
		#pragma warning(disable:4996)
	#endif
	strcpy(tag, next);

	#if defined(_MSC_VER)
		#pragma warning(pop)
	#endif
}

void VnUartPacket_parseModelNumber(VnUartPacket *packet, char *productName)
{
	VnUartPacket_parseModelNumberRaw(packet->data, productName);
}

void VnUartPacket_parseModelNumberRaw(char *packet, char* productName)
{
	size_t packetIndex;
	char* next;

	next = VnUartPacket_startAsciiPacketParse(packet, &packetIndex);

	if (*(next + strlen(next) + 1) == '*')
	{
		productName[0] = '\0';
		return;
	}

	next = VnUartPacket_getNextData(packet, &packetIndex);

	#if defined(_MSC_VER)
		/* Disable warnings regarding using strcpy_s since this
		 * function's signature does not provide us with information
		 * about the length of 'out'. */
		#pragma warning(push)
		#pragma warning(disable:4996)
	#endif
	strcpy(productName, next);

	#if defined(_MSC_VER)
		#pragma warning(pop)
	#endif
}

void VnUartPacket_parseHardwareRevision(VnUartPacket *packet, uint32_t* revision)
{
	VnUartPacket_parseHardwareRevisionRaw(packet->data, revision);
}

void VnUartPacket_parseHardwareRevisionRaw(char *packet, uint32_t* revision)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*revision = ATOU32;
}

void VnUartPacket_parseSerialNumber(VnUartPacket *packet, uint32_t* serialNum)
{
	VnUartPacket_parseSerialNumberRaw(packet->data, serialNum);
}

void VnUartPacket_parseSerialNumberRaw(char *packet, uint32_t* serialNum)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*serialNum = ATOU32;
}

void VnUartPacket_parseFirmwareVersion(VnUartPacket *packet, char *firmwareVersion)
{
	VnUartPacket_parseFirmwareVersionRaw(packet->data, firmwareVersion);
}

void VnUartPacket_parseFirmwareVersionRaw(char *packet, char* firmwareVersion)
{
	size_t packetIndex;
	char* next;

	next = VnUartPacket_startAsciiPacketParse(packet, &packetIndex);

	if (*(next + strlen(next) + 1) == '*')
	{
		firmwareVersion[0] = '\0';
		return;
	}

	next = VnUartPacket_getNextData(packet, &packetIndex);

	#if defined(_MSC_VER)
		/* Disable warnings regarding using strcpy_s since this
		 * function's signature does not provide us with information
		 * about the length of 'out'. */
		#pragma warning(push)
		#pragma warning(disable:4996)
	#endif
	strcpy(firmwareVersion, next);

	#if defined(_MSC_VER)
		#pragma warning(pop)
	#endif
}

void VnUartPacket_parseSerialBaudRate(VnUartPacket *packet, uint32_t* baudrate)
{
	VnUartPacket_parseSerialBaudRateRaw(packet->data, baudrate);
}

void VnUartPacket_parseSerialBaudRateRaw(char *packet, uint32_t* baudrate)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*baudrate = ATOU32;
}

void VnUartPacket_parseAsyncDataOutputType(VnUartPacket *packet, uint32_t* ador)
{
	VnUartPacket_parseAsyncDataOutputTypeRaw(packet->data, ador);
}

void VnUartPacket_parseAsyncDataOutputTypeRaw(char *packet, uint32_t* ador)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*ador = ATOU32;
}

void VnUartPacket_parseAsyncDataOutputFrequency(VnUartPacket *packet, uint32_t* adof)
{
	VnUartPacket_parseAsyncDataOutputFrequencyRaw(packet->data, adof);
}

void VnUartPacket_parseAsyncDataOutputFrequencyRaw(char *packet, uint32_t* adof)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*adof = ATOU32;
}

void VnUartPacket_parseYawPitchRoll(VnUartPacket *packet, vec3f* yawPitchRoll)
{
	VnUartPacket_parseYawPitchRollRaw(packet->data, yawPitchRoll);
}

void VnUartPacket_parseYawPitchRollRaw(char *packet, vec3f* yawPitchRoll)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	yawPitchRoll->x = ATOFF; NEXTRAW
	yawPitchRoll->y = ATOFF; NEXTRAW
	yawPitchRoll->z = ATOFF;
}

void VnUartPacket_parseAttitudeQuaternion(VnUartPacket *packet, vec4f* quat)
{
	VnUartPacket_parseAttitudeQuaternionRaw(packet->data, quat);
}

void VnUartPacket_parseAttitudeQuaternionRaw(char *packet, vec4f* quat)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	quat->x = ATOFF; NEXTRAW
	quat->y = ATOFF; NEXTRAW
	quat->z = ATOFF; NEXTRAW
	quat->w = ATOFF;
}

void VnUartPacket_parseQuaternionMagneticAccelerationAndAngularRates(VnUartPacket *packet, vec4f* quat, vec3f* mag, vec3f* accel, vec3f* gyro)
{
	VnUartPacket_parseQuaternionMagneticAccelerationAndAngularRatesRaw(packet->data, quat, mag, accel, gyro);
}

void VnUartPacket_parseQuaternionMagneticAccelerationAndAngularRatesRaw(char *packet, vec4f* quat, vec3f* mag, vec3f* accel, vec3f* gyro)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	quat->x = ATOFF; NEXTRAW
	quat->y = ATOFF; NEXTRAW
	quat->z = ATOFF; NEXTRAW
	quat->w = ATOFF; NEXTRAW
	mag->x = ATOFF; NEXTRAW
	mag->y = ATOFF; NEXTRAW
	mag->z = ATOFF; NEXTRAW
	accel->x = ATOFF; NEXTRAW
	accel->y = ATOFF; NEXTRAW
	accel->z = ATOFF; NEXTRAW
	gyro->x = ATOFF; NEXTRAW
	gyro->y = ATOFF; NEXTRAW
	gyro->z = ATOFF;
}

void VnUartPacket_parseMagneticMeasurements(VnUartPacket *packet, vec3f* mag)
{
	VnUartPacket_parseMagneticMeasurementsRaw(packet->data, mag);
}

void VnUartPacket_parseMagneticMeasurementsRaw(char *packet, vec3f* mag)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	mag->x = ATOFF; NEXTRAW
	mag->y = ATOFF; NEXTRAW
	mag->z = ATOFF;
}

void VnUartPacket_parseAccelerationMeasurements(VnUartPacket *packet, vec3f* accel)
{
	VnUartPacket_parseAccelerationMeasurementsRaw(packet->data, accel);
}

void VnUartPacket_parseAccelerationMeasurementsRaw(char *packet, vec3f* accel)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	accel->x = ATOFF; NEXTRAW
	accel->y = ATOFF; NEXTRAW
	accel->z = ATOFF;
}

void VnUartPacket_parseAngularRateMeasurements(VnUartPacket *packet, vec3f* gyro)
{
	VnUartPacket_parseAngularRateMeasurementsRaw(packet->data, gyro);
}

void VnUartPacket_parseAngularRateMeasurementsRaw(char *packet, vec3f* gyro)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	gyro->x = ATOFF; NEXTRAW
	gyro->y = ATOFF; NEXTRAW
	gyro->z = ATOFF;
}

void VnUartPacket_parseMagneticAccelerationAndAngularRates(VnUartPacket *packet, vec3f* mag, vec3f* accel, vec3f* gyro)
{
	VnUartPacket_parseMagneticAccelerationAndAngularRatesRaw(packet->data, mag, accel, gyro);
}

void VnUartPacket_parseMagneticAccelerationAndAngularRatesRaw(char *packet, vec3f* mag, vec3f* accel, vec3f* gyro)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	mag->x = ATOFF; NEXTRAW
	mag->y = ATOFF; NEXTRAW
	mag->z = ATOFF; NEXTRAW
	accel->x = ATOFF; NEXTRAW
	accel->y = ATOFF; NEXTRAW
	accel->z = ATOFF; NEXTRAW
	gyro->x = ATOFF; NEXTRAW
	gyro->y = ATOFF; NEXTRAW
	gyro->z = ATOFF;
}

void VnUartPacket_parseMagneticAndGravityReferenceVectors(VnUartPacket *packet, vec3f* magRef, vec3f* accRef)
{
	VnUartPacket_parseMagneticAndGravityReferenceVectorsRaw(packet->data, magRef, accRef);
}

void VnUartPacket_parseMagneticAndGravityReferenceVectorsRaw(char *packet, vec3f* magRef, vec3f* accRef)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	magRef->x = ATOFF; NEXTRAW
	magRef->y = ATOFF; NEXTRAW
	magRef->z = ATOFF; NEXTRAW
	accRef->x = ATOFF; NEXTRAW
	accRef->y = ATOFF; NEXTRAW
	accRef->z = ATOFF;
}

void VnUartPacket_parseFilterMeasurementsVarianceParameters(VnUartPacket *packet, float* angularWalkVariance, vec3f* angularRateVariance, vec3f* magneticVariance, vec3f* accelerationVariance)
{
	VnUartPacket_parseFilterMeasurementsVarianceParametersRaw(packet->data, angularWalkVariance, angularRateVariance, magneticVariance, accelerationVariance);
}

void VnUartPacket_parseFilterMeasurementsVarianceParametersRaw(char *packet, float* angularWalkVariance, vec3f* angularRateVariance, vec3f* magneticVariance, vec3f* accelerationVariance)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*angularWalkVariance = ATOFF; NEXTRAW
	angularRateVariance->x = ATOFF; NEXTRAW
	angularRateVariance->y = ATOFF; NEXTRAW
	angularRateVariance->z = ATOFF; NEXTRAW
	magneticVariance->x = ATOFF; NEXTRAW
	magneticVariance->y = ATOFF; NEXTRAW
	magneticVariance->z = ATOFF; NEXTRAW
	accelerationVariance->x = ATOFF; NEXTRAW
	accelerationVariance->y = ATOFF; NEXTRAW
	accelerationVariance->z = ATOFF;
}

void VnUartPacket_parseMagnetometerCompensation(VnUartPacket *packet, mat3f* c, vec3f* b)
{
	VnUartPacket_parseMagnetometerCompensationRaw(packet->data, c, b);
}

void VnUartPacket_parseMagnetometerCompensationRaw(char *packet, mat3f* c, vec3f* b)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	c->e00 = ATOFF; NEXTRAW
	c->e01 = ATOFF; NEXTRAW
	c->e02 = ATOFF; NEXTRAW
	c->e10 = ATOFF; NEXTRAW
	c->e11 = ATOFF; NEXTRAW
	c->e12 = ATOFF; NEXTRAW
	c->e20 = ATOFF; NEXTRAW
	c->e21 = ATOFF; NEXTRAW
	c->e22 = ATOFF; NEXTRAW
	b->x = ATOFF; NEXTRAW
	b->y = ATOFF; NEXTRAW
	b->z = ATOFF;
}

void VnUartPacket_parseFilterActiveTuningParameters(VnUartPacket *packet, float* magneticDisturbanceGain, float* accelerationDisturbanceGain, float* magneticDisturbanceMemory, float* accelerationDisturbanceMemory)
{
	VnUartPacket_parseFilterActiveTuningParametersRaw(packet->data, magneticDisturbanceGain, accelerationDisturbanceGain, magneticDisturbanceMemory, accelerationDisturbanceMemory);
}

void VnUartPacket_parseFilterActiveTuningParametersRaw(char *packet, float* magneticDisturbanceGain, float* accelerationDisturbanceGain, float* magneticDisturbanceMemory, float* accelerationDisturbanceMemory)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*magneticDisturbanceGain = ATOFF; NEXTRAW
	*accelerationDisturbanceGain = ATOFF; NEXTRAW
	*magneticDisturbanceMemory = ATOFF; NEXTRAW
	*accelerationDisturbanceMemory = ATOFF;
}

void VnUartPacket_parseAccelerationCompensation(VnUartPacket *packet, mat3f* c, vec3f* b)
{
	VnUartPacket_parseAccelerationCompensationRaw(packet->data, c, b);
}

void VnUartPacket_parseAccelerationCompensationRaw(char *packet, mat3f* c, vec3f* b)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	c->e00 = ATOFF; NEXTRAW
	c->e01 = ATOFF; NEXTRAW
	c->e02 = ATOFF; NEXTRAW
	c->e10 = ATOFF; NEXTRAW
	c->e11 = ATOFF; NEXTRAW
	c->e12 = ATOFF; NEXTRAW
	c->e20 = ATOFF; NEXTRAW
	c->e21 = ATOFF; NEXTRAW
	c->e22 = ATOFF; NEXTRAW
	b->x = ATOFF; NEXTRAW
	b->y = ATOFF; NEXTRAW
	b->z = ATOFF;
}

void VnUartPacket_parseReferenceFrameRotation(VnUartPacket *packet, mat3f* c)
{
	VnUartPacket_parseReferenceFrameRotationRaw(packet->data, c);
}

void VnUartPacket_parseReferenceFrameRotationRaw(char *packet, mat3f* c)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	c->e00 = ATOFF; NEXTRAW
	c->e01 = ATOFF; NEXTRAW
	c->e02 = ATOFF; NEXTRAW
	c->e10 = ATOFF; NEXTRAW
	c->e11 = ATOFF; NEXTRAW
	c->e12 = ATOFF; NEXTRAW
	c->e20 = ATOFF; NEXTRAW
	c->e21 = ATOFF; NEXTRAW
	c->e22 = ATOFF;
}

void VnUartPacket_parseYawPitchRollMagneticAccelerationAndAngularRates(VnUartPacket *packet, vec3f* yawPitchRoll, vec3f* mag, vec3f* accel, vec3f* gyro)
{
	VnUartPacket_parseYawPitchRollMagneticAccelerationAndAngularRatesRaw(packet->data, yawPitchRoll, mag, accel, gyro);
}

void VnUartPacket_parseYawPitchRollMagneticAccelerationAndAngularRatesRaw(char *packet, vec3f* yawPitchRoll, vec3f* mag, vec3f* accel, vec3f* gyro)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	yawPitchRoll->x = ATOFF; NEXTRAW
	yawPitchRoll->y = ATOFF; NEXTRAW
	yawPitchRoll->z = ATOFF; NEXTRAW
	mag->x = ATOFF; NEXTRAW
	mag->y = ATOFF; NEXTRAW
	mag->z = ATOFF; NEXTRAW
	accel->x = ATOFF; NEXTRAW
	accel->y = ATOFF; NEXTRAW
	accel->z = ATOFF; NEXTRAW
	gyro->x = ATOFF; NEXTRAW
	gyro->y = ATOFF; NEXTRAW
	gyro->z = ATOFF;
}

void VnUartPacket_parseCommunicationProtocolControl(VnUartPacket *packet, uint8_t* serialCount, uint8_t* serialStatus, uint8_t* spiCount, uint8_t* spiStatus, uint8_t* serialChecksum, uint8_t* spiChecksum, uint8_t* errorMode)
{
	VnUartPacket_parseCommunicationProtocolControlRaw(packet->data, serialCount, serialStatus, spiCount, spiStatus, serialChecksum, spiChecksum, errorMode);
}

void VnUartPacket_parseCommunicationProtocolControlRaw(char *packet, uint8_t* serialCount, uint8_t* serialStatus, uint8_t* spiCount, uint8_t* spiStatus, uint8_t* serialChecksum, uint8_t* spiChecksum, uint8_t* errorMode)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*serialCount = ATOU8; NEXTRAW
	*serialStatus = ATOU8; NEXTRAW
	*spiCount = ATOU8; NEXTRAW
	*spiStatus = ATOU8; NEXTRAW
	*serialChecksum = ATOU8; NEXTRAW
	*spiChecksum = ATOU8; NEXTRAW
	*errorMode = ATOU8;
}

void VnUartPacket_parseSynchronizationControl(VnUartPacket *packet, uint8_t* syncInMode, uint8_t* syncInEdge, uint16_t* syncInSkipFactor, uint32_t* reserved1, uint8_t* syncOutMode, uint8_t* syncOutPolarity, uint16_t* syncOutSkipFactor, uint32_t* syncOutPulseWidth, uint32_t* reserved2)
{
	VnUartPacket_parseSynchronizationControlRaw(packet->data, syncInMode, syncInEdge, syncInSkipFactor, reserved1, syncOutMode, syncOutPolarity, syncOutSkipFactor, syncOutPulseWidth, reserved2);
}

void VnUartPacket_parseSynchronizationControlRaw(char *packet, uint8_t* syncInMode, uint8_t* syncInEdge, uint16_t* syncInSkipFactor, uint32_t* reserved1, uint8_t* syncOutMode, uint8_t* syncOutPolarity, uint16_t* syncOutSkipFactor, uint32_t* syncOutPulseWidth, uint32_t* reserved2)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*syncInMode = ATOU8; NEXTRAW
	*syncInEdge = ATOU8; NEXTRAW
	*syncInSkipFactor = ATOU16; NEXTRAW
	*reserved1 = ATOU32; NEXTRAW
	*syncOutMode = ATOU8; NEXTRAW
	*syncOutPolarity = ATOU8; NEXTRAW
	*syncOutSkipFactor = ATOU16; NEXTRAW
	*syncOutPulseWidth = ATOU32; NEXTRAW
	*reserved2 = ATOU32;
}

void VnUartPacket_parseSynchronizationStatus(VnUartPacket *packet, uint32_t* syncInCount, uint32_t* syncInTime, uint32_t* syncOutCount)
{
	VnUartPacket_parseSynchronizationStatusRaw(packet->data, syncInCount, syncInTime, syncOutCount);
}

void VnUartPacket_parseSynchronizationStatusRaw(char *packet, uint32_t* syncInCount, uint32_t* syncInTime, uint32_t* syncOutCount)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*syncInCount = ATOU32; NEXTRAW
	*syncInTime = ATOU32; NEXTRAW
	*syncOutCount = ATOU32;
}

void VnUartPacket_parseFilterBasicControl(VnUartPacket *packet, uint8_t* magMode, uint8_t* extMagMode, uint8_t* extAccMode, uint8_t* extGyroMode, vec3f* gyroLimit)
{
	VnUartPacket_parseFilterBasicControlRaw(packet->data, magMode, extMagMode, extAccMode, extGyroMode, gyroLimit);
}

void VnUartPacket_parseFilterBasicControlRaw(char *packet, uint8_t* magMode, uint8_t* extMagMode, uint8_t* extAccMode, uint8_t* extGyroMode, vec3f* gyroLimit)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*magMode = ATOU8; NEXTRAW
	*extMagMode = ATOU8; NEXTRAW
	*extAccMode = ATOU8; NEXTRAW
	*extGyroMode = ATOU8; NEXTRAW
	gyroLimit->x = ATOFF; NEXTRAW
	gyroLimit->y = ATOFF; NEXTRAW
	gyroLimit->z = ATOFF;
}

void VnUartPacket_parseVpeBasicControl(VnUartPacket *packet, uint8_t* enable, uint8_t* headingMode, uint8_t* filteringMode, uint8_t* tuningMode)
{
	VnUartPacket_parseVpeBasicControlRaw(packet->data, enable, headingMode, filteringMode, tuningMode);
}

void VnUartPacket_parseVpeBasicControlRaw(char *packet, uint8_t* enable, uint8_t* headingMode, uint8_t* filteringMode, uint8_t* tuningMode)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*enable = ATOU8; NEXTRAW
	*headingMode = ATOU8; NEXTRAW
	*filteringMode = ATOU8; NEXTRAW
	*tuningMode = ATOU8;
}

void VnUartPacket_parseVpeMagnetometerBasicTuning(VnUartPacket *packet, vec3f* baseTuning, vec3f* adaptiveTuning, vec3f* adaptiveFiltering)
{
	VnUartPacket_parseVpeMagnetometerBasicTuningRaw(packet->data, baseTuning, adaptiveTuning, adaptiveFiltering);
}

void VnUartPacket_parseVpeMagnetometerBasicTuningRaw(char *packet, vec3f* baseTuning, vec3f* adaptiveTuning, vec3f* adaptiveFiltering)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	baseTuning->x = ATOFF; NEXTRAW
	baseTuning->y = ATOFF; NEXTRAW
	baseTuning->z = ATOFF; NEXTRAW
	adaptiveTuning->x = ATOFF; NEXTRAW
	adaptiveTuning->y = ATOFF; NEXTRAW
	adaptiveTuning->z = ATOFF; NEXTRAW
	adaptiveFiltering->x = ATOFF; NEXTRAW
	adaptiveFiltering->y = ATOFF; NEXTRAW
	adaptiveFiltering->z = ATOFF;
}

void VnUartPacket_parseVpeMagnetometerAdvancedTuning(VnUartPacket *packet, vec3f* minFiltering, vec3f* maxFiltering, float* maxAdaptRate, float* disturbanceWindow, float* maxTuning)
{
	VnUartPacket_parseVpeMagnetometerAdvancedTuningRaw(packet->data, minFiltering, maxFiltering, maxAdaptRate, disturbanceWindow, maxTuning);
}

void VnUartPacket_parseVpeMagnetometerAdvancedTuningRaw(char *packet, vec3f* minFiltering, vec3f* maxFiltering, float* maxAdaptRate, float* disturbanceWindow, float* maxTuning)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	minFiltering->x = ATOFF; NEXTRAW
	minFiltering->y = ATOFF; NEXTRAW
	minFiltering->z = ATOFF; NEXTRAW
	maxFiltering->x = ATOFF; NEXTRAW
	maxFiltering->y = ATOFF; NEXTRAW
	maxFiltering->z = ATOFF; NEXTRAW
	*maxAdaptRate = ATOFF; NEXTRAW
	*disturbanceWindow = ATOFF; NEXTRAW
	*maxTuning = ATOFF;
}

void VnUartPacket_parseVpeAccelerometerBasicTuning(VnUartPacket *packet, vec3f* baseTuning, vec3f* adaptiveTuning, vec3f* adaptiveFiltering)
{
	VnUartPacket_parseVpeAccelerometerBasicTuningRaw(packet->data, baseTuning, adaptiveTuning, adaptiveFiltering);
}

void VnUartPacket_parseVpeAccelerometerBasicTuningRaw(char *packet, vec3f* baseTuning, vec3f* adaptiveTuning, vec3f* adaptiveFiltering)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	baseTuning->x = ATOFF; NEXTRAW
	baseTuning->y = ATOFF; NEXTRAW
	baseTuning->z = ATOFF; NEXTRAW
	adaptiveTuning->x = ATOFF; NEXTRAW
	adaptiveTuning->y = ATOFF; NEXTRAW
	adaptiveTuning->z = ATOFF; NEXTRAW
	adaptiveFiltering->x = ATOFF; NEXTRAW
	adaptiveFiltering->y = ATOFF; NEXTRAW
	adaptiveFiltering->z = ATOFF;
}

void VnUartPacket_parseVpeAccelerometerAdvancedTuning(VnUartPacket *packet, vec3f* minFiltering, vec3f* maxFiltering, float* maxAdaptRate, float* disturbanceWindow, float* maxTuning)
{
	VnUartPacket_parseVpeAccelerometerAdvancedTuningRaw(packet->data, minFiltering, maxFiltering, maxAdaptRate, disturbanceWindow, maxTuning);
}

void VnUartPacket_parseVpeAccelerometerAdvancedTuningRaw(char *packet, vec3f* minFiltering, vec3f* maxFiltering, float* maxAdaptRate, float* disturbanceWindow, float* maxTuning)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	minFiltering->x = ATOFF; NEXTRAW
	minFiltering->y = ATOFF; NEXTRAW
	minFiltering->z = ATOFF; NEXTRAW
	maxFiltering->x = ATOFF; NEXTRAW
	maxFiltering->y = ATOFF; NEXTRAW
	maxFiltering->z = ATOFF; NEXTRAW
	*maxAdaptRate = ATOFF; NEXTRAW
	*disturbanceWindow = ATOFF; NEXTRAW
	*maxTuning = ATOFF;
}

void VnUartPacket_parseVpeGryoBasicTuning(VnUartPacket *packet, vec3f* angularWalkVariance, vec3f* baseTuning, vec3f* adaptiveTuning)
{
	VnUartPacket_parseVpeGryoBasicTuningRaw(packet->data, angularWalkVariance, baseTuning, adaptiveTuning);
}

void VnUartPacket_parseVpeGryoBasicTuningRaw(char *packet, vec3f* angularWalkVariance, vec3f* baseTuning, vec3f* adaptiveTuning)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	angularWalkVariance->x = ATOFF; NEXTRAW
	angularWalkVariance->y = ATOFF; NEXTRAW
	angularWalkVariance->z = ATOFF; NEXTRAW
	baseTuning->x = ATOFF; NEXTRAW
	baseTuning->y = ATOFF; NEXTRAW
	baseTuning->z = ATOFF; NEXTRAW
	adaptiveTuning->x = ATOFF; NEXTRAW
	adaptiveTuning->y = ATOFF; NEXTRAW
	adaptiveTuning->z = ATOFF;
}

void VnUartPacket_parseFilterStartupGyroBias(VnUartPacket *packet, vec3f* bias)
{
	VnUartPacket_parseFilterStartupGyroBiasRaw(packet->data, bias);
}

void VnUartPacket_parseFilterStartupGyroBiasRaw(char *packet, vec3f* bias)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	bias->x = ATOFF; NEXTRAW
	bias->y = ATOFF; NEXTRAW
	bias->z = ATOFF;
}

void VnUartPacket_parseMagnetometerCalibrationControl(VnUartPacket *packet, uint8_t* hsiMode, uint8_t* hsiOutput, uint8_t* convergeRate)
{
	VnUartPacket_parseMagnetometerCalibrationControlRaw(packet->data, hsiMode, hsiOutput, convergeRate);
}

void VnUartPacket_parseMagnetometerCalibrationControlRaw(char *packet, uint8_t* hsiMode, uint8_t* hsiOutput, uint8_t* convergeRate)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*hsiMode = ATOU8; NEXTRAW
	*hsiOutput = ATOU8; NEXTRAW
	*convergeRate = ATOU8;
}

void VnUartPacket_parseCalculatedMagnetometerCalibration(VnUartPacket *packet, mat3f* c, vec3f* b)
{
	VnUartPacket_parseCalculatedMagnetometerCalibrationRaw(packet->data, c, b);
}

void VnUartPacket_parseCalculatedMagnetometerCalibrationRaw(char *packet, mat3f* c, vec3f* b)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	c->e00 = ATOFF; NEXTRAW
	c->e01 = ATOFF; NEXTRAW
	c->e02 = ATOFF; NEXTRAW
	c->e10 = ATOFF; NEXTRAW
	c->e11 = ATOFF; NEXTRAW
	c->e12 = ATOFF; NEXTRAW
	c->e20 = ATOFF; NEXTRAW
	c->e21 = ATOFF; NEXTRAW
	c->e22 = ATOFF; NEXTRAW
	b->x = ATOFF; NEXTRAW
	b->y = ATOFF; NEXTRAW
	b->z = ATOFF;
}

void VnUartPacket_parseIndoorHeadingModeControl(VnUartPacket *packet, float* maxRateError, uint8_t* reserved1)
{
	VnUartPacket_parseIndoorHeadingModeControlRaw(packet->data, maxRateError, reserved1);
}

void VnUartPacket_parseIndoorHeadingModeControlRaw(char *packet, float* maxRateError, uint8_t* reserved1)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*maxRateError = ATOFF; NEXTRAW
	*reserved1 = ATOU8;
}

void VnUartPacket_parseVelocityCompensationMeasurement(VnUartPacket *packet, vec3f* velocity)
{
	VnUartPacket_parseVelocityCompensationMeasurementRaw(packet->data, velocity);
}

void VnUartPacket_parseVelocityCompensationMeasurementRaw(char *packet, vec3f* velocity)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	velocity->x = ATOFF; NEXTRAW
	velocity->y = ATOFF; NEXTRAW
	velocity->z = ATOFF;
}

void VnUartPacket_parseVelocityCompensationControl(VnUartPacket *packet, uint8_t* mode, float* velocityTuning, float* rateTuning)
{
	VnUartPacket_parseVelocityCompensationControlRaw(packet->data, mode, velocityTuning, rateTuning);
}

void VnUartPacket_parseVelocityCompensationControlRaw(char *packet, uint8_t* mode, float* velocityTuning, float* rateTuning)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*mode = ATOU8; NEXTRAW
	*velocityTuning = ATOFF; NEXTRAW
	*rateTuning = ATOFF;
}

void VnUartPacket_parseVelocityCompensationStatus(VnUartPacket *packet, float* x, float* xDot, vec3f* accelOffset, vec3f* omega)
{
	VnUartPacket_parseVelocityCompensationStatusRaw(packet->data, x, xDot, accelOffset, omega);
}

void VnUartPacket_parseVelocityCompensationStatusRaw(char *packet, float* x, float* xDot, vec3f* accelOffset, vec3f* omega)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*x = ATOFF; NEXTRAW
	*xDot = ATOFF; NEXTRAW
	accelOffset->x = ATOFF; NEXTRAW
	accelOffset->y = ATOFF; NEXTRAW
	accelOffset->z = ATOFF; NEXTRAW
	omega->x = ATOFF; NEXTRAW
	omega->y = ATOFF; NEXTRAW
	omega->z = ATOFF;
}

void VnUartPacket_parseImuMeasurements(VnUartPacket *packet, vec3f* mag, vec3f* accel, vec3f* gyro, float* temp, float* pressure)
{
	VnUartPacket_parseImuMeasurementsRaw(packet->data, mag, accel, gyro, temp, pressure);
}

void VnUartPacket_parseImuMeasurementsRaw(char *packet, vec3f* mag, vec3f* accel, vec3f* gyro, float* temp, float* pressure)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	mag->x = ATOFF; NEXTRAW
	mag->y = ATOFF; NEXTRAW
	mag->z = ATOFF; NEXTRAW
	accel->x = ATOFF; NEXTRAW
	accel->y = ATOFF; NEXTRAW
	accel->z = ATOFF; NEXTRAW
	gyro->x = ATOFF; NEXTRAW
	gyro->y = ATOFF; NEXTRAW
	gyro->z = ATOFF; NEXTRAW
	*temp = ATOFF; NEXTRAW
	*pressure = ATOFF;
}

void VnUartPacket_parseGpsConfiguration(VnUartPacket *packet, uint8_t* mode, uint8_t* ppsSource, uint8_t* reserved1, uint8_t* reserved2, uint8_t* reserved3)
{
	VnUartPacket_parseGpsConfigurationRaw(packet->data, mode, ppsSource, reserved1, reserved2, reserved3);
}

void VnUartPacket_parseGpsConfigurationRaw(char *packet, uint8_t* mode, uint8_t* ppsSource, uint8_t* reserved1, uint8_t* reserved2, uint8_t* reserved3)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*mode = ATOU8; NEXTRAW
	*ppsSource = ATOU8; NEXTRAW
	*reserved1 = ATOU8; NEXTRAW
	*reserved2 = ATOU8; NEXTRAW
	*reserved3 = ATOU8;
}

void VnUartPacket_parseGpsAntennaOffset(VnUartPacket *packet, vec3f* position)
{
	VnUartPacket_parseGpsAntennaOffsetRaw(packet->data, position);
}

void VnUartPacket_parseGpsAntennaOffsetRaw(char *packet, vec3f* position)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	position->x = ATOFF; NEXTRAW
	position->y = ATOFF; NEXTRAW
	position->z = ATOFF;
}

void VnUartPacket_parseGpsSolutionLla(VnUartPacket *packet, double* time, uint16_t* week, uint8_t* gpsFix, uint8_t* numSats, vec3d* lla, vec3f* nedVel, vec3f* nedAcc, float* speedAcc, float* timeAcc)
{
	VnUartPacket_parseGpsSolutionLlaRaw(packet->data, time, week, gpsFix, numSats, lla, nedVel, nedAcc, speedAcc, timeAcc);
}

void VnUartPacket_parseGpsSolutionLlaRaw(char *packet, double* time, uint16_t* week, uint8_t* gpsFix, uint8_t* numSats, vec3d* lla, vec3f* nedVel, vec3f* nedAcc, float* speedAcc, float* timeAcc)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*time = ATOFD; NEXTRAW
	*week = ATOU16; NEXTRAW
	*gpsFix = ATOU8; NEXTRAW
	*numSats = ATOU8; NEXTRAW
	lla->x = ATOFD; NEXTRAW
	lla->y = ATOFD; NEXTRAW
	lla->z = ATOFD; NEXTRAW
	nedVel->x = ATOFF; NEXTRAW
	nedVel->y = ATOFF; NEXTRAW
	nedVel->z = ATOFF; NEXTRAW
	nedAcc->x = ATOFF; NEXTRAW
	nedAcc->y = ATOFF; NEXTRAW
	nedAcc->z = ATOFF; NEXTRAW
	*speedAcc = ATOFF; NEXTRAW
	*timeAcc = ATOFF;
}

void VnUartPacket_parseGpsSolutionEcef(VnUartPacket *packet, double* tow, uint16_t* week, uint8_t* gpsFix, uint8_t* numSats, vec3d* position, vec3f* velocity, vec3f* posAcc, float* speedAcc, float* timeAcc)
{
	VnUartPacket_parseGpsSolutionEcefRaw(packet->data, tow, week, gpsFix, numSats, position, velocity, posAcc, speedAcc, timeAcc);
}

void VnUartPacket_parseGpsSolutionEcefRaw(char *packet, double* tow, uint16_t* week, uint8_t* gpsFix, uint8_t* numSats, vec3d* position, vec3f* velocity, vec3f* posAcc, float* speedAcc, float* timeAcc)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*tow = ATOFD; NEXTRAW
	*week = ATOU16; NEXTRAW
	*gpsFix = ATOU8; NEXTRAW
	*numSats = ATOU8; NEXTRAW
	position->x = ATOFD; NEXTRAW
	position->y = ATOFD; NEXTRAW
	position->z = ATOFD; NEXTRAW
	velocity->x = ATOFF; NEXTRAW
	velocity->y = ATOFF; NEXTRAW
	velocity->z = ATOFF; NEXTRAW
	posAcc->x = ATOFF; NEXTRAW
	posAcc->y = ATOFF; NEXTRAW
	posAcc->z = ATOFF; NEXTRAW
	*speedAcc = ATOFF; NEXTRAW
	*timeAcc = ATOFF;
}

void VnUartPacket_parseInsSolutionLla(VnUartPacket *packet, double* time, uint16_t* week, uint16_t* status, vec3f* yawPitchRoll, vec3d* position, vec3f* nedVel, float* attUncertainty, float* posUncertainty, float* velUncertainty)
{
	VnUartPacket_parseInsSolutionLlaRaw(packet->data, time, week, status, yawPitchRoll, position, nedVel, attUncertainty, posUncertainty, velUncertainty);
}

void VnUartPacket_parseInsSolutionLlaRaw(char *packet, double* time, uint16_t* week, uint16_t* status, vec3f* yawPitchRoll, vec3d* position, vec3f* nedVel, float* attUncertainty, float* posUncertainty, float* velUncertainty)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*time = ATOFD; NEXTRAW
	*week = ATOU16; NEXTRAW
	*status = ATOU16X; NEXTRAW
	yawPitchRoll->x = ATOFF; NEXTRAW
	yawPitchRoll->y = ATOFF; NEXTRAW
	yawPitchRoll->z = ATOFF; NEXTRAW
	position->x = ATOFD; NEXTRAW
	position->y = ATOFD; NEXTRAW
	position->z = ATOFD; NEXTRAW
	nedVel->x = ATOFF; NEXTRAW
	nedVel->y = ATOFF; NEXTRAW
	nedVel->z = ATOFF; NEXTRAW
	*attUncertainty = ATOFF; NEXTRAW
	*posUncertainty = ATOFF; NEXTRAW
	*velUncertainty = ATOFF;
}

void VnUartPacket_parseInsSolutionEcef(VnUartPacket *packet, double* time, uint16_t* week, uint16_t* status, vec3f* yawPitchRoll, vec3d* position, vec3f* velocity, float* attUncertainty, float* posUncertainty, float* velUncertainty)
{
	VnUartPacket_parseInsSolutionEcefRaw(packet->data, time, week, status, yawPitchRoll, position, velocity, attUncertainty, posUncertainty, velUncertainty);
}

void VnUartPacket_parseInsSolutionEcefRaw(char *packet, double* time, uint16_t* week, uint16_t* status, vec3f* yawPitchRoll, vec3d* position, vec3f* velocity, float* attUncertainty, float* posUncertainty, float* velUncertainty)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*time = ATOFD; NEXTRAW
	*week = ATOU16; NEXTRAW
	*status = ATOU16X; NEXTRAW
	yawPitchRoll->x = ATOFF; NEXTRAW
	yawPitchRoll->y = ATOFF; NEXTRAW
	yawPitchRoll->z = ATOFF; NEXTRAW
	position->x = ATOFD; NEXTRAW
	position->y = ATOFD; NEXTRAW
	position->z = ATOFD; NEXTRAW
	velocity->x = ATOFF; NEXTRAW
	velocity->y = ATOFF; NEXTRAW
	velocity->z = ATOFF; NEXTRAW
	*attUncertainty = ATOFF; NEXTRAW
	*posUncertainty = ATOFF; NEXTRAW
	*velUncertainty = ATOFF;
}

void VnUartPacket_parseInsBasicConfiguration(VnUartPacket *packet, uint8_t* scenario, uint8_t* ahrsAiding, uint8_t* estBaseline, uint8_t* resv2)
{
	VnUartPacket_parseInsBasicConfigurationRaw(packet->data, scenario, ahrsAiding, estBaseline, resv2);
}

void VnUartPacket_parseInsBasicConfigurationRaw(char *packet, uint8_t* scenario, uint8_t* ahrsAiding, uint8_t* estBaseline, uint8_t* resv2)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*scenario = ATOU8; NEXTRAW
	*ahrsAiding = ATOU8; NEXTRAW
	*estBaseline = ATOU8; NEXTRAW
	*resv2 = ATOU8;
}

void VnUartPacket_parseInsAdvancedConfiguration(VnUartPacket *packet, uint8_t* useMag, uint8_t* usePres, uint8_t* posAtt, uint8_t* velAtt, uint8_t* velBias, uint8_t* useFoam, uint8_t* gpsCovType, uint8_t* velCount, float* velInit, float* moveOrigin, float* gpsTimeout, float* deltaLimitPos, float* deltaLimitVel, float* minPosUncertainty, float* minVelUncertainty)
{
	VnUartPacket_parseInsAdvancedConfigurationRaw(packet->data, useMag, usePres, posAtt, velAtt, velBias, useFoam, gpsCovType, velCount, velInit, moveOrigin, gpsTimeout, deltaLimitPos, deltaLimitVel, minPosUncertainty, minVelUncertainty);
}

void VnUartPacket_parseInsAdvancedConfigurationRaw(char *packet, uint8_t* useMag, uint8_t* usePres, uint8_t* posAtt, uint8_t* velAtt, uint8_t* velBias, uint8_t* useFoam, uint8_t* gpsCovType, uint8_t* velCount, float* velInit, float* moveOrigin, float* gpsTimeout, float* deltaLimitPos, float* deltaLimitVel, float* minPosUncertainty, float* minVelUncertainty)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*useMag = ATOU8; NEXTRAW
	*usePres = ATOU8; NEXTRAW
	*posAtt = ATOU8; NEXTRAW
	*velAtt = ATOU8; NEXTRAW
	*velBias = ATOU8; NEXTRAW
	*useFoam = ATOU8; NEXTRAW
	*gpsCovType = ATOU8; NEXTRAW
	*velCount = ATOU8; NEXTRAW
	*velInit = ATOFF; NEXTRAW
	*moveOrigin = ATOFF; NEXTRAW
	*gpsTimeout = ATOFF; NEXTRAW
	*deltaLimitPos = ATOFF; NEXTRAW
	*deltaLimitVel = ATOFF; NEXTRAW
	*minPosUncertainty = ATOFF; NEXTRAW
	*minVelUncertainty = ATOFF;
}

void VnUartPacket_parseInsStateLla(VnUartPacket *packet, vec3f* yawPitchRoll, vec3d* position, vec3f* velocity, vec3f* accel, vec3f* angularRate)
{
	VnUartPacket_parseInsStateLlaRaw(packet->data, yawPitchRoll, position, velocity, accel, angularRate);
}

void VnUartPacket_parseInsStateLlaRaw(char *packet, vec3f* yawPitchRoll, vec3d* position, vec3f* velocity, vec3f* accel, vec3f* angularRate)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	yawPitchRoll->x = ATOFF; NEXTRAW
	yawPitchRoll->y = ATOFF; NEXTRAW
	yawPitchRoll->z = ATOFF; NEXTRAW
	position->x = ATOFD; NEXTRAW
	position->y = ATOFD; NEXTRAW
	position->z = ATOFD; NEXTRAW
	velocity->x = ATOFF; NEXTRAW
	velocity->y = ATOFF; NEXTRAW
	velocity->z = ATOFF; NEXTRAW
	accel->x = ATOFF; NEXTRAW
	accel->y = ATOFF; NEXTRAW
	accel->z = ATOFF; NEXTRAW
	angularRate->x = ATOFF; NEXTRAW
	angularRate->y = ATOFF; NEXTRAW
	angularRate->z = ATOFF;
}

void VnUartPacket_parseInsStateEcef(VnUartPacket *packet, vec3f* yawPitchRoll, vec3d* position, vec3f* velocity, vec3f* accel, vec3f* angularRate)
{
	VnUartPacket_parseInsStateEcefRaw(packet->data, yawPitchRoll, position, velocity, accel, angularRate);
}

void VnUartPacket_parseInsStateEcefRaw(char *packet, vec3f* yawPitchRoll, vec3d* position, vec3f* velocity, vec3f* accel, vec3f* angularRate)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	yawPitchRoll->x = ATOFF; NEXTRAW
	yawPitchRoll->y = ATOFF; NEXTRAW
	yawPitchRoll->z = ATOFF; NEXTRAW
	position->x = ATOFD; NEXTRAW
	position->y = ATOFD; NEXTRAW
	position->z = ATOFD; NEXTRAW
	velocity->x = ATOFF; NEXTRAW
	velocity->y = ATOFF; NEXTRAW
	velocity->z = ATOFF; NEXTRAW
	accel->x = ATOFF; NEXTRAW
	accel->y = ATOFF; NEXTRAW
	accel->z = ATOFF; NEXTRAW
	angularRate->x = ATOFF; NEXTRAW
	angularRate->y = ATOFF; NEXTRAW
	angularRate->z = ATOFF;
}

void VnUartPacket_parseStartupFilterBiasEstimate(VnUartPacket *packet, vec3f* gyroBias, vec3f* accelBias, float* pressureBias)
{
	VnUartPacket_parseStartupFilterBiasEstimateRaw(packet->data, gyroBias, accelBias, pressureBias);
}

void VnUartPacket_parseStartupFilterBiasEstimateRaw(char *packet, vec3f* gyroBias, vec3f* accelBias, float* pressureBias)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	gyroBias->x = ATOFF; NEXTRAW
	gyroBias->y = ATOFF; NEXTRAW
	gyroBias->z = ATOFF; NEXTRAW
	accelBias->x = ATOFF; NEXTRAW
	accelBias->y = ATOFF; NEXTRAW
	accelBias->z = ATOFF; NEXTRAW
	*pressureBias = ATOFF;
}

void VnUartPacket_parseDeltaThetaAndDeltaVelocity(VnUartPacket *packet, float* deltaTime, vec3f* deltaTheta, vec3f* deltaVelocity)
{
	VnUartPacket_parseDeltaThetaAndDeltaVelocityRaw(packet->data, deltaTime, deltaTheta, deltaVelocity);
}

void VnUartPacket_parseDeltaThetaAndDeltaVelocityRaw(char *packet, float* deltaTime, vec3f* deltaTheta, vec3f* deltaVelocity)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*deltaTime = ATOFF; NEXTRAW
	deltaTheta->x = ATOFF; NEXTRAW
	deltaTheta->y = ATOFF; NEXTRAW
	deltaTheta->z = ATOFF; NEXTRAW
	deltaVelocity->x = ATOFF; NEXTRAW
	deltaVelocity->y = ATOFF; NEXTRAW
	deltaVelocity->z = ATOFF;
}

void VnUartPacket_parseDeltaThetaAndDeltaVelocityConfiguration(VnUartPacket *packet, uint8_t* integrationFrame, uint8_t* gyroCompensation, uint8_t* accelCompensation, uint8_t* reserved1, uint16_t* reserved2)
{
	VnUartPacket_parseDeltaThetaAndDeltaVelocityConfigurationRaw(packet->data, integrationFrame, gyroCompensation, accelCompensation, reserved1, reserved2);
}

void VnUartPacket_parseDeltaThetaAndDeltaVelocityConfigurationRaw(char *packet, uint8_t* integrationFrame, uint8_t* gyroCompensation, uint8_t* accelCompensation, uint8_t* reserved1, uint16_t* reserved2)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*integrationFrame = ATOU8; NEXTRAW
	*gyroCompensation = ATOU8; NEXTRAW
	*accelCompensation = ATOU8; NEXTRAW
	*reserved1 = ATOU8; NEXTRAW
	*reserved2 = ATOU16;
}

void VnUartPacket_parseReferenceVectorConfiguration(VnUartPacket *packet, uint8_t* useMagModel, uint8_t* useGravityModel, uint8_t* resv1, uint8_t* resv2, uint32_t* recalcThreshold, float* year, vec3d* position)
{
	VnUartPacket_parseReferenceVectorConfigurationRaw(packet->data, useMagModel, useGravityModel, resv1, resv2, recalcThreshold, year, position);
}

void VnUartPacket_parseReferenceVectorConfigurationRaw(char *packet, uint8_t* useMagModel, uint8_t* useGravityModel, uint8_t* resv1, uint8_t* resv2, uint32_t* recalcThreshold, float* year, vec3d* position)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*useMagModel = ATOU8; NEXTRAW
	*useGravityModel = ATOU8; NEXTRAW
	*resv1 = ATOU8; NEXTRAW
	*resv2 = ATOU8; NEXTRAW
	*recalcThreshold = ATOU32; NEXTRAW
	*year = ATOFF; NEXTRAW
	position->x = ATOFD; NEXTRAW
	position->y = ATOFD; NEXTRAW
	position->z = ATOFD;
}

void VnUartPacket_parseGyroCompensation(VnUartPacket *packet, mat3f* c, vec3f* b)
{
	VnUartPacket_parseGyroCompensationRaw(packet->data, c, b);
}

void VnUartPacket_parseGyroCompensationRaw(char *packet, mat3f* c, vec3f* b)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	c->e00 = ATOFF; NEXTRAW
	c->e01 = ATOFF; NEXTRAW
	c->e02 = ATOFF; NEXTRAW
	c->e10 = ATOFF; NEXTRAW
	c->e11 = ATOFF; NEXTRAW
	c->e12 = ATOFF; NEXTRAW
	c->e20 = ATOFF; NEXTRAW
	c->e21 = ATOFF; NEXTRAW
	c->e22 = ATOFF; NEXTRAW
	b->x = ATOFF; NEXTRAW
	b->y = ATOFF; NEXTRAW
	b->z = ATOFF;
}

void VnUartPacket_parseImuFilteringConfiguration(VnUartPacket *packet, uint16_t* magWindowSize, uint16_t* accelWindowSize, uint16_t* gyroWindowSize, uint16_t* tempWindowSize, uint16_t* presWindowSize, uint8_t* magFilterMode, uint8_t* accelFilterMode, uint8_t* gyroFilterMode, uint8_t* tempFilterMode, uint8_t* presFilterMode)
{
	VnUartPacket_parseImuFilteringConfigurationRaw(packet->data, magWindowSize, accelWindowSize, gyroWindowSize, tempWindowSize, presWindowSize, magFilterMode, accelFilterMode, gyroFilterMode, tempFilterMode, presFilterMode);
}

void VnUartPacket_parseImuFilteringConfigurationRaw(char *packet, uint16_t* magWindowSize, uint16_t* accelWindowSize, uint16_t* gyroWindowSize, uint16_t* tempWindowSize, uint16_t* presWindowSize, uint8_t* magFilterMode, uint8_t* accelFilterMode, uint8_t* gyroFilterMode, uint8_t* tempFilterMode, uint8_t* presFilterMode)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*magWindowSize = ATOU16; NEXTRAW
	*accelWindowSize = ATOU16; NEXTRAW
	*gyroWindowSize = ATOU16; NEXTRAW
	*tempWindowSize = ATOU16; NEXTRAW
	*presWindowSize = ATOU16; NEXTRAW
	*magFilterMode = ATOU8; NEXTRAW
	*accelFilterMode = ATOU8; NEXTRAW
	*gyroFilterMode = ATOU8; NEXTRAW
	*tempFilterMode = ATOU8; NEXTRAW
	*presFilterMode = ATOU8;
}

void VnUartPacket_parseGpsCompassBaseline(VnUartPacket *packet, vec3f* position, vec3f* uncertainty)
{
	VnUartPacket_parseGpsCompassBaselineRaw(packet->data, position, uncertainty);
}

void VnUartPacket_parseGpsCompassBaselineRaw(char *packet, vec3f* position, vec3f* uncertainty)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	position->x = ATOFF; NEXTRAW
	position->y = ATOFF; NEXTRAW
	position->z = ATOFF; NEXTRAW
	uncertainty->x = ATOFF; NEXTRAW
	uncertainty->y = ATOFF; NEXTRAW
	uncertainty->z = ATOFF;
}

void VnUartPacket_parseGpsCompassEstimatedBaseline(VnUartPacket *packet, uint8_t* estBaselineUsed, uint8_t* resv, uint16_t* numMeas, vec3f* position, vec3f* uncertainty)
{
	VnUartPacket_parseGpsCompassEstimatedBaselineRaw(packet->data, estBaselineUsed, resv, numMeas, position, uncertainty);
}

void VnUartPacket_parseGpsCompassEstimatedBaselineRaw(char *packet, uint8_t* estBaselineUsed, uint8_t* resv, uint16_t* numMeas, vec3f* position, vec3f* uncertainty)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*estBaselineUsed = ATOU8; NEXTRAW
	*resv = ATOU8; NEXTRAW
	*numMeas = ATOU16; NEXTRAW
	position->x = ATOFF; NEXTRAW
	position->y = ATOFF; NEXTRAW
	position->z = ATOFF; NEXTRAW
	uncertainty->x = ATOFF; NEXTRAW
	uncertainty->y = ATOFF; NEXTRAW
	uncertainty->z = ATOFF;
}

void VnUartPacket_parseImuRateConfiguration(VnUartPacket *packet, uint16_t* imuRate, uint16_t* navDivisor, float* filterTargetRate, float* filterMinRate)
{
	VnUartPacket_parseImuRateConfigurationRaw(packet->data, imuRate, navDivisor, filterTargetRate, filterMinRate);
}

void VnUartPacket_parseImuRateConfigurationRaw(char *packet, uint16_t* imuRate, uint16_t* navDivisor, float* filterTargetRate, float* filterMinRate)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	*imuRate = ATOU16; NEXTRAW
	*navDivisor = ATOU16; NEXTRAW
	*filterTargetRate = ATOFF; NEXTRAW
	*filterMinRate = ATOFF;
}

void VnUartPacket_parseYawPitchRollTrueBodyAccelerationAndAngularRates(VnUartPacket *packet, vec3f* yawPitchRoll, vec3f* bodyAccel, vec3f* gyro)
{
	VnUartPacket_parseYawPitchRollTrueBodyAccelerationAndAngularRatesRaw(packet->data, yawPitchRoll, bodyAccel, gyro);
}

void VnUartPacket_parseYawPitchRollTrueBodyAccelerationAndAngularRatesRaw(char *packet, vec3f* yawPitchRoll, vec3f* bodyAccel, vec3f* gyro)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	yawPitchRoll->x = ATOFF; NEXTRAW
	yawPitchRoll->y = ATOFF; NEXTRAW
	yawPitchRoll->z = ATOFF; NEXTRAW
	bodyAccel->x = ATOFF; NEXTRAW
	bodyAccel->y = ATOFF; NEXTRAW
	bodyAccel->z = ATOFF; NEXTRAW
	gyro->x = ATOFF; NEXTRAW
	gyro->y = ATOFF; NEXTRAW
	gyro->z = ATOFF;
}

void VnUartPacket_parseYawPitchRollTrueInertialAccelerationAndAngularRates(VnUartPacket *packet, vec3f* yawPitchRoll, vec3f* inertialAccel, vec3f* gyro)
{
	VnUartPacket_parseYawPitchRollTrueInertialAccelerationAndAngularRatesRaw(packet->data, yawPitchRoll, inertialAccel, gyro);
}

void VnUartPacket_parseYawPitchRollTrueInertialAccelerationAndAngularRatesRaw(char *packet, vec3f* yawPitchRoll, vec3f* inertialAccel, vec3f* gyro)
{
	size_t packetIndex;
	char *result = VnUartPacket_startAsciiResponsePacketParse(packet, &packetIndex);

	yawPitchRoll->x = ATOFF; NEXTRAW
	yawPitchRoll->y = ATOFF; NEXTRAW
	yawPitchRoll->z = ATOFF; NEXTRAW
	inertialAccel->x = ATOFF; NEXTRAW
	inertialAccel->y = ATOFF; NEXTRAW
	inertialAccel->z = ATOFF; NEXTRAW
	gyro->x = ATOFF; NEXTRAW
	gyro->y = ATOFF; NEXTRAW
	gyro->z = ATOFF;
}

void strFromVnAsciiAsync(char *out, VnAsciiAsync val)
{
	#if defined(_MSC_VER)
		/* Disable warnings regarding using strcpy_s since this
		 * function's signature does not provide us with information
		 * about the length of 'out'. */
		#pragma warning(push)
		#pragma warning(disable:4996)
	#endif

	switch (val)
	{
		case VNOFF:
			strcpy(out, "Off");
			break;
		case VNYPR:
			strcpy(out, "Yaw, Pitch, Roll");
			break;
		case VNQTN:
			strcpy(out, "Quaterion");
			break;
		#ifdef EXTRA
		case VNQTM:
			strcpy(out, "Quaternion and Magnetic");
			break;
		case VNQTA:
			strcpy(out, "Quaternion and Acceleration");
			break;
		case VNQTR:
			strcpy(out, "Quaternion and Angular Rates");
			break;
		case VNQMA:
			strcpy(out, "Quaternion, Magnetic and Acceleration");
			break;
		case VNQAR:
			strcpy(out, "Quaternion, Acceleration and Angular Rates");
			break;
		#endif
		case VNQMR:
			strcpy(out, "Quaternion, Magnetic, Acceleration and Angular Rates");
			break;
		#ifdef EXTRA
		case VNDCM:
			strcpy(out, "Directional Cosine Orientation Matrix");
			break;
		#endif
		case VNMAG:
			strcpy(out, "Magnetic Measurements");
			break;
		case VNACC:
			strcpy(out, "Acceleration Measurements");
			break;
		case VNGYR:
			strcpy(out, "Angular Rate Measurements");
			break;
		case VNMAR:
			strcpy(out, "Magnetic, Acceleration, and Angular Rate Measurements");
			break;
		case VNYMR:
			strcpy(out, "Yaw, Pitch, Roll, Magnetic, Acceleration, and Angular Rate Measurements");
			break;
		#ifdef EXTRA
		case VNYCM:
			strcpy(out, "Yaw, Pitch, Roll, and Calibrated Measurements");
			break;
		#endif
		case VNYBA:
			strcpy(out, "Yaw, Pitch, Roll, Body True Acceleration");
			break;
		case VNYIA:
			strcpy(out, "Yaw, Pitch, Roll, Inertial True Acceleration");
			break;
		#ifdef EXTRA
		case VNICM:
			strcpy(out, "Yaw, Pitch, Roll, Inertial Magnetic/Acceleration, and Angular Rate Measurements");
			break;
		#endif
		case VNIMU:
			strcpy(out, "Calibrated Inertial Measurements");
			break;
		case VNGPS:
			strcpy(out, "GPS LLA");
			break;
		case VNGPE:
			strcpy(out, "GPS ECEF");
			break;
		case VNINS:
			strcpy(out, "INS LLA solution");
			break;
		case VNINE:
			strcpy(out, "INS ECEF solution");
			break;
		case VNISL:
			strcpy(out, "INS LLA 2 solution");
			break;
		case VNISE:
			strcpy(out, "INS ECEF 2 solution");
			break;
		case VNDTV:
			strcpy(out, "Delta Theta and Delta Velocity");
			break;
		#ifdef EXTRA
		case VNRAW:
			strcpy(out, "Raw Voltage Measurements");
			break;
		case VNCMV:
			strcpy(out, "Calibrated Measurements");
			break;
		case VNSTV:
			strcpy(out, "Kalman Filter State Vector");
			break;
		case VNCOV:
			strcpy(out, "Kalman Filter Covariance Matrix Diagonal");
			break;
		#endif
		default:
			strcpy(out, "Unknown");
			break;
	}

	#if defined(_MSC_VER)
		#pragma warning(pop)
	#endif
}
