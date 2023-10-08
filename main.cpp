#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <chrono>
#include <cstdlib>

HANDLE hProc;
std::uintptr_t procID;

void getProc(std::string_view procName) {
	hProc = { CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };
	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(procEntry);

	if (hProc == INVALID_HANDLE_VALUE) {
		std::cerr << "[ Error ] Invalid Handle Value (TH32CS_SNAPPROCESS)\n";
		return;
	}

	while (Process32Next(hProc, &procEntry)) {
		if (procEntry.szExeFile == procName) {
			CloseHandle(hProc);
			procID = { procEntry.th32ProcessID };
			hProc = { OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID) };
			std::cout << "[ Success ] Process " << '"' << procName << '"' << " Found!\n";
			return;
		}
	}
	std::cerr << "[ Error ] Failed to Find " << procName << " Process.\n";
	return;
}

std::uintptr_t getMod(std::string_view modName) {
	HANDLE hMod{ CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procID) };
	MODULEENTRY32 modEntry;
	modEntry.dwSize = sizeof(modEntry);

	if (hMod == INVALID_HANDLE_VALUE) {
		std::cerr << "[ Error ] Invalid Handle Value (TH32CS_SNAPMODULE)\n";
		return 0;
	}

	while (Module32Next(hMod, &modEntry)) {
		if (modEntry.szModule == modName) {
			CloseHandle(hMod);
			std::cout << "[ Success ] Module " << '"' << modName << '"' << " Found!\n";
			return (std::uintptr_t)modEntry.modBaseAddr;
		}
	}
	std::cerr << "[ Error ] Failed to Find " << modName << " Module.\n";
	return 0;
}


template <class cData>
cData read(std::uintptr_t dwAddress) {
	cData readen;
	ReadProcessMemory(hProc, (LPCVOID)dwAddress, &readen, sizeof(cData), NULL);
	return readen;
}

template <class cData>
void write(std::uintptr_t dwAddress, cData val) {
	WriteProcessMemory(hProc, (LPVOID)dwAddress, &val, sizeof(cData), NULL);
}

int main() {
	getProc("ac_client.exe");
	std::uintptr_t acModule{ getMod("ac_client.exe") };

	while (true) {
		while (GetAsyncKeyState(VK_SPACE)) {
			std::this_thread::sleep_for(std::chrono::microseconds(2));
			write<int>(read<std::uintptr_t>(acModule + 0x18AC00) + 0x5D, 1);
		}
	}
}