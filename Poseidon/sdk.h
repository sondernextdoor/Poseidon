#pragma once
#include "sharedmemory.h"

namespace Driver {

	INT64 NTAPI EnumerateDebuggingDevicesHook(PVOID A1, PINT64 A2) {
		if (ExGetPreviousMode() != UserMode
		    || A1 == nullptr 
		    || !Utils::ProbeUserAddress(A1, sizeof(gData), sizeof(DWORD)) 
		    || !Memory::Copy(&gData, A1, sizeof(CommunicationData))
		    || gData.Magic != 0x999) {
		
			return EnumerateDebuggingDevicesOriginal(A1, A2);
		} 

		InterlockedExchangePointer((PVOID*)gFunc, (PVOID)EnumerateDebuggingDevicesOriginal);
		System::ClearPiDDBCache(L"iqvw64e.sys", gKernelBase);
		SharedMemory::Loop();
	}

	NTSTATUS Initialize() {
		if (gKernelBase = System::GetModuleInfo<char*>("ntoskrnl.exe")) {
			if (auto Func = Utils::FindPatternImage(gKernelBase, 
								"\x48\x8B\x05\x00\x00\x00\x00\x75\x07\x48\x8B\x05\x00\x00\x00\x00\xE8\x00\x00\x00\x00", 
								"xxx????xxxxx????x????")) {

				gFunc = (DWORD64)(Func = (BYTE*)Func + *(INT*)((BYTE*)Func + 3) + 7);
				*(PVOID*)&EnumerateDebuggingDevicesOriginal = InterlockedExchangePointer((PVOID*)Func, (PVOID)EnumerateDebuggingDevicesHook);
				DbgPrint("%s:", "[ + ] Driver Loaded, Function Hooked!");
				return STATUS_SUCCESS;
			}
		}  

		return STATUS_UNSUCCESSFUL;
	}
}
