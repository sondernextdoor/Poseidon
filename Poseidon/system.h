#pragma once
#include "global.h"
#include "utils.h"

namespace System {

	template <typename T = PVOID>
	T GetModuleInfo(const CHAR* Name, DWORD* OutSize = nullptr) {
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

	VOID ClearPiDDBCache(wchar_t* DriverName, char* KernelBase) {
		if (!DriverName) {
			return;
		}

		auto Dereference = [] (BYTE* Address, DWORD Offset) -> BYTE* {
			return (!Address ? nullptr : Address + (DWORD)((*(DWORD*)(Address + Offset) + Offset) + sizeof(DWORD)));
		};

		UNICODE_STRING IntelDriver{ 0 };
		RtlInitUnicodeString(&IntelDriver, DriverName);

		if (KernelBase) {

			auto PiDDBCacheTable{ 
				(PRTL_AVL_TABLE)Dereference((BYTE*)Utils::FindPatternImage(KernelBase, 
											"\x48\x8D\x0D\x00\x00\x00\x00\x4C\x89\x35\x00\x00\x00\x00\x49\x8B\xE9", 
											"xxx????xxx????xxx"), 
											3) };

			if (!PiDDBCacheTable) {
				PiDDBCacheTable = (PRTL_AVL_TABLE)Dereference((BYTE*)Utils::FindPatternImage(KernelBase, 
					"\x48\x8D\x0D\x00\x00\x00\x00\x4C\x89\x35\x00\x00\x00\x00\xBB\x00\x00\x00\x00", 
					"xxx????xxx????x????"), 
					3);

				if (!PiDDBCacheTable) {
					PiDDBCacheTable = (PRTL_AVL_TABLE)Dereference((BYTE*)Utils::FindPatternImage(KernelBase, 
						"\x48\x8D\x0D\x00\x00\x00\x00\x49\x8B\xE9", 
						"xxx????xxx"), 
						3);

					if (PiDDBCacheTable) {
						DWORD64 EntryAddress{ 
							DWORD64(PiDDBCacheTable->BalancedRoot.RightChild) + sizeof(RTL_BALANCED_LINKS) 
						};

						auto Entry { (PiDDBCache*)(EntryAddress) };

						if (!RtlCompareUnicodeString(&Entry->DriverName, &IntelDriver, TRUE)) {
							Entry->TimeDateStamp = 0x8394BE4;
							Entry->DriverName = RTL_CONSTANT_STRING(L"OCModule.sys");
							DbgPrint("%s:", "[+] Cache Cleared!");
						}

						DWORD Count{ 0 };

						for (auto Link{ Entry->List.Flink }; 
							Link != Entry->List.Blink; 
							Link = Link->Flink, Count++) {

							auto CacheEntry{ (PiDDBCache*)(Link) };

							if (!RtlCompareUnicodeString(&CacheEntry->DriverName, &IntelDriver, TRUE)) {
								CacheEntry->TimeDateStamp = 0x8394BE4 + Count;
								CacheEntry->DriverName = RTL_CONSTANT_STRING(L"OCModule.sys");
								DbgPrint("%s:", "[+] Cache Cleared!");
							}
						}
					}

				} else {

					DWORD64 EntryAddress{ 
						DWORD64(PiDDBCacheTable->BalancedRoot.RightChild) + sizeof(RTL_BALANCED_LINKS) 
					};

					auto Entry{ (PiDDBCache*)(EntryAddress) };

					if (!RtlCompareUnicodeString(&Entry->DriverName, &IntelDriver, TRUE)) {
						Entry->TimeDateStamp = 0x8394BE4;
						Entry->DriverName = RTL_CONSTANT_STRING(L"OCModule.sys");
						DbgPrint("%s:", "[+] Cache Cleared!");
					}

					DWORD Count{ 0 };

					for (auto Link{ Entry->List.Flink }; 
						Link != Entry->List.Blink; 
						Link = Link->Flink, Count++) {

						auto CacheEntry{ (PiDDBCache*)(Link) };

						if (!RtlCompareUnicodeString(&CacheEntry->DriverName, &IntelDriver, TRUE)) {
							CacheEntry->TimeDateStamp = 0x8394BE4 + Count;
							CacheEntry->DriverName = RTL_CONSTANT_STRING(L"OCModule.sys");
							DbgPrint("%s:", "[+] Cache Cleared!");
						}
					}
				}

			} else {

				DWORD64 EntryAddress{ 
					DWORD64(PiDDBCacheTable->BalancedRoot.RightChild) + sizeof(RTL_BALANCED_LINKS) 
				};

				auto Entry{ (PiDDBCache*)(EntryAddress) };

				if (!RtlCompareUnicodeString(&Entry->DriverName, &IntelDriver, TRUE)) {
					Entry->TimeDateStamp = 0x8194BE1;
					Entry->DriverName = RTL_CONSTANT_STRING(L"OCKModule.sys");
					DbgPrint("%s:", "[+] Cache Cleared!");
				}

				DWORD Count{ 0 };

				for (auto Link{ Entry->List.Flink }; 
					Link != Entry->List.Blink; 
					Link = Link->Flink, Count++) {

					auto CacheEntry{ (PiDDBCache*)(Link) };

					if (!RtlCompareUnicodeString(&CacheEntry->DriverName, &IntelDriver, TRUE)) {
						CacheEntry->TimeDateStamp = 0x8194BE1 + Count;
						CacheEntry->DriverName = RTL_CONSTANT_STRING(L"OCKModule.sys");
						DbgPrint("%s:", "[+] Cache Cleared!");
					}
				}
			}
		}
	}
}