#pragma once
#include "detours.h"
//shared
#include "Lab2.h"

#define LAB2_PRINT(a,...) Lab2DebugOut("\nLAB2 HOOK: ", a, ##__VA_ARGS__)
using namespace std;
int hideFile(string&);