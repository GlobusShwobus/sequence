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
		for (size_t i = 0; i < 10000; i++) {
			vec.resize(i);
		}
		auto time = clock.MarkMicroSec();
		std::cout << "vec 10k resize, always triggers realloc??? don't know: " << time.count() << " microsec\n";
		std::cout << "vec size and cap" << vec.size() << "   " << vec.capacity() << '\n';
	}
	{
		std::string myMeme = "my memes are bad as bad as my cooking";
		Stopwatch clock;
		Sequence<std::string> seq;
		for (size_t i = 0; i < 10000; i++) {
			seq.resize(i);
		}
		auto time = clock.MarkMicroSec();
		std::cout << "seq 10k resize, always triggers realloc: " << time.count() << " microsec\n";
		std::cout << "seq size and cap" << seq.size() << "   " << seq.capacity() << '\n';
	}

	_CrtDumpMemoryLeaks();
	return 0;

}