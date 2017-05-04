//////////////////////////////////////////////////////////////////////////////
//
//  Detours Test Program (simple.cpp of simple.dll)
//
//  Microsoft Research Detours Package, Version 3.0.
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  This DLL will detour the Windows SleepEx API so that TimedSleep function
//  gets called instead.  TimedSleepEx records the before and after times, and
//  calls the real SleepEx API through the TrueSleepEx function pointer.
//
#include <stdio.h>
#include <windows.h>
#include "detours.h"

static LONG dwSlept = 0;
static DWORD (WINAPI * TrueSleepEx)(DWORD dwMilliseconds, BOOL bAlertable) = SleepEx;
static BOOL (WINAPI * TrueWriteFileEx)(HANDLE hFile, LPCVOID lpBuffer, 
	DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, 
	LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) = WriteFileEx;
static BOOL (WINAPI * TrueWriteFile)(HANDLE hFile, LPCVOID lpBuffer, 
	DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) = WriteFile;

DWORD WINAPI TimedSleepEx(DWORD dwMilliseconds, BOOL bAlertable)
{
    DWORD dwBeg = GetTickCount();
    DWORD ret = TrueSleepEx(dwMilliseconds, bAlertable);
    DWORD dwEnd = GetTickCount();

    InterlockedExchangeAdd(&dwSlept, dwEnd - dwBeg);

    return ret;
}

BOOL WINAPI MyWriteFileEx(HANDLE hFile, LPCVOID lpBuffer, 
	DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, 
	LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	printf("Intercepted WriteFileEx() system call\n");

	FILE* pFile;
	fopen_s(&pFile, "C:\\WriteLog.txt", "a+");
    fprintf(pFile,"Intercepted WriteFileEx() \n");
    fclose(pFile);
	BOOL ret = TrueWriteFileEx(hFile, lpBuffer, nNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine);

    return ret;
}

BOOL WINAPI MyWriteFile(HANDLE hFile, LPCVOID lpBuffer, 
	DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	printf("Intercepted WriteFile() system call\n");
	BOOL ret = TrueWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);

    return ret;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    LONG error;
    (void)hinst;
    (void)reserved;

    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_ATTACH) {
        DetourRestoreAfterWith();

		printf("Testing with DLL Process Attach.\n");
		printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
               " Starting.\n");
        fflush(stdout);

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueSleepEx, TimedSleepEx);
        error = DetourTransactionCommit();

        if (error == NO_ERROR) {
			printf("No error with DLL Process Attach.\n");
            printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
                   " Detoured SleepEx().\n");
        }
        else {
			printf("Error with DLL Process Attach.\n");
            printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
                   " Error detouring SleepEx(): %d\n", error);
        }
		fflush(stdout);

		DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueWriteFileEx, MyWriteFileEx);
        error = DetourTransactionCommit();

        if (error == NO_ERROR) {
			printf("No error with DLL Process Attach.\n");
            printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
                   " Detoured WriteFileEx().\n");
        }
        else {
			printf("Error with DLL Process Attach.\n");
            printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
                   " Error detouring WriteFileEx(): %d\n", error);
        }

		DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueWriteFile, MyWriteFile);
        error = DetourTransactionCommit();

        if (error == NO_ERROR) {
			printf("No error with DLL Process Attach.\n");
            printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
                   " Detoured WriteFile().\n");
        }
        else {
			printf("Error with DLL Process Attach.\n");
            printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
                   " Error detouring WriteFile(): %d\n", error);
        }
		fflush(stdout);
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueSleepEx, TimedSleepEx);
        error = DetourTransactionCommit();

		printf("Testing with DLL Process Detach.\n");
        printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
               " Removed SleepEx() (result=%d), slept %d ticks.\n", error, dwSlept);
        fflush(stdout);

		DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueWriteFileEx, MyWriteFileEx);
        error = DetourTransactionCommit();

		printf("Testing with DLL Process Detach.\n");
        printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
               " Removed WriteFileEx() (result=%d).\n", error);
        fflush(stdout);

		DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueWriteFile, MyWriteFile);
        error = DetourTransactionCommit();

		printf("Testing with DLL Process Detach.\n");
        printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
               " Removed WriteFile() (result=%d).\n", error);
        fflush(stdout);
    }
    return TRUE;
}

//
///////////////////////////////////////////////////////////////// End of File.
