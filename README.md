# KM-UM-Communication

stealthy UM <-> KM communication system without creating any system threads, permanent hooks, driver objects, section objects or device objects.

Process:

- In our driver, we hook a function in ntoskrnl
- In usermode, we manually allocate memory and index it via custom data structures
- We then create a thread in usermode and call the hooked functions corresponding usermode-accessible function
- When the correct magic number is passed to the function, the driver will know it's us, and will then unhook and enter a shared memory loop, trapping our usermode thread in the kernel until we choose to break out of the loop

As long as this is set up prior to any anti-cheat system being active on your system, you can communicate with the driver without being detected by the various security measures employed by invasive anti-cheat technologies such as BattlEye and EasyAntiCheat. No illicit threads, hooks or objects related to communication will be detected by their current methods.

Limitations:

- Dodgy synchronization
- Not many kernel features, just basic remote-process operability
- Not designed with safety as a priority


It's meant to be manually mapped by exploiting Intel's vulnerable network adapter diagnostic driver, iqvw64e.sys

This was created for fun, I do not condone the use of this code in any program that violates the integrity of any online game. This should only be used for learning purposes or to prevent custom software from being falsely detected as a cheat.
