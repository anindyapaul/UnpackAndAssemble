// http://www.rohitab.com/discuss/topic/40560-get-process-name-form-pid-in-kernel-mode-driver/ : Create Process Callback
//

#include <ntddk.h>
 
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+--+-+-+-+-+-+-+-+-+-+-+-+-+*/
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
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    DbgPrint( "Driver Load\n");
    DbgPrint( "---------------------------------------\n");
 
 
    PsSetCreateProcessNotifyRoutine(&ProcessCallback, FALSE);
    DriverObject->DriverUnload = DriverUnload;
    return STATUS_SUCCESS;
}
