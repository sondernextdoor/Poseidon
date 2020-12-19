#pragma once
#include "global.h"

namespace Utils {

	VOID Sleep(INT ms) {
		LARGE_INTEGER li{ 0 };
		li.QuadPart = -10000;

		for (INT i{ 0 }; i < ms; i++) {
			KeDelayExecutionThread(KernelMode, FALSE, &li);
		}
	}

	BOOLEAN ProbeUserAddress(PVOID Address, SIZE_T Size, DWORD Alignment) {
		if (Size == 0) {
			return TRUE;
		}
		
		DWORD64 Current = (DWORD64)Address;
		if (((DWORD64)Address & (Alignment - 1)) != 0) {
			return FALSE;
		}

		DWORD64 Last{ Current + Size - 1 };

		if ((Last < Current) || (Last >= MmUserProbeAddress)) {
			return FALSE;
		}

		return TRUE;
	}

	CHAR* LowerStr(CHAR* Str) {
		for (CHAR* S = Str; *S; ++S) {
			*S = (CHAR)tolower(*S);
		}
		return Str;
	}

	BOOLEAN CheckMask(CHAR* Base, CHAR* Pattern, CHAR* Mask) {
		for (; *Mask; ++Base, ++Pattern, ++Mask) {
			if (*Mask == 'x' && *Base != *Pattern) {
				return FALSE;
			}
		}

		return TRUE;
	}

	PVOID FindPattern(CHAR* Base, DWORD Length, CHAR* Pattern, CHAR* Mask) {
		Length -= (DWORD)strlen(Mask);

		for (DWORD i = 0; i <= Length; ++i) {
			PVOID Addr{ &Base[i] };

			if (CheckMask(static_cast<PCHAR>(Addr), Pattern, Mask)) {
				return Addr;
			}
		}

		return 0;
	}

	PVOID FindPatternImage(CHAR* Base, CHAR* Pattern, CHAR* Mask) {
		PVOID Match{ 0 };

		IMAGE_NT_HEADERS* Headers{ (PIMAGE_NT_HEADERS)(Base + ((PIMAGE_DOS_HEADER)Base)->e_lfanew) };
		IMAGE_SECTION_HEADER* Sections{ IMAGE_FIRST_SECTION(Headers) };

		for (DWORD i = 0; i < Headers->FileHeader.NumberOfSections; ++i) {
			IMAGE_SECTION_HEADER* Section{ &Sections[i] };

			if (*(INT*)Section->Name == 'EGAP' || memcmp(Section->Name, ".text", 5) == 0) {
				Match = FindPattern(Base + Section->VirtualAddress, Section->Misc.VirtualSize, Pattern, Mask);

				if (Match) {
					break;
				}
			}
		}

		return Match;
	}
}
