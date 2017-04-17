#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <tchar.h>
#include <psapi.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "psapi.lib")
using namespace std;

void PrintMemoryInfo( DWORD processID )
{
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    // Print the process identifier.

    printf( "\nProcess ID: %u\n", processID );

    // Print information about the memory usage of the process.

    hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
                                    PROCESS_VM_READ,
                                    FALSE, processID );
    if (NULL == hProcess)
        return;

    if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
    {
        printf( "\tPageFaultCount: 0x%08X\n", pmc.PageFaultCount );
        printf( "\tPeakWorkingSetSize: 0x%08X\n", 
                  pmc.PeakWorkingSetSize );
        printf( "\tWorkingSetSize: 0x%08X\n", pmc.WorkingSetSize );
        printf( "\tQuotaPeakPagedPoolUsage: 0x%08X\n", 
                  pmc.QuotaPeakPagedPoolUsage );
        printf( "\tQuotaPagedPoolUsage: 0x%08X\n", 
                  pmc.QuotaPagedPoolUsage );
        printf( "\tQuotaPeakNonPagedPoolUsage: 0x%08X\n", 
                  pmc.QuotaPeakNonPagedPoolUsage );
        printf( "\tQuotaNonPagedPoolUsage: 0x%08X\n", 
                  pmc.QuotaNonPagedPoolUsage );
        printf( "\tPagefileUsage: 0x%08X\n", pmc.PagefileUsage ); 
        printf( "\tPeakPagefileUsage: 0x%08X\n", 
                  pmc.PeakPagefileUsage );
    }

    CloseHandle( hProcess );
}

void GetListofProcessIdentifiers( )
{

    DWORD aProcesses[1024]; 
    DWORD cbNeeded; 
    DWORD cProcesses;
    unsigned int i;
	
    // Get the list of process identifiers.

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return ;

    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the names of the modules for each process.

    for ( i = 0; i < cProcesses; i++ )
    {
        PrintMemoryInfo( aProcesses[i] );
    }

    return;
}


void getSystemInfoExample()
{
   SYSTEM_INFO siSysInfo;
 
   // Copy the hardware information to the SYSTEM_INFO structure. 
 
   GetSystemInfo(&siSysInfo); 
 
   // Display the contents of the SYSTEM_INFO structure. 

   printf("Hardware information: \n");  
   printf("  OEM ID: %u\n", siSysInfo.dwOemId);
   printf("  Number of processors: %u\n", 
      siSysInfo.dwNumberOfProcessors); 
   printf("  Page size: %u\n", siSysInfo.dwPageSize); 
   printf("  Processor type: %u\n", siSysInfo.dwProcessorType); 
   printf("  Minimum application address: %lx\n", 
      siSysInfo.lpMinimumApplicationAddress); 
   printf("  Maximum application address: %lx\n", 
      siSysInfo.lpMaximumApplicationAddress); 
   printf("  Active processor mask: %u\n", 
      siSysInfo.dwActiveProcessorMask); 
}



void shellcodeexample(){
	//www.dreamincode.net/forums/topic/156507-using-virtualalloc-virtualprotect-and-virtualfree

/* In the beginning we'll have to define the function pointer.  */ 
/* I called the function 'dyncode' and gave it an int argument  */ 
/* as well as an int return value just to show what's possible. */ 
 
int (*dyncode)(int);  /* prototype for call of dynamic code */ 
 
/* The following char array is initialized with some binary code */ 
/* which takes the first argument from the stack, increases it,  */ 
/* and returns to the caller.                                    */ 
/* Just very simple code for testing purposes...                 */ 
 
unsigned char code[] = {0x8B,0x44,0x24,0x04,  /* mov eax, [esp+4] */ 
                        0x40,                 /* inc eax          */ 
                        0xC3                  /* ret              */ 
                       }; 
 
/* Include the prototypes of the functions we are using... */ 
        /* To show you that the code can be dynamically generated    */ 
        /* although I defined static data above, the code is copied  */ 
        /* into an allocated memory area and the starting address is */ 
        /* assigned to the function pointer 'dyncode'.               */ 
        /* The strange stuff in front of the malloc is just to cast  */ 
        /* the address to the same format the function pointer is    */ 
        /* definded with, otherwise you'd get a compiler warning.    */ 
 
        dyncode = (int (*)(int)) VirtualAlloc(NULL, 4096,
                                              MEM_COMMIT, PAGE_EXECUTE_READWRITE);

        /* We now have a page of memory that is readable, writeable    */
        /* and executable. so the memcpy will work without any         */
        /* problems!                                                   */

        memcpy(dyncode, code, sizeof(code)); 
 
        /* To show that the code works it is called with the argument 41 */ 
        /* and the retval sould be 42, obviously.                        */ 
 
        /* This code will now execute correctly!                         */

        printf("retval = %d\n", (*dyncode)(41) );  /* call the code and print the return value */   

        /* Freeing the page allocated.                                   */

        VirtualFree(dyncode, NULL, MEM_RELEASE);
        return;
}

