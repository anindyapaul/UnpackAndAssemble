#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <tchar.h>
#include <psapi.h>
#include <excpt.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "psapi.lib")
using namespace std;
//using namespace System;

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

// exceptionExample 1

DWORD FilterFunction() 
{ 
    printf("1 ");                     // printed first 
    return EXCEPTION_EXECUTE_HANDLER; 
}

void exceptionExample(){
__try 
    { 
        __try 
        { 
            RaiseException( 
                1,                    // exception code 
                0,                    // continuable exception 
                0, NULL);             // no arguments 
        } 
        __finally 
        { 
            printf("2 ");             // this is printed second 
        } 
    } 
    __except ( FilterFunction() ) 
    { 
        printf("3\n");                // this is printed last 
    } 


}

//exception example 2

int k;
int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep) {
puts("in filter.");
if (code == EXCEPTION_ACCESS_VIOLATION) {   // if due to page permission, let OS solves problem
puts("caught AV as expected.");
return EXCEPTION_CONTINUE_SEARCH;
}
else if (code == STATUS_INTEGER_DIVIDE_BY_ZERO){
puts("division with 0");
k=0;
return EXCEPTION_EXECUTE_HANDLER;// exception function written by user -> used if exception is due to WxorE policy
}
else{
puts("didn't catch AV, unexpected."); // 
return EXCEPTION_CONTINUE_EXECUTION;  // having solved the problem u can use this

};

}

void exceptionExample2(){
int* p = 0x00000000; // pointer to NULL
puts("hello");
__try{
puts("in try");
__try{
puts("in try");
//*p = 13; // causes an access violation exception;
k=5;
int t=0;
k=k/t;
}__finally{
puts("in finally. termination: ");
puts(AbnormalTermination() ? "\tabnormal" : "\tnormal");
}
}__except(filter(GetExceptionCode(), GetExceptionInformation())){
puts("in except");
// do something here if EXCEPTION_EXECUTE_HANDLER returns from filter
}
puts("world");

}

// Exception Example 3

void ResetVars( int ) {puts("Variable Resetted");}  
int Eval_Exception ( int n_except ) {  
   if ( n_except != STATUS_INTEGER_DIVIDE_BY_ZERO &&   
      n_except != STATUS_FLOAT_OVERFLOW )   // Pass on most exceptions  
   return EXCEPTION_CONTINUE_SEARCH;  
  
   // Execute some code to clean up problem  
   ResetVars( 0 );   // initializes data to 0  
   return EXCEPTION_CONTINUE_EXECUTION;  
}  

void exceptionExample3(){

	 int Eval_Exception( int );  
  
   __try {
    int k=5;
	int t=0;
	k=k/t;
   
   }  
  
   __except ( Eval_Exception( GetExceptionCode( ))) {  
      ;  
   } 

}



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


void IdentifyProcess( DWORD pid )
{
	PrintMemoryInfo( pid );
    return;
}
 

DWORD createProcessExample(char *processName){
	
	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

   // Start the child process. 
	
    if( !CreateProcess(NULL,   // No module name (use command line)
        processName,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return NULL;
    }

	printf("Process Id:%d.\n",pi.dwProcessId);
    // Wait until child process exits.
	IdentifyProcess(pi.dwProcessId);
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

return pi.dwProcessId;
}


int main(int argc, TCHAR *argv[])
{

//Step 0: Create Process
char processName [] = "C:\\WINDOWS\\system32\\notepad.exe C:\\sado.txt";
DWORD  pid = createProcessExample(processName);

// Step 1:
//Initialization part-> All pages belongs to process
//should be started with only read and execute permissions.


//Step 2:
//Packet malware can decompress itself. 
//Write and Read permissions should be granted to new allocated these new pages.


//Step 3:
//If Page fault occurs, check original permissions and grant either X or W permission to page.


//Step 4: 
//Hooking System Calls


//Step 5:
//In case dangerous syscalls, terminate process and dump the content.


//shellcodeexample();
//getSystemInfoExample();
//GetListofProcessIdentifiers();
//system ("start notepad");
//virtualAllocExample();
//exceptionExample();
//exceptionExample2();
//exceptionExample3();
  puts("MAIN");
  std::cin.get();
  std::cin.ignore();
  return 0;
}
