#include "ZdjCallback.h"
#include <ntimage.h>

extern PFLT_FILTER	Filter;

NTSTATUS GetFileNameForCallBackData(PFLT_CALLBACK_DATA Data,PUNICODE_STRING	Name) { 
	NTSTATUS	Status;

	PFLT_FILE_NAME_INFORMATION	FileNameInfo;
	Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP, &FileNameInfo);// 这里该了一下
	if (!NT_SUCCESS(Status)) goto _Exit;

	Status = FltParseFileNameInformation(FileNameInfo);
	if (!NT_SUCCESS(Status)) goto _Exit;

	*Name = FileNameInfo->Name;

_Exit:
	FltReleaseFileNameInformation(FileNameInfo);

	return	Status;

}
//IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION

FLT_PREOP_CALLBACK_STATUS ZdjPreCallback(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID *CompletionContext
)
{
	UNICODE_STRING	Name = { 0};
	NTSTATUS	Status;

	switch (Data->Iopb->MajorFunction)
	{
	case IRP_MJ_READ:
		Status = GetFileNameForCallBackData(Data, &Name);
		if (NT_SUCCESS(Status))
		{
			//DbgPrint("[Read] Path < %wZ >\n", Name);
		}
		break;
	case IRP_MJ_WRITE:
		Status = GetFileNameForCallBackData(Data, &Name);
		if (NT_SUCCESS(Status))
		{
			//DbgPrint("[Write] Path < %wZ >\n", Name);
		}
		break;
	case	IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION:
		if (
				Data->Iopb->Parameters.AcquireForSectionSynchronization.PageProtection == PAGE_EXECUTE ||
				Data->Iopb->Parameters.AcquireForSectionSynchronization.PageProtection == PAGE_EXECUTE_READ ||
				Data->Iopb->Parameters.AcquireForSectionSynchronization.PageProtection == PAGE_EXECUTE_READWRITE
			)
		{//
			//DbgBreakPoint();
			GetFileNameForCallBackData(Data, &Name);
			HANDLE	Id = PsGetCurrentProcessId();
			DbgPrint("[Section] Path < %wZ >  id=%d\n", Name,Id);

			if (PsGetCurrentProcessId() == 1856) //保护的进程PID
			{
				DbgBreakPoint();
				PVOID	Buf = NULL;
				try
				{
					
					FILE_STANDARD_INFORMATION	FileInfo = { 0 };

					Status = FltQueryInformationFile(FltObjects->Instance, FltObjects->FileObject, &FileInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation, NULL);

					Buf = ExAllocatePoolWithTag(NonPagedPool, FileInfo.AllocationSize.QuadPart, 'zdjS');
					LARGE_INTEGER	Len = { 0 };
					Len.QuadPart = 0;
					Status = FltReadFile(FltObjects->Instance, FltObjects->FileObject, &Len, FileInfo.AllocationSize.QuadPart, Buf, FLTFL_IO_OPERATION_NON_CACHED | FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET, NULL, NULL, NULL);

					if (*(PUSHORT)Buf == 0x5A4D) // 是否是PE文件
					{
						ExFreePoolWithTag(Buf, 'zdjS');
						return STATUS_ACCESS_DENIED;
					}
				
					ExFreePoolWithTag(Buf, 'zdjS');

				}
				finally
				{
					if (!Buf) {
						ExFreePoolWithTag(Buf, 'zdjS');
					}
					
				}
			}
			

		}
		
		
	default:
		Data->IoStatus.Information = 0;
		break;
	}

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;

}


NTSTATUS	ZdjFilterUnloadCallBack(
	FLT_FILTER_UNLOAD_FLAGS Flags
) {
	FltUnregisterFilter(Filter);
	Flags = FLTFL_FILTER_UNLOAD_MANDATORY;
	DbgPrint("ZdjMiniFilter Unload.\n");

	return STATUS_SUCCESS;
}