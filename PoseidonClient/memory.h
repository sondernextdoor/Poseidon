#pragma once
#include "process.h"

namespace Memory {

	bool Read(DWORD ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size) {
		OperationData Data{ 0 };

		Data.Process.Id = ProcessId;
		Data.Memory.Copy.Address = Address;
		Data.Memory.Copy.Buffer = Buffer;
		Data.Memory.Size = Size;
		Data.Memory.Copy.ReadOperation = true;

		return SharedMemory::SendRequest(CopyRequest, Data);
	}
	
	template <typename T>
	T Read(DWORD ProcessId, PVOID Address, SIZE_T Size = sizeof(T)) {
		T Buffer{};
		Read(ProcessId, Address, static_cast<PVOID>(&Buffer), Size);
		return Buffer; 
	}

	bool Write(DWORD ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size) {
		OperationData Data{ 0 };

		Data.Process.Id = ProcessId;
		Data.Memory.Copy.Address = Address;
		Data.Memory.Copy.Buffer = Buffer;
		Data.Memory.Size = Size;
		Data.Memory.Copy.ReadOperation = false;

		return SharedMemory::SendRequest(CopyRequest, Data);
	}

	template <typename T>
	bool Write(DWORD ProcessId, PVOID Address, T Value, SIZE_T Size = sizeof(T)) {
		OperationData Data{ 0 };

		Data.Process.Id = ProcessId;
		Data.Memory.Copy.Address = Address;
		Data.Memory.Copy.Buffer = &Value;
		Data.Memory.Size = Size;
		Data.Memory.Copy.ReadOperation = false;

		return SharedMemory::SendRequest(CopyRequest, Data);
	}

	PVOID AllocateVirtualMemory(DWORD ProcessId, PVOID Base, SIZE_T Size, DWORD AllocType, DWORD Protect) {
		OperationData Data{ 0 };

		Data.Process.Id = ProcessId;
		Data.Memory.Base = Base;
		Data.Memory.Size = Size;
		Data.Memory.AllocType = AllocType;
		Data.Memory.Protect = Protect;

		SharedMemory::SendRequest(AVMRequest, Data);
		return SharedMemory::GetBuffer().Memory.Base;
	}

	bool FreeVirtualMemory(DWORD ProcessId, PVOID Base, SIZE_T Size, DWORD FreeType) {
		OperationData Data{ 0 };

		Data.Process.Id = ProcessId;
		Data.Memory.Base = Base;
		Data.Memory.Size = Size;
		Data.Memory.AllocType = FreeType;

		return SharedMemory::SendRequest(FVMRequest, Data);
	}
	
	DWORD ProtectVirtualMemory(DWORD ProcessId, PVOID Base, SIZE_T Size, DWORD Protect, DWORD* OldProtect = nullptr) {
		OperationData Data{ 0 };

		Data.Process.Id = ProcessId;
		Data.Memory.Base = Base;
		Data.Memory.Size = Size;
		Data.Memory.Protect = Protect;

		if (SharedMemory::SendRequest(PVMRequest, Data)) {
			OperationData Buffer{ SharedMemory::GetBuffer() };

			if (OldProtect) {
				*OldProtect = Buffer.Memory.OldProtect;
			}

			return Buffer.Memory.Protect;
		}
	}

	bool QueryVirtualMemory(DWORD ProcessId, PVOID Address, MEMORY_BASIC_INFORMATION& MemoryBasicInfo, SIZE_T Size) {
		OperationData Data{ 0 };

		Data.Process.Id = ProcessId;
		Data.Memory.Base = Address;
		Data.Memory.Size = Size;

		if (SharedMemory::SendRequest(QVMRequest, Data)) {
			MemoryBasicInfo = SharedMemory::GetBuffer().Memory.MBI;
		}

		return MemoryBasicInfo.Protect ? true : false;
	}
}