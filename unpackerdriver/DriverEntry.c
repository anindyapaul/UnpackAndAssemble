// http://www.rohitab.com/discuss/topic/40560-get-process-name-form-pid-in-kernel-mode-driver/ : Create Process Callback
//
#include "stdio.h"
#include "stdlib.h"
#include "ntddk.h"
 #include <string.h>
 #include "multicpu.h"
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+--+-+-+-+-+-+-+-+-+-+-+-+-+*/
typedef BOOLEAN BOOL;
typedef unsigned long DWORD;
typedef DWORD * PDWORD;
typedef unsigned long ULONG;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned long* PPTE;
typedef LARGE_INTEGER PHYSICAL_MEMORY;


void *g_OldInt0EHandler = NULL;
void *g_Int03Handler = NULL;
void *g_Int01Handler = NULL;
BOOL hooked = FALSE;
BOOL dummy  = FALSE;
//DWORD procidMalware = 0;

/* Function Prototypes */
VOID MyDriver_Unload(PDRIVER_OBJECT  );    
NTSTATUS MyDriver_UnSupportedFunction(PDEVICE_OBJECT DeviceObject, PIRP Irp);



//Global Variables
HANDLE procidMalware = 0; 
#define PROCESS_PAGE_DIR_BASE           0xC0300000
#define PROCESS_PAGE_TABLE_BASE         0xC0000000
#define KERNEL_ADDRESS_START		0x80000000
static DWORD MmSystemPteBase=0xc0000000;
PPTE GetPteAddress( PVOID VirtualAddress );
void HookMemoryPage( PVOID pReadWritePage );
void UnhookMemoryPage( PVOID pReadWritePage );
void MarkPageSupervisorOnly( void *pte );
void MarkPageUserMode( void *pte );
void FixCoW( void *pte );
void *SetInterruptHandler(int IntNo, void *HandlerPtr, BYTE flags);
void *GetInterruptHandler(DWORD IntNo, DWORD IdtNum);
void InstallHandler();
void RemoveHandler();
void mainfunction();
NTSTATUS DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
    );
 
VOID ProcessCallback(
    HANDLE hParentId,
    HANDLE hProcessId,
    BOOLEAN bCreate
    );
     
void GetProcName(
    HANDLE PID,
    PUNICODE_STRING ProcName
    );
char * GetProcessNameFromPid(HANDLE pid);
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+--+-+-+-+-+-+-+-+-+-+-+-+-+*/
extern UCHAR *PsGetProcessImageFileName(IN PEPROCESS Process);
 
extern   NTSTATUS PsLookupProcessByProcessId(
     HANDLE ProcessId,
    PEPROCESS *Process
);
typedef PCHAR (*GET_PROCESS_IMAGE_NAME) (PEPROCESS Process);
GET_PROCESS_IMAGE_NAME gGetProcessImageFileName;
 
 
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+--+-+-+-+-+-+-+-+-+-+-+-+-+*/
char * GetProcessNameFromPid(HANDLE pid)
{
    PEPROCESS Process;
    if (PsLookupProcessByProcessId(pid, & Process) == STATUS_INVALID_PARAMETER)
    {
        return "pid???";
    }
    return (CHAR*)PsGetProcessImageFileName(Process);
}
 
 
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+--+-+-+-+-+-+-+-+-+-+-+-+-+*/
VOID ProcessCallback(
    HANDLE hParentId,  
    HANDLE hProcessId,
    BOOLEAN bCreate)
{
 PUNICODE_STRING ProcName=NULL;
 char *str  = "calc.exe";
 char *str2 = GetProcessNameFromPid(hProcessId);
 
 if(strcmp(str,str2) == 0){
	 DbgPrint("calc.exe is working..");
	 procidMalware = hProcessId;
	 
	 // hook all pages belongs to this process
	 hookProcessPages();
 }
 
 
 
    if (bCreate) {
		
     DbgPrint("Process [%s] Created \n", GetProcessNameFromPid(hProcessId));
    }
    else {
     DbgPrint( "Process [%s] Terminate \n", GetProcessNameFromPid(hProcessId));
    }
	
	DbgPrint("Process Id: %d",hProcessId);
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+--+-+-+-+-+-+-+-+-+-+-+-+-+*/
void DriverUnload(PDRIVER_OBJECT pDriverObject) {
    PsSetCreateProcessNotifyRoutine(ProcessCallback, TRUE);
    DbgPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_INFO_LEVEL,"Driver unloading\n");
}
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+--+-+-+-+-+-+-+-+-+-+-+-+-+*/
DRIVER_INITIALIZE DriverEntry;
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject,PUNICODE_STRING RegistryPath)
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
    unsigned int uiIndex = 0;
    PDEVICE_OBJECT pDeviceObject = NULL;
    UNICODE_STRING usDriverName, usDosDeviceName;
    RtlInitUnicodeString(&usDriverName, L"\\Device\\MyDriver");
    RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevices\\MyDriver"); 
	
	
    DbgPrint( "Driver Load\n");
    DbgPrint( "---------------------------------------\n");

 
    PsSetCreateProcessNotifyRoutine(&ProcessCallback, FALSE);

	
	DbgPrint("Process is created");
	
	// at this point we have process id of program\
	// In another function we should hook Virtual Alloc and that function should return allocated pages
	// Original permissions should be saved, write permission should be removed.
	
	//Hook all pages allocated to process
		//HookMemoryPage(processPage);
	
	//
		
    DriverObject->DriverUnload = DriverUnload;
	
	    NtStatus = IoCreateDevice(DriverObject, 0, &usDriverName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);

    if(NtStatus == STATUS_SUCCESS) {
        /* MajorFunction: is a list of function pointers for entry points into the driver. */
        for(uiIndex = 0; uiIndex < IRP_MJ_MAXIMUM_FUNCTION; uiIndex++)
             DriverObject->MajorFunction[uiIndex] = MyDriver_UnSupportedFunction;

			 
		/* DriverUnload is required to be able to dynamically unload the driver. */
    
        pDeviceObject->Flags |= 0;
		pDeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);
		
        /* Create a Symbolic Link to the device. MyDriver -> \Device\MyDriver */
        IoCreateSymbolicLink(&usDosDeviceName, &usDriverName);
		
		/* hook IDT */
		mainfunction();
    }

	
	
	
	
    return STATUS_SUCCESS;
}

