#include <iostream>
#include <windows.h>
#include <string>
#include "Lab2Hook.h"

string  fullpath, path, filename;
wstring wfullpath, wpath, wfilename;
bool isPathToHiddenFile = false;

HANDLE(WINAPI* realFindFirstFileExA)(LPCSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData,
	FINDEX_SEARCH_OPS  fSearchOp, LPVOID  lpSearchFilter, DWORD dwAdditionalFlags) = FindFirstFileExA;

HANDLE(WINAPI *realFindFirstFileExW)(LPCWSTR  lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID  lpFindFileData,
	FINDEX_SEARCH_OPS  fSearchOp, LPVOID lpSearchFilter, DWORD   dwAdditionalFlags)= FindFirstFileExW;

HANDLE (WINAPI *realCreateFileA)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)= CreateFileA;
	
HANDLE (WINAPI *realCreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)=CreateFileW;

HANDLE (WINAPI *realFindFirstFileA)(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)= FindFirstFileA;

HANDLE(WINAPI *realFindFirstFileW)(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData)= FindFirstFileW;

BOOL (WINAPI *realFindNextFileA)(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)= FindNextFileA;

BOOL (WINAPI *realFindNextFileW)(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData)= FindNextFileW;


string get_path_from_fullpath(std::string&& fullpath_)
{
	size_t backslashPosition = fullpath_.rfind('\\');
	string path = fullpath_.substr(0, backslashPosition + 1);
	return path;
}

wstring wget_path_from_fullpath(wstring&& wfullpath_)
{
	size_t backslashPosition = wfullpath_.rfind('\\');
	wstring wpath = wfullpath_.substr(0, backslashPosition + 1);
	return wpath;
}

wstring find_wfilename(wstring local_wfullpath)
{
	size_t backslashPosition = local_wfullpath.rfind('\\');
	wstring local_wfilename = local_wfullpath.substr(backslashPosition + 1, local_wfullpath.length());
	return local_wfilename;
}

string find_filename(string local_fullpath)
{
	size_t backslashPosition = local_fullpath.rfind('\\');
	string local_filename = local_fullpath.substr(backslashPosition + 1, local_fullpath.length());
	return local_filename;
}


#pragma region

__declspec(dllexport) HANDLE WINAPI Hook_FindFirstFileExA(
	LPCSTR             lpFileName,
	FINDEX_INFO_LEVELS fInfoLevelId,
	LPVOID             lpFindFileData,
	FINDEX_SEARCH_OPS  fSearchOp,
	LPVOID             lpSearchFilter,
	DWORD              dwAdditionalFlags
)
{
	string local_subfilename;
	LAB2_PRINT("FindFirstFileExW " + string(lpFileName));

	string currentDirectoryPath = get_path_from_fullpath(string(lpFileName));
	string local_filename = find_filename(string(lpFileName));

	size_t zPosition = local_filename.rfind('*');
	LAB2_PRINT("Find * " + to_string(zPosition));
	if (zPosition != string::npos)
	{
		if (zPosition + 1 != local_filename.length())
		{
			string local_endsubfilename = local_filename.substr(zPosition + 1, local_filename.length());
			LAB2_PRINT("Find endsub: " + local_endsubfilename);
			local_subfilename = local_filename.substr(0, zPosition);
			LAB2_PRINT("Find sub: " + local_subfilename);
			if (filename.find(local_subfilename) != string::npos && filename.find(local_endsubfilename) != string::npos)
			{
				LAB2_PRINT("INVALID: " + local_subfilename + local_endsubfilename);
				return INVALID_HANDLE_VALUE;
			}

		}
		else
		{
			local_subfilename = local_filename.substr(0, zPosition);
			LAB2_PRINT("Find sub: " + local_subfilename);
			if (filename.find(local_subfilename) != string::npos)
			{
				LAB2_PRINT("INVALID: " + local_subfilename);
				return INVALID_HANDLE_VALUE;
			}
		}
	}
	else if (zPosition == 0)
	{
		local_subfilename = local_filename.substr(zPosition + 1, local_filename.length());
		LAB2_PRINT("Find sub: " + local_subfilename);
		if (filename.find(local_subfilename) != string::npos)
		{
			LAB2_PRINT("INVALID: " + local_subfilename);
			return INVALID_HANDLE_VALUE;
		}
	}

	if ( filename == local_filename)
	{
		return INVALID_HANDLE_VALUE;
	}

	return realFindFirstFileExA(
		lpFileName,
		fInfoLevelId,
		lpFindFileData,
		fSearchOp,
		lpSearchFilter,
		dwAdditionalFlags
	);
}


