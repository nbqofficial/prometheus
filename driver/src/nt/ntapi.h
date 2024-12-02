// DRIVER

#pragma once

#include "ntstructs.h"

extern "C"
NTSYSAPI
NTSTATUS
NTAPI
ObReferenceObjectByName(
	_In_ PUNICODE_STRING ObjectName,
	_In_ ULONG Attributes,
	_In_opt_ PACCESS_STATE AccessState,
	_In_opt_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_TYPE ObjectType,
	_In_ KPROCESSOR_MODE AccessMode,
	_Inout_opt_ PVOID ParseContext,
	_Out_ PVOID * Object
);

extern "C"
NTKERNELAPI
PVOID
NTAPI
RtlFindExportedRoutineByName(
	PVOID ImageBase,
	PCCH RoutineName
);

extern "C"
NTKERNELAPI
NTSTATUS
ZwQuerySystemInformation(
	ULONG InfoClass,
	PVOID Buffer,
	ULONG Length,
	PULONG ReturnLength
);

extern "C"
NTSYSAPI
PIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader(
	PVOID   ModuleAddress
);

extern "C"
NTKERNELAPI
PPEB
NTAPI
PsGetProcessPeb(
	IN PEPROCESS Process
);

extern "C"
NTKERNELAPI
PVOID
NTAPI
PsGetProcessWow64Process(
	IN PEPROCESS Process
);

extern "C"
NTKERNELAPI
PVOID
PsGetProcessSectionBaseAddress(
	_In_ PEPROCESS Process
);