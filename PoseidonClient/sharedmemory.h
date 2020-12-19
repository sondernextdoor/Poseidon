#pragma once
#include "global.h"

namespace SharedMemory {

	CommunicationData Data{ 0 };
	BOOL UseOnce{ true };
	INT Queue{ 0 };


	void PushQueue() {
		Queue += 1;
	}

	void PopQueue() {
		Queue -= 1;
	}

	BOOL WriteSharedMemory(PVOID Address, PVOID Value, SIZE_T Size) {
		return reinterpret_cast<BOOL>(memcpy(Address, Value, Size));
	}

	template <typename T>
	T ReadSharedMemory(PVOID Address, SIZE_T Size = sizeof(T)) {
		T Ret{ 0 };
		memcpy(static_cast<PVOID>(&Ret), Address, Size);
		return Ret;
	}

	BOOL SetStatus(Status Status) {
		return WriteSharedMemory(Data.pStatus, &Status, sizeof(SHORT));
	}

	BOOL SetCode(DWORD Code) {
		return WriteSharedMemory(Data.pCode, &Code, sizeof(DWORD));
	}

	BOOL SetBuffer(OperationData Buffer) {
		return WriteSharedMemory(Data.SharedMemory, &Buffer, sizeof(OperationData));
	}

	Status GetStatus() {
		return static_cast<Status>(ReadSharedMemory<SHORT>(Data.pStatus));
	}

	DWORD GetCode() {
		return ReadSharedMemory<DWORD>(Data.pCode);
	}

	OperationData GetBuffer() {
		return ReadSharedMemory<OperationData>(Data.SharedMemory);
	}

	void Connect(CommunicationData InitData, BOOL bUseOnce) {
		Data = InitData;
		UseOnce = bUseOnce;
		SetStatus(Active);
		SetCode(Complete);
	}

	BOOL SendRequest(Code Request, OperationData Data) {

		do {
			Sleep(10);
		} while (GetCode() != Complete 
			 || GetStatus() != Active 
			 || Queue >= 1);

		PushQueue();

		if (SetBuffer(Data)) {
			if (SetCode(Request)) {
				if (SetStatus(Waiting)) {

					do {
						Sleep(10);
					} while (GetCode() != Complete || GetStatus() != Active);

					PopQueue();
					return true;
				}
			}
		}

		PopQueue();
		return false;
	}

	void Disconnect() {
		SetStatus(UseOnce ? Exit : Inactive);
	}
};
