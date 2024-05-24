#include <iostream>

#include "../include/logger.h"

Loger::Loger()
{
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

Loger::~Loger()
{
}

void Loger::info(std::string message)
{

}
void Loger::warning(std::string message)
{

}
void Loger::error(std::string message)
{

}

void Loger::log(std::string message, STATUS log_state)
{
	switch (log_state)
	{
	case STATUS_INFO:
		SetConsoleTextAttribute(hConsole, 2);
		std::cout << "[INFO]: " << message << std::endl;
		break;
	case STATUS_WARNING:
		break;
	case STATUS_ERROR:
		break;
	default:
		break;
	}
}
