#pragma once

//shared
#include "Lab2.h"

#include <Psapi.h>
#include <TlHelp32.h>
#define NAME_DLL  "\\Lab2Hook.dll"

#define LAB2_PRINT(a,...) {printf("\nLAB2 INJECT: ", a, ##__VA_ARGS__); fflush(stdout);}

using namespace std;
