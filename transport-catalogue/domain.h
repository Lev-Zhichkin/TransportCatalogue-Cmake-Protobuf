#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <memory>

class Timer
{
public:
	Timer(const std::string text);
	~Timer();
	void Stop();

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
	std::string text_;
};
