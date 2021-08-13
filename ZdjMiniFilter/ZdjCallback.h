#pragma once
#include <ntifs.h>
#include <fltkernel.h>

FLT_PREOP_CALLBACK_STATUS ZdjPreCallback(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID *CompletionContext
);

NTSTATUS	ZdjFilterUnloadCallBack(
	FLT_FILTER_UNLOAD_FLAGS Flags
);