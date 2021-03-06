﻿// SysmonDataRecv.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>


//  SetPrivilege enables/disables process token privilege.
BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	LUID luid;
	BOOL bRet = FALSE;

	if (LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
	{
		TOKEN_PRIVILEGES tp;

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = (bEnablePrivilege) ? SE_PRIVILEGE_ENABLED : 0;

		//  Enable the privilege or disable all privileges.
		if (AdjustTokenPrivileges(hToken, FALSE, &tp, NULL, (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
		{
			//  Check to see if you have proper access.
			//  You may get "ERROR_NOT_ALL_ASSIGNED".
			bRet = (GetLastError() == ERROR_SUCCESS);
		}
	}
	return bRet;
}


int main() {

	HANDLE hDevice = NULL, result;
	DWORD BytesReturned;
	OVERLAPPED overlapped;
	int OutBuffer;
	unsigned char *p;
	HANDLE hToken;
	HANDLE hProcess;
	void* lpOutBuffer = NULL;


	// Get handle of current process
	hProcess = GetCurrentProcess();

	// Set SeDebug Privilege
	if (OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		SetPrivilege(hToken, SE_DEBUG_NAME, TRUE);
		CloseHandle(hToken);
	}

	// Open Handle to SysmonDrv devide 
	result = CreateFileW(L"\\\\.\\SysmonDrv", 0xC0000000, 0, 0, 3, 0x40000080, 0);

	if (result == (HANDLE)-1)
	{
		printf("Failed to get handle from SysmonDrv device, Make sure you run the process as administrator and SysmonDrv is running.");
		goto ret;
	}
	else
	{
		printf("Handle from SysmonDrv : 0x%x\n", result);
	}


	InBuffer = 800;

	// Sending ready packet to notify driver about getting handle

	if (DeviceIoControl(result, 0x83400000, 0, 0, &OutBuffer, 4, &BytesReturned, 0))
	{
		hDevice = result;
	}



	// Creating Event
	overlapped.hEvent = CreateEventW(0, 0, 0, 0);
	if (!overlapped.hEvent)
	{
		printf("Failed to initialize event for dispatch.");
		goto ret;
	}

	// Receiving buffer
	if (DeviceIoControl(hDevice, 0x83400008, 0, 0, 0, 0, &BytesReturned, 0)) {
		printf("Sysmon rules updated successfully !\n");
	}
	else
	{
		printf("Sysmon rules updated was noot successfull !\n");

	}

ret:
	return 0;
}
