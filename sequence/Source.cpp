#include "Sequence.h"
#include "Stopwatch.h"
#include <vector>
#include <iostream>
#include <string>

#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#include <conio.h>

auto f1() {
	char a[] = { 's','e','c','r','e','t' };
}
auto f2() {
	char a[6];
	std::cout << a;
}
using namespace seq;
int main() {
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

	{
		Stopwatch clock;
		Sequence<int> test;
		for (int i = 0; i < 1000; i++) {
			test.reserve(test.capacity() + 1);
		}
		auto time = clock.MarkMicroSec();
		std::cout << time.count() << '\n';

		Sequence<int>int2 = test;

	}


	_CrtDumpMemoryLeaks();
	return 0;
}