__declspec(dllexport) HANDLE WINAPI Hook_FindFirstFileExW(
	LPCWSTR            lpFileName,
	FINDEX_INFO_LEVELS fInfoLevelId,
	LPVOID             lpFindFileData,
	FINDEX_SEARCH_OPS  fSearchOp,
	LPVOID             lpSearchFilter,
	DWORD              dwAdditionalFlags
)
{
	wstring local_subfilename;
	LAB2_PRINT(L"FindFirstFileExW " + wstring(lpFileName));

	wstring currentDirectoryPath = wget_path_from_fullpath(wstring(lpFileName));
	wstring local_wfilename = find_wfilename(wstring(lpFileName));

	size_t zPosition = local_wfilename.rfind('*');
	LAB2_PRINT(L"Find * " + to_wstring(zPosition));
	if (zPosition != wstring::npos)
	{
		if (zPosition + 1 != local_wfilename.length())
		{
			wstring local_endsubfilename = local_wfilename.substr(zPosition + 1, local_wfilename.length());
			LAB2_PRINT(L"Find endsub: " + local_endsubfilename);
			local_subfilename = local_wfilename.substr(0, zPosition);
			LAB2_PRINT(L"Find sub: " + local_subfilename);
			if ( wfilename.find(local_subfilename) != wstring::npos && wfilename.find(local_endsubfilename) != wstring::npos)
			{
				LAB2_PRINT(L"INVALID: " + local_subfilename+ local_endsubfilename);
				return INVALID_HANDLE_VALUE;
			}

		}
		else
		{
			local_subfilename = local_wfilename.substr(0, zPosition);
			LAB2_PRINT(L"Find sub: " + local_subfilename);
			if (wfilename.find(local_subfilename) != wstring::npos)
			{
				LAB2_PRINT(L"INVALID: " + local_subfilename);
				return INVALID_HANDLE_VALUE;
			}
		}
	}
	else if (zPosition == 0)
	{
		local_subfilename = local_wfilename.substr(zPosition+1, local_wfilename.length());
		LAB2_PRINT(L"Find sub: " + local_subfilename);
		if (wfilename.find(local_subfilename) != wstring::npos)
		{
			LAB2_PRINT(L"INVALID: " + local_subfilename);
			return INVALID_HANDLE_VALUE;
		}
	}
	

	if (wfilename == local_wfilename)
	{
		return INVALID_HANDLE_VALUE;
	}

	return realFindFirstFileExW(
		lpFileName,
		fInfoLevelId,
		lpFindFileData,
		fSearchOp,
		lpSearchFilter,
		dwAdditionalFlags
	);
}


__declspec(dllexport) HANDLE WINAPI Hook_CreateFileA(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
)
{
	LAB2_PRINT("CreateFileA " + string(lpFileName));

	if (fullpath == string(lpFileName))
	{
		return INVALID_HANDLE_VALUE;
	}

	return realCreateFileA(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);
}


__declspec(dllexport) HANDLE WINAPI Hook_CreateFileW(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
)
{
	LAB2_PRINT(L"CreateFileW " + wstring(lpFileName));

	if (wfullpath == wstring(lpFileName))
	{
		return INVALID_HANDLE_VALUE;
	}

	return realCreateFileW(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);
}


