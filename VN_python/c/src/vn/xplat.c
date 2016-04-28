/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
#include "vn/xplat.h"

#if __linux__
	#include <unistd.h>
#endif

#include "vn/int.h"

#define MAX_KEY_LENGTH			255
#define MAX_PORT_NAME_LENGTH	30

VnError xplat_to_vnerror(
	int32_t nativeErrorCode);

bool vnxp_isOsWinXp();

#if BROKEN
VnError xplat_getComPortRegistryKey(
	char const* portName,
	const wchar_t* controlSetName,
	PHKEY key);
#endif

/* Private variables. */
double _pcFreq = 0.0;
uint64_t _counterStart = -1;

bool vnxp_isOsWinXp()
{
	return false;

	#if BROKEN
	
	DWORD dwVersion = 0;
	DWORD dwMajorVersion = 0;
	DWORD dwMinorVersion = 0;

	dwVersion = GetVersion();

	dwMajorVersion = (DWORD) (LOBYTE(LOWORD(dwVersion)));
	dwMinorVersion = (DWORD) (HIBYTE(LOWORD(dwVersion)));

	return dwMajorVersion == 5 && dwMinorVersion >= 1;
	
	#endif
}


#if BROKEN

VnError vnxp_getComPortRegistryKey(
	char const* portName,
	const wchar_t* controlSetName,
	PHKEY key)
{
	HKEY ftdiBusKey;
	long error;
	TCHAR systemClassName[MAX_PATH] = TEXT("");
	DWORD systemClassNameSize = MAX_PATH;
	TCHAR className[MAX_PATH] = TEXT("");
	DWORD classNameSize = MAX_PATH;
	DWORD numOfSubKeys = 0;
	DWORD maxSubKeyLength;
	DWORD maxClassNameSize;
	DWORD numOfKeyValues;
	DWORD maxValueNameSize;
	DWORD maxValueDataSize;
	DWORD securityDescriptor;
	FILETIME lastWriteTime;
	TCHAR ftdiBusKeyPath[MAX_PATH] = TEXT("SYSTEM\\");

	wcscat(ftdiBusKeyPath, controlSetName);
	wcscat(ftdiBusKeyPath, L"\\Enum\\FTDIBUS");

	/* Open the FTDIBUS on CurrentControlSet. */
	error = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		ftdiBusKeyPath,
		0,
		KEY_READ,
		&ftdiBusKey);

	if (error != ERROR_SUCCESS)
		return xplat_to_vnerror(error);

	/* Get all of the keys of the FTDIBUS key. */
	error = RegQueryInfoKey(
		ftdiBusKey,
		className,
		&classNameSize,
		NULL,
		&numOfSubKeys,
		&maxSubKeyLength,
		&maxClassNameSize,
		&numOfKeyValues,
		&maxValueNameSize,
		&maxValueDataSize,
		&securityDescriptor,
		&lastWriteTime);

	if (error != ERROR_SUCCESS)
		return xplat_to_vnerror(error);

	if (numOfSubKeys > 0)
	{
		int i;

		for (i = 0; i < numOfSubKeys; i++)
		{
			TCHAR subKeyNameToCheck[MAX_PATH] = TEXT("");
			TCHAR keyPortName[MAX_PATH] = TEXT("");
			TCHAR subKeyName[MAX_KEY_LENGTH] = TEXT("");
			DWORD subKeyNameSize = MAX_KEY_LENGTH;
			HKEY subKey;
			DWORD keyPortNameSize = MAX_PATH;
			char retrievedPortName[MAX_PORT_NAME_LENGTH];
			DWORD latencyTimerValue;
			DWORD latencyTimerValueSize = sizeof(DWORD);

			error = RegEnumKeyEx(
				ftdiBusKey,
				i,
				subKeyName,
				&subKeyNameSize,
				NULL,
				NULL,
				NULL,
				&lastWriteTime);

			if (error != ERROR_SUCCESS)
				return xplat_to_vnerror(error);

			wcscpy(subKeyNameToCheck, ftdiBusKeyPath);
			wcscat(subKeyNameToCheck, L"\\");
			wcscat(subKeyNameToCheck, subKeyName);
			wcscat(subKeyNameToCheck, L"\\");
			wcscat(subKeyNameToCheck, L"\\0000\\Device Parameters");

			error = RegOpenKeyEx(
				HKEY_LOCAL_MACHINE,
				subKeyNameToCheck,
				0,
				KEY_QUERY_VALUE,
				&subKey);

			if (error != ERROR_SUCCESS)
				return xplat_to_vnerror(error);

			error = RegQueryValueEx(
				subKey,
				L"PortName",
				NULL,
				NULL,
				(LPBYTE) keyPortName,
				&keyPortNameSize);

			if (error != ERROR_SUCCESS)
				return xplat_to_vnerror(error);

			/* Let's see if this is the port we are looking for. */
			wcstombs(retrievedPortName, keyPortName, MAX_PORT_NAME_LENGTH);
			if (strcmp(retrievedPortName, portName) != 0)
				/* Not the port we are looking for. */
				continue;

			/* We found the port we are looking for! */
			error = RegOpenKeyEx(
				HKEY_LOCAL_MACHINE,
				subKeyNameToCheck,
				0,
				KEY_READ,
				key);

			if (error != ERROR_SUCCESS)
				return xplat_to_vnerror(error);

			return E_NONE;
		}
	}

	/* We must not have been able to find the COM port settings. */
	return E_NOT_FOUND;
}

