#include "Lab2.h"
#include "Lab2Hook.h"
#include "MyNamedPipe.h"

void Lab2DebugOut(const char* fmt, ...) 
{
	va_list args;
	char s[MAX_PRINT_TEXT_LENGHT];
	va_start(args, fmt);
	vsnprintf(s, MAX_PRINT_TEXT_LENGHT, fmt, args);
	OutputDebugStringA(s);
	va_end(args);
}

string gfunction = {0};
MyNamedPipe* gpipe;
BOOLEAN gLab2Hooked = FALSE;

extern "C" LPVOID gLab2OrigPointer = NULL;
extern "C" void Lab3Hook();
extern "C" VOID Lab2HookCallback()
{
	::gpipe->sendMessage("CALLED " + gfunction);
}
int monitor(string &function, MyNamedPipe *pipe)
{
	::gpipe = pipe;
	::gfunction = function;
	if (gLab2Hooked == FALSE || gLab2OrigPointer == NULL)
	{
		gLab2OrigPointer = GetProcAddress(GetModuleHandle("kernel32.dll"), function.c_str());
		if (gLab2OrigPointer == nullptr)
		{
			LAB2_PRINT("ERROR: GetProcAddress : " + to_string(GetLastError()));
			return -1;
		}
		gLab2Hooked = TRUE;
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)gLab2OrigPointer, Lab3Hook);
		LONG error = DetourTransactionCommit();
		if (error == NO_ERROR)
		{
			LAB2_PRINT("Detoured successfully" + function);
		}
		else
		{
			LAB2_PRINT("ERRR detoured failed" + std::to_string(error) + function);
			return -1;
		}
	}
	return 0;
}

BOOL main_function()
{
	int result = 0;
	std::string task;
	const std::string pipename = PIPE_NAME;

	MyNamedPipe *pipe = new MyNamedPipe(pipename);

	// Opening the existing pipe and receiving task from injector
	// Открытие существующего pipe'a и получение задания от инъектора
	try
	{
		pipe->openNamedPipe();

		LAB2_PRINT("ok: Pipe has been opened");

		while (true)
		{
			result = pipe->receiveMessage(task);
			if (result)
			{
				break;
			}

			LAB2_PRINT("ERROR: Task has not been received. Waiting...");

			Sleep(50);
		}
	}
	catch (string error)
	{
		LAB2_PRINT(move(error));

		return FALSE;
	}

	LAB2_PRINT("ok: Task from server has been received: ");

	string command = task.substr(0, 5);
	if (command == "-func")
	{
		string function = task.substr(6, task.length());
		monitor(function, pipe);
	}
	else if (command == "-hide")
	{
		string fileName = task.substr(6, task.length());
		hideFile(fileName);

		delete pipe;
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}


//DllMain
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDLL);
		if (!main_function())
		{
			return FALSE;
		}
	/*case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;*/
	case DLL_PROCESS_DETACH:
		LAB2_PRINT("Process detach...");
		
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}