__declspec(dllexport) HANDLE WINAPI Hook_FindFirstFileA(
	LPCSTR lpFileName,
	LPWIN32_FIND_DATAA lpFindFileData
)
{
	LAB2_PRINT("FindFirstFileA " + string(lpFileName));
	string local_subfilename;
	string currentDirectoryPath = get_path_from_fullpath(string(lpFileName));
	string local_filename = find_filename(string(lpFileName));

	size_t zPosition = local_filename.rfind('*');
	LAB2_PRINT("Find * " + to_string(zPosition));
	if (zPosition != string::npos)
	{
		if (zPosition + 1 != local_filename.length())
		{
			string local_endsubfilename = local_filename.substr(zPosition + 1, local_filename.length());
			LAB2_PRINT("Find endsub: " + local_endsubfilename);
			local_subfilename = local_filename.substr(0, zPosition);
			LAB2_PRINT("Find sub: " + local_subfilename);
			if (filename.find(local_subfilename) != string::npos && filename.find(local_endsubfilename) != string::npos)
			{
				LAB2_PRINT("INVALID: " + local_subfilename + local_endsubfilename);
				return INVALID_HANDLE_VALUE;
			}

		}
		else
		{
			local_subfilename = local_filename.substr(0, zPosition);
			LAB2_PRINT("Find sub: " + local_subfilename);
			if (filename.find(local_subfilename) != string::npos)
			{
				LAB2_PRINT("INVALID: " + local_subfilename);
				return INVALID_HANDLE_VALUE;
			}
		}
	}
	else if (zPosition == 0)
	{
		local_subfilename = local_filename.substr(zPosition + 1, local_filename.length());
		LAB2_PRINT("Find sub: " + local_subfilename);
		if (filename.find(local_subfilename) != string::npos)
		{
			LAB2_PRINT("INVALID: " + local_subfilename);
			return INVALID_HANDLE_VALUE;
		}
	}

	if (filename == local_filename)
	{
		return INVALID_HANDLE_VALUE;
	}
	return realFindFirstFileA(lpFileName, lpFindFileData);
}


__declspec(dllexport) HANDLE WINAPI Hook_FindFirstFileW(
	LPCWSTR lpFileName,
	LPWIN32_FIND_DATAW lpFindFileData
)
{
	LAB2_PRINT(L"FindFirstFileW " + wstring(lpFileName));
	wstring local_subfilename;

	wstring currentDirectoryPath = wget_path_from_fullpath(wstring(lpFileName));
	wstring local_wfilename = find_wfilename(wstring(lpFileName));

	size_t zPosition = local_wfilename.rfind('*');
	LAB2_PRINT(L"Find * " + to_wstring(zPosition));
	if (zPosition != wstring::npos)
	{
		if (zPosition + 1 != local_wfilename.length())
		{
			wstring local_endsubfilename = local_wfilename.substr(zPosition + 1, local_wfilename.length());
			LAB2_PRINT(L"Find endsub: " + local_endsubfilename);
			local_subfilename = local_wfilename.substr(0, zPosition);
			LAB2_PRINT(L"Find sub: " + local_subfilename);
			if (wfilename.find(local_subfilename) != wstring::npos && wfilename.find(local_endsubfilename) != wstring::npos)
			{
				LAB2_PRINT(L"INVALID: " + local_subfilename + local_endsubfilename);
				return INVALID_HANDLE_VALUE;
			}

		}
		else
		{
			local_subfilename = local_wfilename.substr(0, zPosition);
			LAB2_PRINT(L"Find sub: " + local_subfilename);
			if (wfilename.find(local_subfilename) != wstring::npos)
			{
				LAB2_PRINT(L"INVALID: " + local_subfilename);
				return INVALID_HANDLE_VALUE;
			}
		}
	}
	else if (zPosition == 0)
	{
		local_subfilename = local_wfilename.substr(zPosition + 1, local_wfilename.length());
		LAB2_PRINT(L"Find sub: " + local_subfilename);
		if (wfilename.find(local_subfilename) != wstring::npos)
		{
			LAB2_PRINT(L"INVALID: " + local_subfilename);
			return INVALID_HANDLE_VALUE;
		}
	}


	if (wfilename == local_wfilename)
	{
		return INVALID_HANDLE_VALUE;
	}
	return realFindFirstFileW(lpFileName, lpFindFileData);
}


__declspec(dllexport) BOOL WINAPI Hook_FindNextFileA(
	HANDLE hFindFile,
	LPWIN32_FIND_DATAA lpFindFileData
)
{
	LAB2_PRINT("FindNextFileA " + string(lpFindFileData->cFileName));

	return realFindNextFileA(hFindFile, lpFindFileData);
}


