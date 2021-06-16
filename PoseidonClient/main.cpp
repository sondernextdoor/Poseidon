#include "sdk.h"
#include <iomanip>

int main() {
	Client::Connect();


	// Manually calling the functions in process.h and memory.h

	DWORD ProcessId{ Process::GetProcessId(L"notepad.exe") };
	PVOID BaseAddress{ Process::GetBase(ProcessId) };
	int ExampleValue{ Memory::Read<int>(ProcessId, BaseAddress) };

	std::cout << "0x" << std::hex << BaseAddress << std::endl;
	std::cout << std::dec << ExampleValue << std::endl;

	getchar();


	// Or using a KProcess object

	auto Notepad{ KProcess(L"notepad.exe") };
	int ExampleValue2{ Notepad.Read<int>(Notepad.BaseAddress) };

	std::cout << "0x" << std::hex << Notepad.BaseAddress << std::endl;
	std::cout << std::dec << ExampleValue2 << std::endl;

	getchar();


	Client::Disconnect(); // Once this is called, we can never reobtain a connection to the driver without remapping it
}