// hook all pages allocated to process
void hookProcessPages(){
	
	DbgPrint("Hook Process Pages");
	//while(!procidMalware) dummy = TRUE;
}

///


// new interrupt handler
static __declspec( naked ) NewInt0EHandler()
{
        __asm
        {
                pushad
		push fs
		mov bx, 0x30
		mov fs, bx
                mov edx, dword ptr [esp+0x20+4]	//PageFault.ErrorCode
		test edx, 1		// if the fault was caused by a 
		je PassDown		// nonpresent page, pass it down

		test edx, 4
                je PassDown    		// it's a kernelmode pagefault

                mov eax, cr2 	// the faulting address
		cmp eax, KERNEL_ADDRESS_START
		jnb PassDown	// we don't hook kernel pages, pass it down

                mov esi, PROCESS_PAGE_DIR_BASE
                mov ebx, eax
                shr ebx, 22
                lea ebx, [esi + ebx*4]  //ebx = pPTE for large page
                test [ebx], 0x80        //check if its a large page
                jnz PassDown		//pass it down if it is

                mov esi, PROCESS_PAGE_TABLE_BASE
                mov ebx, eax
                shr ebx, 12
                lea ebx, [esi + ebx*4]  //ebx = pPTE
		mov ecx, [ebx]
		test ecx, 0x01	// valid?
		je PassDown

		mov ecx, [ebx]
		test ecx, 0x04	// user-accessible? if so it's not ours
		jnz PassDown

/*
		// now we hope we are dealing with one of our hooked pages
		mov ecx, [esp+0x24+4]
		// log the handler execution
		pushad
		push eax
		push ecx
		mov ecx, [ebx]
		push ecx
		lea ebx, formatMsg
		push ebx
		lea edx, buf
		push edx 		
		mov edi, [sprintf]
		call edi
		add esp, 0x14
		lea edx, buf
		push edx 		
		call DbgPrint
		add esp, 0x4
		popad
*/
                cmp [esp+0x24+4], eax     //Is due to an attempted execute?
                jne LoadDTLB

		pop fs
		popad
                add esp,4		// strip PF error code
		jmp [g_Int01Handler]	// divert to int1 trap handler

LoadDTLB:
                or dword ptr [ebx], 0x04           //mark the page user-acessible
                mov eax, dword ptr [eax]           //load the DTLB
                and dword ptr [ebx], 0xFFFFFFFB    //re-mark page supervisor-only
		pop fs
                popad
                add esp,4
		iretd

PassDown:
		pop fs
		popad
                jmp [g_OldInt0EHandler]

        }
}


// HookMemoryPage

