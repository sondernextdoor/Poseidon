#pragma once
#include "process.h"

namespace Memory {

	template <typename T = PVOID> 
	T Allocate(SIZE_T Size) {
		return reinterpret_cast<T>(ExAllocatePool(NonPagedPool, Size));
	}

	VOID Free(PVOID Buffer) {
		ExFreePool(Buffer);
	}

	BOOLEAN Copy(PVOID Destination, PVOID Source, SIZE_T Size) {
		SIZE_T BytesRead{ 0 };
		return NT_SUCCESS(MmCopyVirtualMemory(IoGetCurrentProcess(), 
						      Source, 
						      IoGetCurrentProcess(), 
						      Destination, 
						      Size, 
						      KernelMode, 
						      &BytesRead)) && BytesRead == Size;
	}

	NTSTATUS CopyVirtualMemory(OperationData* Data) {
		NTSTATUS Status{ STATUS_SUCCESS };
		PEPROCESS eProcess{ Process::GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		if (Data->Memory.Copy.ReadOperation) {
			Status = MmCopyVirtualMemory(eProcess, 
						     Data->Memory.Copy.Address, 
						     IoGetCurrentProcess(), 
						     Data->Memory.Copy.Buffer, 
						     Data->Memory.Size, 
						     UserMode, 
						     &Data->Memory.ReturnLength);
		} else {
			Status = MmCopyVirtualMemory(IoGetCurrentProcess(), 
						     Data->Memory.Copy.Buffer,
						     eProcess, 
						     Data->Memory.Copy.Address, 
						     Data->Memory.Size, 
						     UserMode, 
						     &Data->Memory.ReturnLength);
		}

		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS AllocateVirtualMemory(OperationData* Data) {
		KAPC_STATE Apc{ NULL };
		PEPROCESS eProcess{ Process::GetProcess(Data->Process.Id) };

		if (!eProcess) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		NTSTATUS Status{ ZwAllocateVirtualMemory(ZwCurrentProcess(), 
							 &Data->Memory.Base, 
							 NULL, 
							 &Data->Memory.Size,
							 Data->Memory.AllocType,
							 Data->Memory.Protect) };

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS FreeVirtualMemory(OperationData* Data) {
		KAPC_STATE Apc{ NULL };
		PEPROCESS eProcess{ Process::GetProcess(Data->Process.Id) };

		if (!eProcess) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		NTSTATUS Status{ ZwFreeVirtualMemory(ZwCurrentProcess(),
						     &Data->Memory.Base,
						     &Data->Memory.Size,
						     Data->Memory.FreeType) };

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS ProtectVirtualMemory(OperationData* Data) {
		KAPC_STATE Apc{ NULL };
		PEPROCESS eProcess{ Process::GetProcess(Data->Process.Id) };

		if (!eProcess) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		NTSTATUS Status{ ZwProtectVirtualMemory(ZwCurrentProcess(), 
							&Data->Memory.Base, 
							&Data->Memory.Size,
							Data->Memory.Protect,
							&Data->Memory.OldProtect) };

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS QueryVirtualMemory(OperationData* Data) {
		NTSTATUS Status{ STATUS_SUCCESS };
		KAPC_STATE Apc{ 0 };
		PEPROCESS eProcess{ Process::GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		Status = ZwQueryVirtualMemory(ZwCurrentProcess(), 
					      Data->Memory.Base,
					      MemoryBasicInformation, 
					      &Data->Memory.MBI, 
					      sizeof(Data->Memory.MBI),
					      &Data->Memory.ReturnLength);

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}
}
