#pragma once

extern "C" {
	NTSYSAPI
		PIMAGE_NT_HEADERS
		NTAPI
		RtlImageNtHeader(PVOID Base);

	NTSYSAPI
		NTSTATUS
		NTAPI
		ZwProtectVirtualMemory(
			__in HANDLE ProcessHandle,
			__inout PVOID* BaseAddress,
			__inout PSIZE_T RegionSize,
			__in ULONG NewProtect,
			__out PULONG OldProtect
		);

	NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(
		IN HANDLE			ProcessId,
		OUT PEPROCESS* Process
	);

	NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(
		IN PEPROCESS		Process
	);

	NTSTATUS
		ZwQuerySystemInformation(
			defs::SYS_INFO_CLASS SystemInformationClass,
			PVOID SystemInformation,
			ULONG SystemInformationLength,
			ULONG* ReturnLength);

	NTKERNELAPI
		PVOID
		NTAPI
		RtlFindExportedRoutineByName(
			_In_ PVOID ImageBase,
			_In_ PCCH RoutineNam
		);

	NTSTATUS NTAPI MmCopyVirtualMemory(
		PEPROCESS SourceProcess,
		PVOID SourceAddress,
		PEPROCESS TargetProcess,
		PVOID TargetAddress,
		SIZE_T BufferSize,
		KPROCESSOR_MODE PreviousMode,
		PSIZE_T ReturnSize
	);

	NTSTATUS ZwAllocateVirtualMemory(
		_In_    HANDLE    ProcessHandle,
		_Inout_ PVOID* BaseAddress,
		_In_    ULONG_PTR ZeroBits,
		_Inout_ PSIZE_T   RegionSize,
		_In_    ULONG     AllocationType,
		_In_    ULONG     Protect
	);

	NTKERNELAPI PPEB NTAPI PsGetProcessPeb(IN PEPROCESS Process);


	NTKERNELAPI PVOID NTAPI PsGetThreadWin32Thread(__in PETHREAD 	Thread);


	NTKERNELAPI PVOID NTAPI PsSetThreadWin32Thread(_Inout_ PETHREAD 	Thread,
		_In_opt_ PVOID 	Win32Thread,
		_In_opt_ PVOID 	OldWin32Thread
	);

	NTKERNELAPI
		PVOID NTAPI
		PsGetCurrentProcessWin32Process(
			VOID
		);

	NTKERNELAPI PVOID NTAPI
		PsGetCurrentThreadWin32Thread(
			VOID
		);

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
			_Out_ PVOID* Object
		);
}