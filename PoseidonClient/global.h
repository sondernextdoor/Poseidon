#pragma once
#include <Windows.h>
#include <winternl.h>
#include <TlHelp32.h>
#include <iostream>
#include <vector>

PVOID(NTAPI *NtConvertBetweenAuxiliaryCounterAndPerformanceCounter)(PVOID, PVOID, PVOID, PVOID);

enum Code {
	Complete,
	BaseRequest,
	SizeRequest,
	PebRequest,
	QIPRequest,
	CopyRequest,
	AVMRequest,
	FVMRequest,
	PVMRequest,
	QVMRequest,
	ModuleRequest,
	IndexRequest
};

enum Status {
	Inactive,
	Active,
	Waiting,
	Exit
};

typedef struct OperationData {

	struct {
		char*   Name;
		DWORD	Id;
		PVOID	BaseAddress;
		SIZE_T  Size;
		PPEB	Peb;
		PROCESS_BASIC_INFORMATION PBI;
	} Process;

	struct {
		SIZE_T Size;
		SIZE_T ReturnLength;

		struct {
			PVOID Address;
			PVOID Buffer;
			BOOLEAN	ReadOperation;
		} Copy;

		PVOID Base;
		DWORD AllocType;
		DWORD FreeType;
		DWORD Protect;
		DWORD OldProtect;
		MEMORY_BASIC_INFORMATION MBI;
	} Memory;

	struct {
		PVOID BaseAddress;
		SIZE_T SizeOfImage;
		int Index;
	} Module;
};

typedef struct CommunicationData {

	DWORD	ProcessId;
	PVOID	SharedMemory;
	DWORD*	pCode;
	SHORT*	pStatus;
	DWORD	Magic;
};

typedef struct MODULE {
	PVOID BaseAddress;
	DWORD SizeOfImage;
};