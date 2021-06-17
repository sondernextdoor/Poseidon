#pragma once
#include "memory.h"

namespace Client {
	bool ErrorFlag{ false };

	void KernelThread(PVOID LParam) {
		INT64 Status{ 0 };

		CommunicationData Data{ *(CommunicationData*)LParam };
		PVOID pData{ &Data };

		HMODULE Module{ LoadLibrary(L"ntdll.dll") };

		if (!Module) {
			return;
		}

		*(PVOID*)&NtConvertBetweenAuxiliaryCounterAndPerformanceCounter = GetProcAddress(Module, "NtConvertBetweenAuxiliaryCounterAndPerformanceCounter");

		if (!NtConvertBetweenAuxiliaryCounterAndPerformanceCounter) {
			return;
		}
		// Endless function, so if it success, there's an error
		NtConvertBetweenAuxiliaryCounterAndPerformanceCounter((PVOID)1, &pData, &Status, nullptr);
		ErrorFlag = true;
	}

	void Connect() {
		CommunicationData Data{ 0 };

		PVOID Memory{ VirtualAlloc(nullptr, 
					   sizeof(OperationData) * 2, 
					   MEM_COMMIT | MEM_RESERVE, 
					   PAGE_READWRITE) };

		if (!Memory) {
			return;
		}

		Data.ProcessId = GetCurrentProcessId();
		Data.SharedMemory = Memory;
		Data.pCode = (DWORD*)Memory + sizeof(OperationData);
		Data.pStatus = (SHORT*)Data.pCode + 8;
		Data.Magic = 0x999;

		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)KernelThread, &Data, 0, nullptr);

		Sleep(500);

		if (ErrorFlag) {
			std::cout << "Error Connecting";
			getchar();
			exit(0);
		}

		SharedMemory::Connect(Data);
	}

	void Disconnect() {
		SharedMemory::Disconnect();
	}
}

class KProcess {
public:

	wchar_t* ImageName{};
	DWORD ProcessId{};
	PVOID BaseAddress{};
	DWORD Size{};
	PPEB Peb{};
	DWORD ModuleCount{};
	std::vector <MODULE> ModuleList{};

	KProcess(const wchar_t* ImageName) {
		this->ImageName = const_cast<wchar_t*>(ImageName);
		this->ProcessId = Process::GetProcessId(ImageName);
		this->BaseAddress = Process::GetBase(this->ProcessId);
		this->Size = Process::GetSize(this->ProcessId);
		this->Peb = Process::GetPeb(this->ProcessId);
		this->ModuleCount = Process::GetModuleCount(this->ProcessId);

		for (int i = 0; i < this->ModuleCount; i++) {
			DWORD SizeOfImage{ 0 };
			PVOID BaseAddress{ Process::GetModuleByIndex(this->ProcessId, i, SizeOfImage) };
			ModuleList.push_back({ BaseAddress, SizeOfImage });
		}
	}

	template <typename T>
	T GetModuleInfo(const char* ModuleName, DWORD &OutSize) {
		return Process::GetModuleInfo<T>(this->ProcessId, ModuleName, OutSize);
	}

	PROCESS_BASIC_INFORMATION QueryInformationProcess() {
		PROCESS_BASIC_INFORMATION Pbi{ 0 };
		Process::QueryInformation(this->ProcessId, Pbi);
		return Pbi;
	}

	bool Read(PVOID Address, PVOID Buffer, SIZE_T Size) {
		return Memory::Read(this->ProcessId, Address, Buffer, Size);
	}
	
	template <typename T>
	T Read(PVOID Address, SIZE_T Size = sizeof(T)) {
		return Memory::Read<T>(this->ProcessId, Address, Size);
	}

	bool Write(PVOID Address, PVOID Buffer, SIZE_T Size) {
		return Memory::Write(this->ProcessId, Address, Buffer, Size);
	}

	template <typename T>
	bool Write(PVOID Address, T Value, SIZE_T Size = sizeof(T)) {
		return Memory::Write<T>(this->ProcessId, Address, Value, Size);
	}

	PVOID AllocateVirtualMemory(PVOID Base, SIZE_T Size, DWORD AllocType, DWORD Protect) {
		return Memory::AllocateVirtualMemory(this->ProcessId, Base, Size, AllocType, Protect);
	}

	bool FreeVirtualMemory(PVOID Base, SIZE_T Size, DWORD FreeType) {
		return Memory::FreeVirtualMemory(this->ProcessId, Base, Size, FreeType);
	}
	
	DWORD ProtectVirtualMemory(PVOID Base, SIZE_T Size, DWORD Protect, DWORD* OldProtect) {
		return Memory::ProtectVirtualMemory(this->ProcessId, Base, Size, Protect, OldProtect);
	}

	bool QueryVirtualMemory(PVOID Address, MEMORY_BASIC_INFORMATION& MemoryBasicInfo, SIZE_T Size) {
		return Memory::QueryVirtualMemory(this->ProcessId, Address, MemoryBasicInfo, Size);
	}

	MEMORY_BASIC_INFORMATION QueryVirtualMemory(PVOID Address, SIZE_T Size) {
		MEMORY_BASIC_INFORMATION Mbi{ 0 };
		this->QueryVirtualMemory(Address, Mbi, Size);
		return Mbi;
	}

	BYTE* PatternFinder(BYTE* Start, DWORD Size, const char* Signature, const char* Mask) {
		auto CompareData = [] (const char* Data, const char* Signature, const char* Mask) -> BOOL {
			for (; *Mask; ++Mask, ++Data, ++Signature) {
				if (*Mask == 'x' && *Data != *Signature) {
					return FALSE;
				}
			}
			return (*Mask == NULL);
		};

		auto Buffer{ static_cast<char*>(VirtualAlloc(nullptr, Size, MEM_COMMIT, PAGE_READWRITE)) };
		this->Read(Start, Buffer, Size);

		for (DWORD64 i = 0; i < Size; i++) {
			if (CompareData(Buffer + i, Signature, Mask)) {
				VirtualFree(Buffer, 0, MEM_RELEASE);
				return Start + i;
			}
		}

		VirtualFree(Buffer, NULL, MEM_RELEASE);
		return NULL;
	}

	BYTE* AbsoluteAddress(BYTE* Rip, DWORD InstructionLength) {
		DWORD RelativeOffset{ 0 };
		this->Read(Rip + InstructionLength - 4, &RelativeOffset, sizeof(DWORD));
		return Rip + InstructionLength + RelativeOffset;
	}

	BYTE* RelativeAddress(BYTE* DestinationAddress, BYTE* SourceAddress, DWORD InstructionLength) {
		return reinterpret_cast<BYTE*>(reinterpret_cast<uint64_t>(SourceAddress) 
					       - InstructionLength 
					       - reinterpret_cast<uint64_t>(DestinationAddress));
	}
};