VnError xplat_comport_isOptimized(
	char const* portName,
	bool* isOptimized)
{
	#if _WIN32
	
	HKEY systemKey;
	LSTATUS error;
	DWORD systemNumOfSubKeys = 0;
	int i = 0;
	
	*isOptimized = false;

	/* Get the list of ControlSets. */
	error = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		L"SYSTEM",
		0,
		KEY_READ,
		&systemKey);

	if (error != ERROR_SUCCESS)
		return xplat_to_vnerror(error);

	error = RegQueryInfoKey(
		systemKey,
		NULL,
		NULL,
		NULL,
		&systemNumOfSubKeys,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	if (error != ERROR_SUCCESS)
		return xplat_to_vnerror(error);

	/* Go through each ControlSet00X. */
	for (i = 0; i < systemNumOfSubKeys; i++)
	{
		TCHAR controlSetName[MAX_KEY_LENGTH] = TEXT("");
		DWORD controlSetNameSize = MAX_KEY_LENGTH;
		HKEY comPortKey;
		DWORD latencyTimerValue;
		DWORD latencyTimerValueSize = sizeof(DWORD);

		error = RegEnumKeyEx(
			systemKey,
			i,
			controlSetName,
			&controlSetNameSize,
			NULL,
			NULL,
			NULL,
			NULL);

		if (error != ERROR_SUCCESS)
			return xplat_to_vnerror(error);

		/* See if this matches our ControlSet00X pattern. */
		if (wcsncmp(L"ControlSet", controlSetName, 10) != 0)
			/* Not what we are looking for. */
			continue;

		error = vnxp_getComPortRegistryKey(
			portName,
			controlSetName,
			&comPortKey);

		if (error == E_NOT_FOUND)
			/* No registry entry. */
			continue;

		if (error != E_NONE && error != E_NOT_FOUND)
			return xplat_to_vnerror(error);

		/* Check the value of the LatencyTimer field. */
		error = RegQueryValueEx(
			comPortKey,
			L"LatencyTimer",
			NULL,
			NULL,
			(LPBYTE) &latencyTimerValue,
			&latencyTimerValueSize);

		if (error != ERROR_SUCCESS)
			return xplat_to_vnerror(error);

		if (latencyTimerValue != 1)
			/* We have already inialized isOptimized to false. */
			return E_NONE;
	}

	/* If we got here, either we did not find any registry entries for the COM
	   port (possibly indicating this is not an FTDI USB Virtual COM port) or
	   the registry entries we found were already optimized. */
	
	*isOptimized = true;

	#elif __linux__

	/* Currently Linux USB COM ports don't need any optimization. */
	*isOptimized = true;

	#endif

	return E_NONE;
}

VnError xplat_comport_optimize(
	char const* portName)
{
	#if _WIN32
	
	HKEY systemKey;
	long error;
	DWORD systemNumOfSubKeys = 0;
	int i = 0;
	DWORD optimizedLatencyTimerValue = 1;
	bool haveFoundAtLeastOneRegistryEntryForComPort = false;

	/* Get the list of ControlSets. */
	error = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		L"SYSTEM",
		0,
		KEY_READ,
		&systemKey);

	if (error != ERROR_SUCCESS)
		return xplat_to_vnerror(error);

	error = RegQueryInfoKey(
		systemKey,
		NULL,
		NULL,
		NULL,
		&systemNumOfSubKeys,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	if (error != ERROR_SUCCESS)
		return xplat_to_vnerror(error);

	/* Go through each ControlSet00X. */
	for (i = 0; i < systemNumOfSubKeys; i++)
	{
		TCHAR controlSetName[MAX_KEY_LENGTH] = TEXT("");
		DWORD controlSetNameSize = MAX_KEY_LENGTH;
		HKEY comPortKey, comPortKeyWriteAccess;
		DWORD latencyTimerValue;
		DWORD latencyTimerValueSize = sizeof(DWORD);

		error = RegEnumKeyEx(
			systemKey,
			i,
			controlSetName,
			&controlSetNameSize,
			NULL,
			NULL,
			NULL,
			NULL);

		if (error != ERROR_SUCCESS)
			return xplat_to_vnerror(error);

		/* See if this matches our ControlSet00X pattern. */
		if (wcsncmp(L"ControlSet", controlSetName, 10) != 0)
			/* Not what we are looking for. */
			continue;

		error = vnxp_getComPortRegistryKey(
			portName,
			controlSetName,
			&comPortKey);

		if (error != E_NONE && error != E_NOT_FOUND)
			return xplat_to_vnerror(error);

		haveFoundAtLeastOneRegistryEntryForComPort = true;

		error = RegOpenKeyEx(
			comPortKey,
			NULL,
			0,
			KEY_SET_VALUE,
			&comPortKeyWriteAccess);

		if (error != ERROR_SUCCESS)
			return xplat_to_vnerror(error);

		error = RegSetValueEx(
			comPortKeyWriteAccess,
			L"LatencyTimer",
			0,
			REG_DWORD,
			(uint8_t*) &optimizedLatencyTimerValue,
			sizeof(DWORD));

		if (error != ERROR_SUCCESS)
			return xplat_to_vnerror(error);
	}

	if (haveFoundAtLeastOneRegistryEntryForComPort)
		return E_NONE;
	else
		/* Did not find any registry entries for the COM port. */
		return E_NOT_FOUND;

	#elif __linux__

	/* Nothing necessary to do on Linux machines. */
	return E_NONE;

	#endif
}

#endif

VnError xplat_sleepMs(uint32_t numOfMillisecondsToSleep)
{
	#if _WIN32
	Sleep(numOfMillisecondsToSleep);
	#elif __linux__
	usleep(numOfMillisecondsToSleep * 1000);
	#endif

	return E_NONE;
}

