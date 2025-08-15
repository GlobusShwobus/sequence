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

	class asstangle {
		int x;
		int y;
		int asswidth;
		int assheight;
	public:
		asstangle() = default;
		asstangle(int a, int b, int c, int l) :x(a), y(b), asswidth(c), assheight(l) {}
	};

	{
		Stopwatch clock;
		std::vector<asstangle> vec;
		for (int i = 0; i < 10000; i++) {
			asstangle ass(i, i, i, i);
			vec.push_back(ass);
		}
		auto time = clock.MarkMicroSec();
		std::cout << "vec 10k push_back with copy: " << time.count() << " microsec\n";
	}
	{
		Stopwatch clock;
		Sequence<asstangle> seq;
		for (int i = 0; i < 10000; i++) {
			asstangle ass(i, i, i, i);
			seq.push_back(ass);
		}
		auto time = clock.MarkMicroSec();
		std::cout << "seq 10k push_back with copy: " << time.count() << " microsec\n";
	}
	{
		Stopwatch clock;
		std::vector<asstangle> vec;
		for (int i = 0; i < 10000; i++) {
			asstangle ass(i, i, i, i);
			vec.push_back(std::move(ass));
		}
		auto time = clock.MarkMicroSec();
		std::cout << "vec 10k push_back with move: " << time.count() << " microsec\n";
	}
	{
		Stopwatch clock;
		Sequence<asstangle> seq;
		for (int i = 0; i < 10000; i++) {
			asstangle ass(i, i, i, i);
			seq.push_back(std::move(ass));
		}
		auto time = clock.MarkMicroSec();
		std::cout << "seq 10k push_back with move: " << time.count() << " microsec\n";
	}


	_CrtDumpMemoryLeaks();
	return 0;

}