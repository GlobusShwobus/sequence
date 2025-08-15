#include "Sequence.h"
#include "Stopwatch.h"
#include <vector>
#include <iostream>
#include <string>

#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#include <conio.h>


using namespace seq;
int main() {
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);


	constexpr double constDouble = 5.2;
	std::cout << "address: " << &constDouble << " value " << constDouble << '\n';
	const double* constDoubleRuntime = &constDouble;
	std::cout << "address: "<< constDoubleRuntime << " value "<<*constDoubleRuntime <<'\n';
	double* whatAmINow = const_cast<double*>(constDoubleRuntime);
	std::cout << "address: " << whatAmINow << " value " << *whatAmINow << '\n';
	*whatAmINow = 420.0;
	std::cout << "address: " << &constDouble << " value " << constDouble << '\n';
	std::cout << "address: " << constDoubleRuntime << " value " << *constDoubleRuntime << '\n';
	std::cout << "address: " << whatAmINow << " value " << *whatAmINow << '\n';




	_CrtDumpMemoryLeaks();
	return 0;
}