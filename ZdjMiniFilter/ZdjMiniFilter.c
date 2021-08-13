#include "ZdjMiniFilter.h"
#include "ZdjCallback.h"

PFLT_FILTER	Filter = NULL;

NTSTATUS	ZdjCreateKey(__in PWCHAR	CreateKeyPach,__out PHANDLE	KeyHandle) {
	UNICODE_STRING	KeyString = { 0 };
	ULONG	Disposition = 0;
	OBJECT_ATTRIBUTES	ObjectAttribute = { 0 };

	RtlInitUnicodeString(&KeyString, CreateKeyPach);

	InitializeObjectAttributes(&ObjectAttribute, &KeyString, OBJ_CASE_INSENSITIVE, NULL, NULL);
	
	
	return	ZwCreateKey(KeyHandle, KEY_ALL_ACCESS, &ObjectAttribute, 0, NULL, REG_OPTION_VOLATILE, &Disposition);
}

NTSTATUS	ZdjSetKey(__in  HANDLE	KeyHandle,ULONG	Type,PVOID	Key,PVOID	Value,ULONG	Size) {
	UNICODE_STRING	KeyString = { 0 };
	RtlInitUnicodeString(&KeyString, Key);

	return	ZwSetValueKey(KeyHandle, &KeyString, 0, Type, Value, Size);
}

NTSTATUS	ZdjMiniFilterInit(PUNICODE_STRING	Reg , PWCHAR	Altitude) {
	NTSTATUS	Status;
	HANDLE	KeyHandle;
	WCHAR	RegPatch[256] = { 0 };
	WCHAR	Instance[] = L"Mini Instance";
	ULONG	Flags = 0;

	memcpy(RegPatch, Reg->Buffer, Reg->Length);
	wcscat(RegPatch, L"\\Instances");

	Status = ZdjCreateKey(RegPatch, &KeyHandle);
	if (!NT_SUCCESS(Status))	goto _EXIT;

	Status = ZdjSetKey(KeyHandle,REG_SZ,L"DefaultInstance", Instance, sizeof(Instance));
	if (!NT_SUCCESS(Status))	goto _EXIT;

	ZwClose(KeyHandle);



	wcscat(RegPatch, L"\\Mini Instance");
	Status = ZdjCreateKey(RegPatch, &KeyHandle);
	if (!NT_SUCCESS(Status))	goto _EXIT;

	Status = ZdjSetKey(KeyHandle,REG_SZ,L"Altitude", Altitude, 2 * wcslen(Altitude));
	if (!NT_SUCCESS(Status))	goto _EXIT;

	Status = ZdjSetKey(KeyHandle, REG_DWORD, L"Flags", &Flags, 4);
	if (!NT_SUCCESS(Status))	goto _EXIT;


_EXIT:
	if (KeyHandle)	Status = ZwClose(KeyHandle);

	return	Status;
}

NTSTATUS	ZdjRegisterFilter(PDRIVER_OBJECT	pDriverObj) {
	FLT_REGISTRATION	FilterReg = { 0 };
	FLT_OPERATION_REGISTRATION	OperationReg[4] = {0};

	RtlZeroMemory(
		&FilterReg,
		sizeof(FilterReg));

	RtlZeroMemory(
		&OperationReg,
		sizeof(OperationReg));


	FilterReg.Size = sizeof(FLT_REGISTRATION);
	FilterReg.Version = FLT_REGISTRATION_VERSION;
	FilterReg.FilterUnloadCallback = ZdjFilterUnloadCallBack;
	FilterReg.OperationRegistration = OperationReg;

	OperationReg[0].Flags = FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO;
	OperationReg[0].MajorFunction = IRP_MJ_READ;
	OperationReg[0].PreOperation = ZdjPreCallback;

	OperationReg[1].Flags = FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO;
	OperationReg[1].MajorFunction = IRP_MJ_WRITE;
	OperationReg[1].PreOperation = ZdjPreCallback;


	//OperationReg[2].Flags = 0;
	OperationReg[2].MajorFunction = IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION;
	OperationReg[2].PreOperation = ZdjPreCallback;
	

	OperationReg[3].MajorFunction = IRP_MJ_OPERATION_END;//×¢ÒâÒ»ÏÂ


	return	FltRegisterFilter(pDriverObj, &FilterReg, &Filter);
}


NTSTATUS	ZdjMiniFilter(PDRIVER_OBJECT	pDriverObj,PUNICODE_STRING	Reg) {
	NTSTATUS	Status = STATUS_SUCCESS;;
	DbgBreakPoint();
	Status	= ZdjMiniFilterInit(Reg, L"321007");
	if (!NT_SUCCESS(Status)) {
		DbgBreakPoint();
		return	STATUS_UNSUCCESSFUL;
	}

	Status = ZdjRegisterFilter(pDriverObj);
	if (!NT_SUCCESS(Status)) {
		DbgBreakPoint();
		return	STATUS_UNSUCCESSFUL;
	}

	Status = FltStartFiltering(Filter);
	if (!NT_SUCCESS(Status)) {
		DbgBreakPoint();
		return	STATUS_UNSUCCESSFUL;
	}


	return	Status;
}

