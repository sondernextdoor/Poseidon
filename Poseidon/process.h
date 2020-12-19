#pragma once
#include "global.h"

namespace Process {

	PEPROCESS GetProcess(DWORD ProcessId) {
		PEPROCESS eProcess{ nullptr };
		PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(ProcessId), &eProcess);
		return eProcess;
	}

	NTSTATUS GetBaseAddress(OperationData* Data) {
		PEPROCESS eProcess{ GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		Data->Process.BaseAddress = PsGetProcessSectionBaseAddress(eProcess);

		ObfDereferenceObject(eProcess);
		return Data->Process.BaseAddress ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	}

	NTSTATUS GetMainModuleSize(OperationData* Data) {
		KAPC_STATE Apc{ 0 };
		DWORD Size{ NULL };
		PEPROCESS eProcess{ GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		if (LIST_ENTRY* ModuleEntry{ PsGetProcessPeb(eProcess)->Ldr->InLoadOrderModuleList.Flink }) {
			Data->Process.Size = CONTAINING_RECORD(ModuleEntry, 
												   LDR_DATA_TABLE_ENTRY, 
												   InLoadOrderLinks)->SizeOfImage;
		}

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);

		return Data->Process.Size ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	}

	NTSTATUS GetPeb(OperationData* Data) {
		PEPROCESS eProcess{ GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		Data->Process.Peb = PsGetProcessPeb(eProcess);

		ObfDereferenceObject(eProcess);
		return Data->Process.Peb ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	}

	NTSTATUS QueryInformation(OperationData* Data) {
		KAPC_STATE Apc{ 0 };
		PEPROCESS eProcess{ GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		NTSTATUS Status{ ZwQueryInformationProcess(ZwCurrentProcess(), 
												   ProcessBasicInformation, 
												   &Data->Process.PBI, 
												   sizeof(Data->Process.PBI),
												   nullptr) };

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS GetModuleInfo(OperationData* Data) {
		KAPC_STATE Apc{ 0 };
		PVOID Base{ nullptr };
		DWORD Size{ NULL};
		UNICODE_STRING usModule{ 0 };

		if (Data->Process.Name) {
			ANSI_STRING asModule{ 0 };

			RtlInitAnsiString(&asModule, Data->Process.Name);
			if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&usModule, &asModule, TRUE))) {
				return STATUS_UNSUCCESSFUL;
			}
		}

		PEPROCESS eProcess{ GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		LIST_ENTRY* List = &(PsGetProcessPeb(eProcess)->Ldr->InLoadOrderModuleList);

		for (LIST_ENTRY* Entry = List->Flink; Entry != List;) {
			auto Module{ CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks) };

			if (Module) {
				++Data->Module.Index;

				if (Data->Process.Name && !RtlCompareUnicodeString(&Module->BaseDllName, &usModule, TRUE)) {
					Data->Module.BaseAddress = Module->DllBase;
					Data->Module.SizeOfImage = Module->SizeOfImage;
				}
			}

			Entry = Module->InLoadOrderLinks.Flink;
		}

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Data->Module.SizeOfImage ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	}


	NTSTATUS GetModuleInfoByIndex(OperationData* Data) {
		KAPC_STATE Apc{ 0 };
		int Count{ 0 };
		PEPROCESS eProcess{ GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		LIST_ENTRY* List = &(PsGetProcessPeb(eProcess)->Ldr->InLoadOrderModuleList);

		for (LIST_ENTRY* Entry = List->Flink; Entry != List;) {
			auto Module{ CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks) };
		
			if (Module && Count == Data->Module.Index) {
				Data->Module.BaseAddress = Module->DllBase;
				Data->Module.SizeOfImage = Module->SizeOfImage;
				break;
			}

			Count += 1;
			Entry = Module->InLoadOrderLinks.Flink;
		}

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return STATUS_SUCCESS;
	}
}