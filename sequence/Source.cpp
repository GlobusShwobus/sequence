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

	{
		std::string myMeme = "my memes are bad as bad as my cooking";
		Stopwatch clock;
		std::vector<std::string> vec;
		for (int i = 0; i < 10000; i++) {
			vec.push_back(myMeme);
		}
		auto time = clock.MarkMicroSec();
		std::cout << "vec 10k pushes, always triggers realloc??? don't know: " << time.count() << " microsec\n";
	}
	{
		std::string myMeme = "my memes are bad as bad as my cooking";
		Stopwatch clock;
		Sequence<std::string> seq;
		for (int i = 0; i < 10000; i++) {
			seq.push_back(myMeme);
		}
		auto time = clock.MarkMicroSec();
		std::cout << "seq 10k pushes, always triggers realloc: " << time.count() << " microsec\n";
	}

	_CrtDumpMemoryLeaks();
	return 0;

}