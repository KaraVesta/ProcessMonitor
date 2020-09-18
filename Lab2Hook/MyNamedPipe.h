#pragma once

#include <iostream>
#include <Windows.h>

using namespace std;

class MyNamedPipe
{

private:
	HANDLE hPipe;
	string pipename;

public:
	explicit MyNamedPipe(const string&);
	~MyNamedPipe();

public:
	void openNamedPipe();
	void sendMessage(const string&);
	int receiveMessage(string&);

};