__declspec(dllexport) BOOL WINAPI Hook_FindNextFileW(
	HANDLE hFindFile,
	LPWIN32_FIND_DATAW lpFindFileData
)
{
	LAB2_PRINT(L"FindNextFileW : " + wstring(lpFindFileData->cFileName));

	return realFindNextFileW(hFindFile, lpFindFileData);
}

#pragma endregion

void setPathsToFile(string& fileName_)
{
	size_t backslashPosition = fileName_.rfind('\\');

	fullpath = fileName_;
	path = fullpath.substr(0, backslashPosition + 1);
	filename = fullpath.substr(backslashPosition + 1, fullpath.length());

	wfullpath = wstring(fullpath.begin(), fullpath.end());
	wpath = wstring(path.begin(), path.end());
	wfilename = wstring(filename.begin(), filename.end());
}


int hideFile(string& fileName)
{
	int i;
	const string funcNamesToReplace[] =
	{
		"CreateFileA",     "CreateFileW",
		"FindFirstFileA",   "FindFirstFileW",
		"FindFirstFileExA", "FindFirstFileExW",
		"FindNextFileA",    "FindNextFileW"
	};

	setPathsToFile(fileName);
	LONG err = NULL;

	i = 0;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)realCreateFileA, Hook_CreateFileA);
	err = DetourTransactionCommit();
	if (err == NO_ERROR)
	{
		LAB2_PRINT("Detoured successfully" + funcNamesToReplace[i]);
	}
	else
	{
		LAB2_PRINT("ERRR: detoured failed" + std::to_string(err) + funcNamesToReplace[i]);
		return -1;
	}
	err = NULL;

	i = 2;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)realFindFirstFileA, Hook_FindFirstFileA);
	err = DetourTransactionCommit();
	if (err == NO_ERROR)
	{
		LAB2_PRINT("Detoured successfully" + funcNamesToReplace[i]);
	}
	else
	{
		LAB2_PRINT("ERRR: detoured failed" + std::to_string(err) + funcNamesToReplace[i]);
		return -1;
	}
	err = NULL;

	i = 3;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)realFindFirstFileW, Hook_FindFirstFileW);
	err = DetourTransactionCommit();
	if (err == NO_ERROR)
	{
		LAB2_PRINT("Detoured successfully" + funcNamesToReplace[i]);
	}
	else
	{
		LAB2_PRINT("ERRR: detoured failed" + to_string(err) + funcNamesToReplace[i]);
		return -1;
	}
	err = NULL;

	i = 4;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)realFindFirstFileExA, Hook_FindFirstFileExA);
	err = DetourTransactionCommit();
	if (err == NO_ERROR)
	{
		LAB2_PRINT("Detoured successfully" + funcNamesToReplace[i]);
	}
	else
	{
		LAB2_PRINT("ERRR: detoured failed" + std::to_string(err) + funcNamesToReplace[i]);
		return -1;
	}
	err = NULL;

	i = 5;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)realFindFirstFileExW, Hook_FindFirstFileExW);
	err = DetourTransactionCommit();
	if (err == NO_ERROR)
	{
		LAB2_PRINT("Detoured successfully" + funcNamesToReplace[i]);
	}
	else
	{
		LAB2_PRINT("ERRR: detoured failed" + std::to_string(err) + funcNamesToReplace[i]);
		return -1;
	}
	err = NULL;

	i = 6;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)realFindNextFileA, Hook_FindNextFileA);
	err = DetourTransactionCommit();
	if (err == NO_ERROR)
	{
		LAB2_PRINT("Detoured successfully" + funcNamesToReplace[i]);
	}
	else
	{
		LAB2_PRINT("ERRR: detoured failed" + std::to_string(err) + funcNamesToReplace[i]);
		return -1;
	}
	err = NULL;

	i = 7;
	LAB2_PRINT("[+] Address of " + funcNamesToReplace[i] + " has been got");
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)realFindNextFileW, Hook_FindNextFileW);
	err = DetourTransactionCommit();
	if (err == NO_ERROR)
	{
		LAB2_PRINT("Detoured successfully" + funcNamesToReplace[i]);
	}
	else
	{
		LAB2_PRINT("ERRR: detoured failed" + std::to_string(err) + funcNamesToReplace[i]);
		return -1;
	}
	err = NULL;

	return 0;
}
