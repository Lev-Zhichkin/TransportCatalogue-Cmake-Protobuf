#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <memory>

class Timer
{
public:
	Timer(const std::string text)
	{
		m_StartTimepoint = std::chrono::high_resolution_clock::now();
		text_ = text;
	}

	~Timer()
	{
		Stop();
	}

	void Stop()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(m_StartTimepoint).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::milliseconds>(endTimepoint).time_since_epoch().count();

		auto duration = end - start;

		std::cout << std::endl << std::endl << std::endl << "      " << text_ << "     " << duration << "       " << text_ << "       " << std::endl << std::endl << std::endl;
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
	std::string text_;
};
