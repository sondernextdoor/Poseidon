#pragma once
#include "global.h"
#include "sharedmemory.h"

namespace Process {

	DWORD GetProcessId(const wchar_t* ImageName) {
		HANDLE Snapshot{ CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };
		PROCESSENTRY32W Process{ sizeof(PROCESSENTRY32W) };

		if (Process32FirstW(Snapshot, &Process)) {
			do {
				if (!wcscmp(ImageName, Process.szExeFile)) {
					CloseHandle(Snapshot);
					return Process.th32ProcessID;
				}
			} while (Process32NextW(Snapshot, &Process));
		}

		CloseHandle(Snapshot);
		return NULL;
	}

	PVOID GetBase(DWORD ProcessId) {
		OperationData Data{ 0 };
		Data.Process.Id = ProcessId;
		
		if (SharedMemory::SendRequest(BaseRequest, Data)) {
			return SharedMemory::GetBuffer().Process.BaseAddress;
		} return nullptr;
	}

	DWORD GetSize(DWORD ProcessId) {
		OperationData Data{ 0 };
		Data.Process.Id = ProcessId;

		if (SharedMemory::SendRequest(SizeRequest, Data)) {
			return SharedMemory::GetBuffer().Process.Size;
		} return NULL;
	}

	template <typename T>
	T GetModuleInfo(DWORD ProcessId, const char* ModuleName, DWORD &OutSize) {
		OperationData Data{ 0 };

		Data.Process.Id = ProcessId;
		Data.Process.Name = const_cast<char*>(ModuleName);

		if (SharedMemory::SendRequest(ModuleRequest, Data)) {
			OperationData Buffer{ SharedMemory::GetBuffer() };
			OutSize = Buffer.Module.SizeOfImage;
			return reinterpret_cast<T>(Buffer.Module.BaseAddress);
		}

		return {};
	}

	bool QueryInformation(DWORD ProcessId, PROCESS_BASIC_INFORMATION& PBI) {
		OperationData Data{ 0 };

		Data.Process.Id = ProcessId;

		if (SharedMemory::SendRequest(QIPRequest, Data)) {
			PBI = SharedMemory::GetBuffer().Process.PBI;
		}

		return PBI.PebBaseAddress ? true : false;
	}

	PPEB GetPeb(DWORD ProcessId) {
		OperationData Data{ 0 };
		Data.Process.Id = ProcessId;

		if (SharedMemory::SendRequest(PebRequest, Data)) {
			return SharedMemory::GetBuffer().Process.Peb;
		} return nullptr;
	}

	DWORD GetModuleCount(DWORD ProcessId) {
		OperationData Data{ 0 };

		Data.Process.Id = ProcessId;

		if (SharedMemory::SendRequest(ModuleRequest, Data)) {
			return SharedMemory::GetBuffer().Module.Index;
		} return NULL;
	}

	PVOID GetModuleByIndex(DWORD ProcessId, DWORD Index, DWORD& OutSize) {
		PVOID Base{ nullptr };
		OperationData Data{ 0 };

		Data.Process.Id = ProcessId;
		Data.Module.Index = Index;

		if (SharedMemory::SendRequest(IndexRequest, Data)) {
			OperationData Buffer{ SharedMemory::GetBuffer() };
			Base = Buffer.Module.BaseAddress;
			OutSize = Buffer.Module.SizeOfImage;
		}

		return Base;
	}
}