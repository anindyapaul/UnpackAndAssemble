#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <psapi.h>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <Dbghelp.h>

#pragma comment( lib, "psapi.lib" )
#pragma comment( lib, "Dbghelp.lib" )

void WriteDump(HANDLE hProcess, DWORD dwProcessId)
{
    HANDLE hFile = CreateFile( "C:\\Dump1.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

  if( hFile != INVALID_HANDLE_VALUE ) 
  {
    BOOL  fSuccess;

    printf( "hProcess   : %d (0x%x)\n", hProcess, hProcess );
    printf( "dwProcessId: %u (0x%lx)\n", dwProcessId, dwProcessId );
	
    SetLastError( 0L );

    fSuccess = MiniDumpWriteDump( hProcess, 
                                  dwProcessId, 
                                  hFile, 
								  MiniDumpWithFullMemory, 
								  NULL,
                                  NULL, 
                                  NULL );

    DWORD dwErr = GetLastError();

    if( ! fSuccess )
    {
      printf( "MiniDumpWriteDump -FAILED (LastError:%u)\n", dwErr );

      LPVOID lpMsgBuf;

      FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL,
                     dwErr,
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                     (LPTSTR) &lpMsgBuf,
                     0,
                     NULL );

      // Display the string.
      printf( "%s\n", (LPCTSTR)lpMsgBuf );

      // Free the buffer.
      LocalFree( lpMsgBuf );
    }
	
  } 
}

void find_locs(HANDLE process) {

    unsigned char *p = NULL;
    MEMORY_BASIC_INFORMATION info;

    for ( p = NULL;
        VirtualQueryEx(process, p, &info, sizeof(info)) == sizeof(info);
        p += info.RegionSize ) 
    {
        std::vector<char> buffer;
        std::vector<char>::iterator pos;

        if (info.State == MEM_COMMIT && 
            (info.Type == MEM_MAPPED || info.Type == MEM_PRIVATE)) 
        {
            SIZE_T bytes_read;
            buffer.resize(info.RegionSize);
            ReadProcessMemory(process, p, &buffer[0], info.RegionSize, &bytes_read);
            buffer.resize(bytes_read);
            
			FILE* pFile;
			fopen_s(&pFile, "C:\\WriteMemory.txt", "a+");
			for(pos=buffer.begin() ; pos < buffer.end(); pos++) {
				fprintf(pFile, "%s", *pos);
			}
			fclose(pFile);
        }
    }
}

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1

void PrintMemoryInfo( DWORD processID, unsigned int i )
{
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    // Print the process identifier.

    printf( "\nProcess ID: %u\n", processID );

    // Print information about the memory usage of the process.

    hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );
    if (NULL == hProcess)
        return;

	if(((int)processID) == 3124) {
		find_locs(hProcess);
		WriteDump(hProcess, processID);
	}

    CloseHandle( hProcess );
}


int main( void )
{
    // Get the list of process identifiers.
	
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
    {
        return 1;
    }

    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the memory usage for each process

    for ( i = 0; i < cProcesses; i++ )
    {
        PrintMemoryInfo( aProcesses[i], i );
    }

    return 0;
}
