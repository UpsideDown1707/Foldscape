#include "application.hpp"
#include <chrono>
#include <iostream>

class AppTimer
{
	std::chrono::steady_clock::time_point m_startTime;

public:
	AppTimer()
		: m_startTime{std::chrono::steady_clock::now()}
	{}
	~AppTimer()
	{
		std::cout << "Application run time: " <<
				std::chrono::duration<double>(std::chrono::steady_clock::now() - m_startTime).count() <<
				" seconds. " << std::endl;
	}
};

int main(int argc, char* argv[])
{
#ifdef DEBUG
	AppTimer timer;
#endif
	if (argc)
		foldscape::SaveProgramFolder(argv[0]);
	return foldscape::Application().Main(argc, argv);
}
