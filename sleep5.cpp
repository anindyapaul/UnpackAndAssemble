//////////////////////////////////////////////////////////////////////////////
//
//  Detours Test Program (sleep5.cpp of sleep5.exe)
//
//  Microsoft Research Detours Package, Version 3.0.
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

int __cdecl main(int argc, char ** argv)
{
    if (argc == 2) {
        Sleep(atoi(argv[1]) * 1000);
    }
    else {
        printf("sleep5.exe: Starting. Will sleep for 2 seconds.\n");

        Sleep(2000);

		printf("sleep5.exe: Done sleeping.\n");
		printf("sleep5.exe: Start writing.\n");
		FILE* pFile;
		fopen_s(&pFile, "C:\\Test.txt", "a+");
		fprintf(pFile,"Wrote From sleep5.cpp \n");
		fclose(pFile);

        printf("sleep5.exe: Done writing.\n");
    }
    return 0;
}
//
///////////////////////////////////////////////////////////////// End of File.
