#include <ntifs.h>
#include "ZdjMiniFilter.h"


VOID UnloadDriver(PDRIVER_OBJECT	pDriverObj) {

}

NTSTATUS	DriverEntry(PDRIVER_OBJECT	pDriverObj,PUNICODE_STRING	pReg) {
	NTSTATUS	Status = STATUS_SUCCESS;
	DbgBreakPoint();
	Status  = ZdjMiniFilter(pDriverObj, pReg);

	return	Status;
}