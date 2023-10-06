#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <chrono>

HANDLE hProc;
std::uintptr_t procID;

void getProc(std::string_view procName) {
	hProc = { CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };
	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(procEntry);

	if (hProc == INVALID_HANDLE_VALUE) {
		std::cerr << "Error: Invalid Handle Value (TH32CS_SNAPPROCESS)\n";
		return;
	}

	while (Process32Next(hProc, &procEntry)) {
		if (procEntry.szExeFile == procName) {
			procID = procEntry.th32ProcessID;
			CloseHandle(hProc);
			hProc = { OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID) };
			std::cout << "Process " << procName << " Found.\n";
			return;
		}
	}
	std::cerr << "Error: Failed to Search For " << procName << " Process.\n";
	return;
}

std::uintptr_t getMod(std::string_view modName) {
	HANDLE hMod{ CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procID) };
	MODULEENTRY32 modEntry;
	modEntry.dwSize = sizeof(modEntry);

	if (hMod == INVALID_HANDLE_VALUE) {
		std::cerr << "Error: Invalid Handle Value (TH32CS_SNAPMODULE)\n";
		return 0;
	}

	while (Module32Next(hMod, &modEntry)) {
		if (modEntry.szModule == modName) {
			CloseHandle(hMod);
			std::cout << "Module " << modName << " Found.\n";
			return (std::uintptr_t)modEntry.modBaseAddr;
		}
	}
	std::cerr << "Error: Failed to Search For " << modName << " Module.\n";
	return 0;
}

template <class cData>
cData read(std::uintptr_t dwAddress) {
	cData readVal;
	ReadProcessMemory(hProc, (LPCVOID)dwAddress, &readVal, sizeof(cData), NULL);
	return readVal;
}

template <class cData>
void write(cData dwAddress, cData value) {
	WriteProcessMemory(hProc, (LPVOID)dwAddress, &value, sizeof(cData), NULL);
}

int main() {
	getProc("ac_client.exe");
	std::uintptr_t clientModule{ getMod("ac_client.exe") };

	std::uintptr_t i_canJump{ clientModule + 0x18AC5D };

	while (true) {
		while (true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			write<int>(read<std::uintptr_t>(clientModule + 0x18AC00) + 0x5D, 1);
		}
	}
}