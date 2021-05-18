# KM-UM-Communication

stealthy UM <-> KM communication system without creating any system threads, permanent hooks, driver objects, section objects or device objects.

Process:

- In our driver, we hook a function in ntoskrnl
- In usermode, we manually allocate memory and index it via custom data structures
- We then create a thread in usermode and call the hooked functions corresponding usermode-accessible function
- When the correct magic number is passed to the function, the driver will know it's us, and will then unhook and enter a shared memory loop, trapping our usermode thread in the kernel until we choose to break out of the loop

As long as this is set up prior to any anti-cheat being active on your system, you can communicate with the driver without being detected by the various security measures employed by invasive anti-cheat technologies such as BattlEye and EasyAntiCheat. No illicit threads, hooks or objects related to communication will be detected by their current methods.

Limitations:

- Dodgy synchronization
- Not many kernel features, just basic remote-process operability
- Not designed with safety as a priority
- Only tested on Windows 10 1909


It's meant to be manually mapped by exploiting Intel's vulnerable network adapter diagnostic driver, iqvw64e.sys

This was created for fun, I do not condone the use of this code in any program that violates the integrity of any online game. This should only be used for learning purposes or to prevent custom software from being falsely detected as a cheat.

Usage:

- Map the driver
- Start the client
- Start the target process
- Do stuff

You have to modify the client to sleep until your target process is running (since it must be set up prior to any anti-cheat being active). Basic example of how main.cpp in the client should typically look:

```
int main() {
	Client::Connect();

	for (;;) {
		Sleep(100);

		if (YourTargetProcessIsRunning) {
			break;
		}
	}

	// Do stuff
  
        Client::Disconnect();
  }
  ```
  
You can either call the functions in memory.h and process.h manually, or you can just create a KProcess object for easier use. KProcess features are as follows:

```
        // Read Memory

	int Value = Notepad.Read<int>((PVOID)0xDEADBEEF);
	Notepad.Read((PVOID)0xDEADBEEF, &Value, sizeof(int)); // Overload


	// Write Memory

	Notepad.Write<int>((PVOID)0xDEADBEEF, 2);
	Notepad.Write((PVOID)0xDEADBEEF, &Value, sizeof(int)); // Overload


	// Allocate Virtual Memory

	Notepad.AllocateVirtualMemory(PVOID Base, SIZE_T Size, DWORD AllocType, DWORD Protect);


	// Free Virtual Memory

	Notepad.FreeVirtualMemory(PVOID Base, SIZE_T Size, DWORD FreeType);


	// Change Virtual Memory Protection

	Notepad.ProtectVirtualMemory(PVOID Base, SIZE_T Size, DWORD Protect, DWORD* OldProtect);


	// Query Virtual Memory. MEMORY_BASIC_INFORMATION only.

	MEMORY_BASIC_INFORMATION MBI{ 0 };

	bool bResult = Notepad.QueryVirtualMemory(PVOID Address, MEMORY_BASIC_INFORMATION& MemoryBasicInfo, SIZE_T Size);
	MBI = Notepad.QueryVirtualMemory(PVOID Address, SIZE_T Size); // Overload


	// Query Process Information

	Notepad.QueryInformationProcess();


	// Get module info by name

	Notepad.GetModuleInfo(const char* ModuleName, DWORD& ModuleSize);


	// Pattern finder

	Notepad.PatternFinder(BYTE* Start, DWORD Size, const char* Signature, const char* Mask);


	// Get absolute address of specified asm instruction

	Notepad.AbsoluteAddress(BYTE* Rip, DWORD InstructionLength);


	// Get relative address of specified asm instruction

	Notepad.RelativeAddress(BYTE* DestinationAddress, BYTE* SourceAddress, DWORD InstructionLength);


	Notepad.BaseAddress;     // Base Address
	Notepad.ImageName;	 // Name
	Notepad.ModuleCount;     // Number of modules
	Notepad.ModuleList;      // std::vector containing all modules' base address and size
	Notepad.Peb;		 // Process Environment Block
	Notepad.ProcessId;	 // Process Id
	Notepad.Size;		 // Main module size
  ```