void HookMemoryPage( PVOID pPage )
{
	char buf[100];
	void *pReadWritePte;

        pReadWritePte = GetPteAddress( pPage );
	sprintf(buf, "Hooking page va:%x ppte:%x pte:%x ",
			pPage, pReadWritePte, *(DWORD *)pReadWritePte);
	
	DbgPrint(buf);
	__asm {	
		mov eax, pPage	// access page in case it is paged out
		mov eax, [eax]
		cli
	}

        if( hooked == FALSE )
        {      
		InstallHandler();
        	hooked = TRUE;
        }

        pReadWritePte = GetPteAddress( pPage );

	__asm {	
			mov eax, pPage
			invlpg [eax]
	}
	// Pages marked Copy-on-Write won't work, change them to normal perms
        FixCoW( pReadWritePte );

        //Mark the page supervisor mode only
        MarkPageSupervisorOnly( pReadWritePte );

	__asm {	
			mov eax, pPage
			invlpg [eax]
			sti  //reenable interrupts
	}
	sprintf(buf, "  New pte:%x\n", *(DWORD *)pReadWritePte);
	DbgPrint(buf);
	
}

PPTE GetPteAddress( PVOID VirtualAddress )
{
        PPTE pPTE = 0;
        __asm
        {
                cli                     //disable interrupts
                pushad
                mov esi, PROCESS_PAGE_DIR_BASE
                mov edx, VirtualAddress
                mov eax, edx
                shr eax, 22
                lea eax, [esi + eax*4]  //pointer to page directory entry
                test [eax], 0x80        //is it a large page?
                jnz Is_Large_Page       //it's a large page
                mov esi, PROCESS_PAGE_TABLE_BASE
                shr edx, 12
                lea eax, [esi + edx*4]  //pointer to page table entry (PTE)
                mov pPTE, eax
                jmp Done

                //NOTE: There is not a page table for large pages because
                //the phys frames are contained in the page directory.
                Is_Large_Page:
                mov pPTE, eax

                Done:
                popad
                sti                    //reenable interrupts
        }//end asm

        return pPTE;

}//end GetPteAddress

void MarkPageSupervisorOnly( void *pte )
{
	*(DWORD *)pte &= 0xFFFFFFFB;
}
void MarkPageUserMode( void *pte )
{
	*(DWORD *)pte |= 0x4;
}
void FixCoW( void *pte )
{
	*(DWORD *)pte &= 0xFFFFFDFF;	// mark CoW off
	*(DWORD *)pte |= 0x2;		// mark writable
	*(DWORD *)pte |= 0x40;		// Dirty, dirty page
}

void UnhookMemoryPage( PVOID pPage )
{
	void *pReadWritePte;

	__asm {	
		mov eax, pPage	// access page in case it is paged out
		mov eax, [eax]
		cli
	}

        pReadWritePte = GetPteAddress( pPage );

	__asm {	
			mov eax, pPage
			invlpg [eax]
	}
        MarkPageUserMode( pReadWritePte );

	__asm {	
			mov eax, pPage
			invlpg [eax]
			sti  //reenable interrupts
		}
}

void InstallHandler() 
{
	g_OldInt0EHandler = GetInterruptHandler(0x0e,0);
	g_Int03Handler = GetInterruptHandler(0x03,0);
	g_Int01Handler = GetInterruptHandler(0x01,0);
	SetInterruptHandler(0x0E,&NewInt0EHandler,0x8e);
}

void RemoveHandler()
{
	if (g_OldInt0EHandler != NULL)
		SetInterruptHandler(0x0E, g_OldInt0EHandler, 0x8e);
}

void* GetInterruptHandler(DWORD IntNo, DWORD IdtNum)
{
   DWORD IDTBase=0;
   DWORD OldIntr;
   WORD  *IdtEntry;

   IDTBase = mp_GetIDTBase(IdtNum);

   IdtEntry=(WORD*)(IDTBase+IntNo*8);

   OldIntr=IdtEntry[0]+(IdtEntry[3]<<16);

   return (void*)OldIntr;
}

void* SetInterruptHandler(int IntNo, void *HandlerPtr, BYTE flags)
{

   __asm
   {
          pushad

          mov    eax, IntNo
          mov    dl,  flags
          mov    edi, HandlerPtr
          mov    bx,  cs

          cli                       ; TO DO: StopSecondCPUs()
          call   mp_HookInterrupt
          sti

          popad
   }

   return (void*)mp_OldHandler;
}

/*
 * MyDriver_UnSupportedFunction: called when a major function is issued that isn't supported.
 */
NTSTATUS MyDriver_UnSupportedFunction(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    NTSTATUS NtStatus = STATUS_NOT_SUPPORTED;
    DbgPrint("MyDriver_UnSupportedFunction Called \r\n");

    return NtStatus;
}