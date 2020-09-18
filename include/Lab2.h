#pragma once
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

#define MAX_PRINT_TEXT_LENGHT 1024
#define PIPE_NAME "\\\\.\\pipe\\lab2namedpipe"
#define DLL_TO_HOOK TEXT("kernel32.dll")

void Lab2DebugOut(const char* fmt, ...);
