#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <IntSafe.h>
#include <ntimage.h>

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
	IndexRequest,
	NameRequest,
};

enum Status {
	Inactive,	// We'll use this status to let the driver know it can sleep for a while
	Active,		// We'll use this status to let the driver know we may be sending requests any second
	Waiting,	// We'll use this status to let the driver know we sent a request and are waiting for completion
	Exit		// We'll use this status to let the driver know it can exit the shared memory loop and untrap our thread
};

typedef struct OperationData {

	struct {
		char* Name;
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
		const char *Name;
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

INT64(NTAPI *EnumerateDebuggingDevicesOriginal)(PVOID, PVOID);
CommunicationData gData{};
PEPROCESS gProcess{};
DWORD64 gFunc{};
CHAR* gKernelBase{};
DWORD ActiveThreadsOffset{ 0x5F0 };

typedef enum _SYSTEM_INFORMATION_CLASS 
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemNextEventIdInformation,
	SystemEventIdsInformation,
	SystemCrashDumpInformation,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemPlugPlayBusInformation,
	SystemDockInformation,
	SystemProcessorSpeedInformation,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation
} SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;

typedef struct _PEB_LDR_DATA 
{
	ULONG Length;
	UCHAR Initialized;
	PVOID SsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY 
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB 
{
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	UCHAR BitField;
	PVOID Mutant;
	PVOID ImageBaseAddress;
	PPEB_LDR_DATA Ldr;
	PVOID ProcessParameters;
	PVOID SubSystemData;
	PVOID ProcessHeap;
	PVOID FastPebLock;
	PVOID AtlThunkSListPtr;
	PVOID IFEOKey;
	PVOID CrossProcessFlags;
	PVOID KernelCallbackTable;
	ULONG SystemReserved;
	ULONG AtlThunkSListPtr32;
	PVOID ApiSetMap;
} PEB, * PPEB;

typedef struct _SYSTEM_MODULE 
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[MAXIMUM_FILENAME_LENGTH];
} SYSTEM_MODULE, *PSYSTEM_MODULE;

typedef struct _SYSTEM_MODULE_INFORMATION 
{
	ULONG NumberOfModules;
	SYSTEM_MODULE Modules[1];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[MAXIMUM_FILENAME_LENGTH];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

typedef struct PiDDBCache
{
	LIST_ENTRY		List;
	UNICODE_STRING	DriverName;
	ULONG			TimeDateStamp;
	NTSTATUS		LoadStatus;
	char			_0x0028[16];
};

extern "C" 
{
	NTKERNELAPI
	PVOID
	PsGetProcessSectionBaseAddress(
		PEPROCESS Process
	);

	NTKERNELAPI
	PPEB
	NTAPI
	PsGetProcessPeb(
		PEPROCESS Process
	);

	NTKERNELAPI
	NTSTATUS
	MmCopyVirtualMemory(
		PEPROCESS SourceProcess,
		PVOID SourceAddress,
		PEPROCESS TarGet,
		PVOID TargetAddress,
		SIZE_T BufferSize,
		KPROCESSOR_MODE PreviousMode,
		PSIZE_T ReturnSize
	);

	NTSYSCALLAPI 
	NTSTATUS 
	NTAPI 
	ZwQuerySystemInformation(
		ULONG InfoClass,
		PVOID Buffer, 
		ULONG Length,
		PULONG ReturnLength
	);

	NTSYSCALLAPI
	NTSTATUS 
	ZwQueryInformationProcess(
		HANDLE ProcessHandle,
		PROCESSINFOCLASS ProcessInformationClass,
		PVOID ProcessInformation,
		ULONG ProcessInformationLength,
		PULONG ReturnLength
	);

	NTSYSCALLAPI 
	NTSTATUS 
	NTAPI 
	ZwProtectVirtualMemory(
		HANDLE ProcessHandle, 
		PVOID *BaseAddress, 
		PSIZE_T RegionSize,
		ULONG NewAccessProtection, 
		PULONG OldAccessProtection
	);
}