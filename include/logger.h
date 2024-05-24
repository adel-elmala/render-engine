#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


enum STATUS
{
	STATUS_INFO,
	STATUS_WARNING,
	STATUS_ERROR
};

class Loger
{
public:
	Loger();
	~Loger();
	void info(std::string message);
	void warning(std::string message);
	void error(std::string message);

private:
	void log(std::string message, STATUS log_state);
	HANDLE hConsole;
};

