#pragma once
#include <ntifs.h>
#include <fltkernel.h>

NTSTATUS	ZdjMiniFilterInit(PUNICODE_STRING	Reg, PWCHAR	Altitude);
NTSTATUS	ZdjMiniFilterRegisterFilter(PDRIVER_OBJECT	pDriverObj);
NTSTATUS	ZdjMiniFilter(PDRIVER_OBJECT	pDriverObj, PUNICODE_STRING	Reg);