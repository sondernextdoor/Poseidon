#include "sdk.h"

NTSTATUS DriverEntry(DRIVER_OBJECT* DriverObject, UNICODE_STRING* RegistryPath) {
	return Driver::Initialize();
}