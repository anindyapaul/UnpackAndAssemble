#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <tchar.h>
#include <psapi.h>
#include <excpt.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "psapi.lib")
using namespace std;

//Globals
DWORD dwPageSize = 4096;
LPTSTR lpNxtPage;               // Address of the next page to ask for


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

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{//stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
   
	//Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
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

void IdentifyProcess( DWORD pid )
{
	PrintMemoryInfo( pid );
    return;
}


VOID ErrorExit(LPTSTR lpMsg)
{
    _tprintf(TEXT("Error! %s with error code of %ld.\n"),
             lpMsg, GetLastError ());
    exit (0);
}
 
INT PageFaultExceptionFilter(DWORD dwCode,HANDLE hProcess)
{
    LPVOID lpvResult;
	_tprintf(TEXT("Exception function running.\n"));
    // If the exception is not a page fault, exit.

    if (dwCode != EXCEPTION_ACCESS_VIOLATION)
    {
        _tprintf(TEXT("Exception code = %d.\n"), dwCode);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    _tprintf(TEXT("Exception is a page fault.\n"));

    // If the reserved pages are used up, exit.

  /*  if (dwPages >= PAGELIMIT)
    {
        _tprintf(TEXT("Exception: out of pages.\n"));
        return EXCEPTION_EXECUTE_HANDLER;
    }
	*/

    // Otherwise, commit another page.

    lpvResult = VirtualAllocEx(hProcess,
                     (LPVOID) lpNxtPage, // Next page to commit
                     dwPageSize,         // Page size, in bytes
                     MEM_COMMIT,         // Allocate a committed page
                     PAGE_READWRITE);    // Read/write access
    if (lpvResult == NULL )
    {
        _tprintf(TEXT("VirtualAlloc failed.\n"));
        return EXCEPTION_EXECUTE_HANDLER;
    }
    else
    {
        _tprintf(TEXT("Allocating another page.\n"));
    }

    // Increment the page count, and advance lpNxtPage to the next page.

//    dwPages++;
    lpNxtPage = (LPTSTR) ((PCHAR) lpNxtPage + dwPageSize);

    // Continue execution where the page fault occurred.

    return EXCEPTION_CONTINUE_EXECUTION;
}



// Create Process

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
        0,              // CREATE_SUSPENDED =4
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
    
	IdentifyProcess(pi.dwProcessId);

	// Allocate memory to process

	LPVOID lpvBase= VirtualAllocEx(pi.hProcess,
				    NULL,
				    5*4096,    // number of pages to be allocated
					MEM_RESERVE ,//initially just reserved, if neccessary commit!!
				    PAGE_EXECUTE_READ);// start with X and R permissions

	if (lpvBase == NULL )
        ErrorExit(TEXT("VirtualAlloc reserve failed."));
	
	
	// Wait until child process exits.
	WaitForSingleObject( pi.hProcess,5000 );//INFINITE, 5000ms

	// change protection
	int failornot;
	DWORD dPermission=0;
	LPTSTR lpPtr;                 // Generic character pointer
	lpPtr = lpNxtPage = (LPTSTR) lpvBase;
	//string lastError;
	__try
	{
	failornot=VirtualProtectEx(pi.hProcess,
					 lpvBase,
					 2,
					 PAGE_READWRITE,
					 &dPermission);
	 cout << "Dpermission: " << &dPermission << "failornot:" << failornot;
	 //lastError = GetLastErrorAsString();
	}
	 __except( PageFaultExceptionFilter( GetExceptionCode(),pi.hProcess ) )
	 {
		 // This code is executed only if the filter function
            // is unsuccessful in committing the next page.

            _tprintf (TEXT("Exiting process.\n"));

            ExitProcess( GetLastError() );
	 }

  //  cout << "GetLastError: " << lastError << ".";
	printf("Process resumed");
	//ResumeThread(pi.hThread);
	IdentifyProcess(pi.dwProcessId);
    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

return pi.dwProcessId;
}


int main(int argc, TCHAR *argv[])
{

//Step 0: Create Process
//first part program name, second part argument
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
