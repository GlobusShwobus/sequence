#pragma once
#include <chrono>

class Stopwatch {
	std::chrono::steady_clock::time_point last;

	std::chrono::duration<float> Mark() {
		const auto old = last;
		last = std::chrono::steady_clock::now();
		const std::chrono::duration<float> frametime = last - old;
		return frametime;
	}

public:
	Stopwatch() {
		last = std::chrono::steady_clock::now();
	}

	float MarkFloat() {
		auto time = Mark();
		return time.count();
	}
	std::chrono::milliseconds MarkMilliSec() {
		auto time = Mark();
		return std::chrono::duration_cast<std::chrono::milliseconds>(time);
	}
	std::chrono::microseconds MarkMicroSec() {
		auto time = Mark();
		return std::chrono::duration_cast<std::chrono::microseconds>(time);
	}
};