/* A program to demonstrate the use of guard pages of memory. Allocate
   a page of memory as a guard page, then try to access the page. That
   will fail, but doing so releases the lock on the guard page, so the
   next access works correctly.

   The output will look like this. The actual address may vary.

   This computer has a page size of 4096.
   Committed 4096 bytes at address 0x00520000
   Cannot lock at 00520000, error = 0x80000001
   2nd Lock Achieved at 00520000

   This sample does not show how to use the guard page fault to
   "grow" a dynamic array, such as a stack. */
void virtualAllocExample(){
  LPVOID lpvAddr,lpvAddr2;               // address of the test memory
  DWORD dwPageSize;             // amount of memory to allocate.
  BOOL bLocked;                 // address of the guarded memory
  SYSTEM_INFO sSysInfo;         // useful information about the system

  GetSystemInfo(&sSysInfo);     // initialize the structure

  _tprintf(TEXT("This computer has page size %d.\n"), sSysInfo.dwPageSize);
  dwPageSize = sSysInfo.dwPageSize;

  // Try to allocate the memory.

  lpvAddr = VirtualAlloc(NULL,    //system determines where to allocate region
						 17*dwPageSize,
                         MEM_RESERVE | MEM_COMMIT, // reserve and commit memory, no detail yet, should be fine for now
                         PAGE_EXECUTE_READ );//| PAGE_GUARD

  if(lpvAddr == NULL) {
    _tprintf(TEXT("VirtualAlloc failed. Error: %ld\n"),
             GetLastError());
    return ;

  } else {
    _ftprintf(stderr, TEXT("Committed %lu Kbytes at address 0x%lp\n"),
              17*dwPageSize/1024, lpvAddr);
  }

  // Try to allocate the memory for second time 

    lpvAddr2 = VirtualAlloc(NULL,    //system determines where to allocate region
						 4*dwPageSize,
                         MEM_RESERVE | MEM_COMMIT, // reserve and commit memory, no detail yet, should be fine for now
                         PAGE_EXECUTE_READ );//| PAGE_GUARD

  if(lpvAddr2 == NULL) {
    _tprintf(TEXT("VirtualAlloc failed. Error: %ld\n"),
             GetLastError());
    return ;

  } else {
    _ftprintf(stderr, TEXT("Committed %lu Kbytes at address 0x%lp\n"),
              4*dwPageSize/1024, lpvAddr2);
  }

  // Try to lock the committed memory. This fails the first time 
  // because of the guard page.

  bLocked = VirtualLock(lpvAddr, dwPageSize);
  if (!bLocked) {
    _ftprintf(stderr, TEXT("Cannot lock at %lp, error = 0x%lx\n"),
              lpvAddr, GetLastError());
  } else {
    _ftprintf(stderr, TEXT("Lock Achieved at %lp\n"), lpvAddr);
  }

  // Try to lock the committed memory again. This succeeds the second
  // time because the guard page status was removed by the first 
  // access attempt.

  bLocked = VirtualLock(lpvAddr, dwPageSize);

  if (!bLocked) {
    _ftprintf(stderr, TEXT("Cannot get 2nd lock at %lp, error = %lx\n"),
              lpvAddr, GetLastError());
  } else {
    _ftprintf(stderr, TEXT("2nd Lock Achieved at %lp\n"), lpvAddr);
  }



  return;
}

int main()
{
shellcodeexample();
//getSystemInfoExample();
//GetListofProcessIdentifiers();
//system ("start notepad");
virtualAllocExample();


  
  std::cin.get();
  std::cin.ignore();
  return 0;
}
