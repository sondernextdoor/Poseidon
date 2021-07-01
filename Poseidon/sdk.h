#pragma once
#include "sharedmemory.h"

#define RVA(addr, size) (BYTE*)addr + *(INT*)((BYTE*)addr + ((size) - 4)) + size

namespace Driver {

	INT64 NTAPI EnumerateDebuggingDevicesHook(PVOID A1, PINT64 A2) {
		if (ExGetPreviousMode() != UserMode
		    || A1 == nullptr 
		    || !Utils::ProbeUserAddress(A1, sizeof(gData), sizeof(DWORD)) 
		    || !Memory::Copy(&gData, A1, sizeof(CommunicationData))
		    || gData.Magic != 0x999) {

			// NtConvertBetweenAuxiliaryCounterAndPerformanceCounter() was not called by our usermode client
			// Call the original EnumerateDebuggingDevices() for whoever called

			return EnumerateDebuggingDevicesOriginal(A1, A2);
		} 

		// NtConvertBetweenAuxiliaryCounterAndPerformanceCounter() was called by the usermode client
		// We're only able to execute code right now because the usermode thread within the client transitioned into the kernel to complete the system call
		// We can take advantage of this and execute code in our driver for as long as we want by simply never returning

		InterlockedExchangePointer((PVOID*)gFunc, (PVOID)EnumerateDebuggingDevicesOriginal);
		// Unhook EnumerateDebuggingDevices() - it can be detected easily
		// But AC won't you ban simply for leaving a hook, they can flag you, but not ban, there's why: https://www.unknowncheats.me/forum/2461143-post15.html
		SharedMemory::Loop();
	}

	NTSTATUS Initialize() {
		auto OSInfo{ System::GetOSVersion() };

		if (OSInfo.dwBuildNumber < 19041) {
			ActiveThreadsOffset = OSInfo.dwBuildNumber == 10240 ? 0x490 : 0x498;
		}

		if (gKernelBase = System::GetModuleInfo<char*>("ntoskrnl.exe")) {
			if (auto Func = Utils::FindPatternImage(gKernelBase, 
								"\x48\x8B\x05\x00\x00\x00\x00\x75\x07\x48\x8B\x05\x00\x00\x00\x00\xE8\x00\x00\x00\x00", 
								"xxx????xxxxx????x????")) {

				gFunc = (DWORD64)(Func = RVA(Func, 7));
				*(PVOID*)&EnumerateDebuggingDevicesOriginal = InterlockedExchangePointer((PVOID*)Func, (PVOID)EnumerateDebuggingDevicesHook); // Hook EnumerateDebuggingDevices()
				return STATUS_SUCCESS;
			}
		}

		return STATUS_UNSUCCESSFUL;
	}
}