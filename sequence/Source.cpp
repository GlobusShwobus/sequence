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
			vec.push_back({ i + i,i + i + i,i + i + i + i,i + i + i + i + i });
		}
		auto time = clock.MarkMicroSec();
		std::cout << "vec 10k empalcebacks, always triggers realloc??? don't know: " << time.count() << " microsec\n";
	}
	{
		Stopwatch clock;
		Sequence<asstangle> seq;
		for (int i = 0; i < 10000; i++) {
			seq.push_back({i + i, i + i + i, i + i + i + i, i + i + i + i + i});
		}
		auto time = clock.MarkMicroSec();
		std::cout << "seq 10k resize, always triggers realloc: " << time.count() << " microsec\n";
	}
	{
		Sequence<asstangle> asses(69);
		asses.pop_back();
		asses.pop_back();
		asses.pop_back();
		asses.pop_back();
	}
	{
		Sequence<asstangle> asses2;
		auto it = asses2.begin();
		if (it.base() == nullptr) {
			std::cout << "i am stupid\n";
		}
		it++;

		//std::vector<asstangle> asses2;
		//auto it = asses2.end();
		//if (it._Ptr == nullptr) {
		//	std::cout << "i am stupid\n";
		//}
		//it++;

	}
	
	_CrtDumpMemoryLeaks();
	return 0;

}