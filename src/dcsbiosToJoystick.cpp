// vJoyClient.cpp : Simple feeder application with a FFB demo
//


// Monitor Force Feedback (FFB) vJoy device
#include "stdafx.h"
//#include "Devioctl.h"
#include "public.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "vjoyinterface.h"
#include "Math.h"
#include "DcsBiosReader.hpp"

// Default device ID (Used when ID not specified)
#define DEV_ID		1

int serial_result = 0;


JOYSTICK_POSITION_V2 iReport; // The structure that holds the full position data

int
__cdecl
_tmain(int argc, _TCHAR* argv[])
{
	int stat = 0;
	UINT DevID = DEV_ID;
	USHORT X = 0;
	USHORT Y = 0;
	USHORT Z = 0;
	LONG   Btns = 0;


	PVOID pPositionMessage;
	UINT	IoCode = LOAD_POSITIONS;
	UINT	IoSize = sizeof(JOYSTICK_POSITION);
	// HID_DEVICE_ATTRIBUTES attrib;
	BYTE id = 1;
	UINT iInterface = 1;

	// Set the target Joystick - get it from the command-line 
	if (argc > 1)
		DevID = _tstoi(argv[1]);

	// Get the driver attributes (Vendor ID, Product ID, Version Number)
	if (!vJoyEnabled())
	{
		_tprintf("Function vJoyEnabled Failed - make sure that vJoy is installed and enabled\n");
		int dummy = getchar();
		stat = -2;
		RelinquishVJD(DevID);
		return stat;
	}
	else
	{
		wprintf(L"Vendor: %hs\nProduct :%hs\nVersion Number:%hs\n", static_cast<TCHAR*> (GetvJoyManufacturerString()), static_cast<TCHAR*>(GetvJoyProductString()), static_cast<TCHAR*>(GetvJoySerialNumberString()));
	};

	// Get the status of the vJoy device before trying to acquire it
	VjdStat status = GetVJDStatus(DevID);

	switch (status)
	{
	case VJD_STAT_OWN:
		_tprintf("vJoy device %d is already owned by this feeder\n", DevID);
		break;
	case VJD_STAT_FREE:
		_tprintf("vJoy device %d is free\n", DevID);
		break;
	case VJD_STAT_BUSY:
		_tprintf("vJoy device %d is already owned by another feeder\nCannot continue\n", DevID);
		return -3;
	case VJD_STAT_MISS:
		_tprintf("vJoy device %d is not installed or disabled\nCannot continue\n", DevID);
		return -4;
	default:
		_tprintf("vJoy device %d general error\nCannot continue\n", DevID);
		return -1;
	};

	// Acquire the vJoy device
	if (!AcquireVJD(DevID))
	{
		_tprintf("Failed to acquire vJoy device number %d.\n", DevID);
		int dummy = getchar();
		stat = -1;
		RelinquishVJD(DevID);
		return stat;
	}
	else
		_tprintf("Acquired device number %d - OK\n", DevID);


	// Start endless loop
	// The loop injects position data to the vJoy device
	// If it fails it let's the user try again
	DcsBiosReader reader("COM3"); // change to your COM port
	if (!reader.openPort())
	{
		printf("Error opening dcsBios serial port");
		return 1;
	}
		

	while (1)
	{
		//Check dcsbios
		reader.readLoop();


		// Set destenition vJoy device
		id = (BYTE)DevID;
		iReport.bDevice = id;

		// Set position data of 3 first axes
		if (Z>35000) Z=0;
		Z += 200;
		iReport.wAxisZ = Z;
		iReport.wAxisX = 32000-Z;
		iReport.wAxisY = Z/2+7000;

		// Set position data of first 8 buttons
		Btns = 1<<(Z/4000);
		iReport.lButtons = Btns;

		// Send position data to vJoy device
		pPositionMessage = (PVOID)(&iReport);
		if (!UpdateVJD(DevID, pPositionMessage))
		{
			printf("Feeding vJoy device number %d failed - try to enable device then press enter\n", DevID);
			getchar();
			AcquireVJD(DevID);
		}
		Sleep(2);
	}

	RelinquishVJD(DevID);
	return 0;
}

