# KM-UM-Communication

stealthy UM <-> KM communication system without creating any system threads, permanent hooks, driver objects, section objects or device objects.

Process:

- Hook a function in ntoskrnl
- Create a thread and call that function in user-mode, trapping the thread in kernel-space
- Unhook in hooked function and enter a shared memory loop
- Shared memory is manually implemented. Memory is allocated in the user-mode process and indexed via custom data structures

Limitations:

- Dodgy synchronization
- Not many kernel features, just basic remote-process operability
- Not designed with safety as a priority


It's meant to be manually mapped by exploiting Intel's vulernable network adapter diagnostic driver, iqvw64e.sys
