//============================================================================
// Name        : modbusd.cpp
// Author      : Miha.Malyshev@yandex.ru
// Version     :
// Copyright   : Michael Malyshev (Miha.Malyshev@yandex.ru)
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "Log.h"
#include "ModbusDaemon.h"
#include "Utils.h"

int main(int argc, char* argv[]) {
	int error = -1;
	LogInit("/tmp/modbusd.log");
	CModbusDaemon* pDaemon = new CModbusDaemon(std::string("modbusd"), argc, argv);
	if(pDaemon) {
		error = pDaemon->start();
		delete pDaemon;
	}
	return error;
}
