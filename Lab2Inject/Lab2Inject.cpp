#include "Lab2.h"
#include "Lab2Inject.h"
#include "PipeServer.h"

BOOL Lab2_SetPrivilege(HANDLE hToken, LPCTSTR szPrivName, BOOL fEnable) {
	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	LookupPrivilegeValue(NULL, szPrivName, &tp.Privileges[0].Luid);
	tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
	AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
	return((GetLastError() == ERROR_SUCCESS));
}
HANDLE get_process_id_by_name(char* processName)
{
	HANDLE hProcess = nullptr;
	HANDLE hSnapshot = nullptr;

	PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };

	// Takes a snapshot of the all processes in the system
	// ������ ������ ���� ��������� � ������
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		std::cout << "[-] CreateToolhelp32Snapshot failed: " << GetLastError() << std::endl;
		return nullptr;
	}

	// Retrieve information about the first process
	// �������� ���������� � ������ ��������
	if (!Process32First(hSnapshot, &pe32))
	{
		std::cout << "[-] Process32First failed: " << GetLastError() << std::endl;
		CloseHandle(hSnapshot);
		return nullptr;
	}

	// Now walk the snapshot of processes
	// ������ ��������� �� ������ ���������
	do
	{
		if (std::string(pe32.szExeFile) == std::string(processName))
		{
			hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pe32.th32ProcessID);
			if (hProcess == nullptr)
			{
				std::cout << "[-] OpenProcess failed: " << GetLastError() << std::endl;
				CloseHandle(hSnapshot);
				return nullptr;
			}

			CloseHandle(hSnapshot);
			return hProcess;
		}
	} while (Process32Next(hSnapshot, &pe32));

	CloseHandle(hSnapshot);
	return nullptr;
}

HANDLE det_process_id(char* argumentSpecifier, char* processNameOrId)
{
	HANDLE hProcess = nullptr;
	HANDLE hToken = NULL;
	DWORD targetProcessId = NULL;

	HANDLE hCurrentProc = GetCurrentProcess();
	if (!OpenProcessToken(hCurrentProc, TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		printf("OpenProcessToken Error 0x%x!", GetLastError());
	}
	else {
		if (!Lab2_SetPrivilege(hToken, SE_DEBUG_NAME, TRUE)) {
			printf("Lab2_SetPrivilegeSE_DEBUG_NAME Error 0x%x!", GetLastError());
		}
	}

	if (std::string(argumentSpecifier) == "-pid")
	{
		targetProcessId = std::stoi(processNameOrId);
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProcessId);
		if (!hProcess)
		{
			std::cout << "[-] OpenProcess failed : " << GetLastError() << std::endl;
		}

	}
	else if (std::string(argumentSpecifier) == "-name")
	{
		hProcess = get_process_id_by_name(processNameOrId);
	}
	else
	{
		std::cout << "[-] Invalid argument specifier" << std::endl;
	}
	
	return hProcess;
}

void usage()
{
	printf("\n<Lab2Inject.exe> <target proc>....\n");
}

HANDLE inject_dll_to_the_process(HANDLE hProcess, const std::string& dllPath)
{
	LPVOID lpfnLoadLibraryA;
	LPVOID lpvResult;
	HANDLE hThread;
	HANDLE hToken = NULL;

	lpfnLoadLibraryA = GetProcAddress(GetModuleHandle("C:\\Windows\\System32\\kernel32.dll"), "LoadLibraryA");
	if (!lpfnLoadLibraryA)
	{
		std::cout << "[-] GetProcAddress failed : " << GetLastError() << std::endl;
		return NULL;
	}

	lpvResult = VirtualAllocEx(hProcess, nullptr, dllPath.size(), MEM_COMMIT, PAGE_READWRITE);
	if (!lpvResult) {
		std::cout << "[-] VirtualAllocEx failed : " << GetLastError() << std::endl;
		return NULL;
	}

	if (!WriteProcessMemory(hProcess, lpvResult, dllPath.c_str(), dllPath.size(), nullptr))
	{
		std::cout << "[-] WriteProcessMemory failed : " << GetLastError() << std::endl;
		return NULL;
	}

	hThread = CreateRemoteThread(hProcess, nullptr, 0,
		(LPTHREAD_START_ROUTINE)lpfnLoadLibraryA, lpvResult, 0, nullptr);
	if (!hThread)
	{
		std::cout << "[-] CreateRemoteThread failed : " << GetLastError() << std::endl;
		return NULL;
	}

	return hThread;
}

int main(int argc, char* argv[]) 
{
	setlocale(LC_ALL, "RUS");
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	HANDLE remote_thread_h = NULL;
	HANDLE hProcess;
	char currentDirectory[MAX_PATH] = { 0 };
	const string pipename = PIPE_NAME;
	const string task(std::string(argv[3]) + std::string(" ") + std::string(argv[4]));

	GetCurrentDirectory(sizeof(currentDirectory), currentDirectory);

	string full_dll_path(currentDirectory);
	full_dll_path.append(NAME_DLL);
	if (argc < 1)
	{
		usage();
		return __LINE__;
	}
	string message;
	int result = 0;

	hProcess = det_process_id(argv[1], argv[2]);
	if (!hProcess)
	{
		cout << "ERROR: Process handle has not been got" << endl;
		return -1;
	}

	cout << "ok: Process handle has been got" << endl;
	
	PipeServer pipe(pipename);
	try
	{
		pipe.createNamedPipe();
	}
	catch (std::string & error)
	{
		cout << error << endl;
		return -1;
	}

	cout << "ok: Pipe has been created" << endl;
	
	remote_thread_h = inject_dll_to_the_process(hProcess, full_dll_path);
	if (remote_thread_h == NULL)
	{
		LAB2_PRINT("Unable to inject DLL to process!");
		cout << "ERROR: Unable to inject DLL to process!" << endl;
		return -1;
	}

	cout << "DLL " << full_dll_path << " has been injected to "<< argv[2] << endl;

	cout << "Waiting for a pipe client... " << endl;

	pipe.waitForClient();

	cout << "ok: Pipe client has been connected" << endl;

	try
	{
		pipe.sendMessage(task);
	}
	catch (std::string & error)
	{
		cout << error << endl;
		return -1;
	}

	cout << "ok: Task to client has been sent" << endl;

	if (string("-func") == argv[3])
	{
		cout << "Waiting messages from client... " << endl;

		while (true)
		{
			try
			{
				result = pipe.receiveMessage(message);
			}
			catch (string & error)
			{
				cout << error << endl;
				return -1;
			}

			if (result)
			{
				cout << message << endl;
				message.clear();
			}

			Sleep(100);
		}
	}

	cout << "ok: Injector has finished" << endl;

	return 0;
}
