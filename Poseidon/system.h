#pragma once
#include "global.h"
#include "utils.h"

namespace System {

	template <typename T = PVOID>
	T GetModuleInfo(const char* Name, DWORD* OutSize = nullptr) {
		PVOID Base{ nullptr };
		DWORD RequiredSize{ 0 };

		if (ZwQuerySystemInformation(SystemModuleInformation, 
				             nullptr,
					     NULL,
					     &RequiredSize) != STATUS_INFO_LENGTH_MISMATCH) {

			return reinterpret_cast<T>(nullptr);
		}

		auto Modules{ Memory::Allocate<SYSTEM_MODULE_INFORMATION*>(RequiredSize) };

		if (!Modules) {
			return reinterpret_cast<T>(nullptr);
		}

		if (!NT_SUCCESS(ZwQuerySystemInformation(SystemModuleInformation, 
							 Modules, 
							 RequiredSize, 
							 nullptr))) {
			Memory::Free(Modules);
			return reinterpret_cast<T>(nullptr);
		}

		for (DWORD i = 0; i < Modules->NumberOfModules; ++i) {
			SYSTEM_MODULE CurModule{ Modules->Modules[i] };

			if (strstr(Utils::LowerStr((CHAR*)CurModule.FullPathName), Name)) 
			{
				Base = CurModule.ImageBase;

				if (OutSize) {
					*OutSize = CurModule.ImageSize;
				}

				break;
			}
		}

		Memory::Free(Modules);
		return reinterpret_cast<T>(Base);
	}

	OSVERSIONINFOW GetOSVersion() {
		OSVERSIONINFOW OSInfo{ 0 };
		RtlGetVersion(&OSInfo);
		return OSInfo;
	}
}
