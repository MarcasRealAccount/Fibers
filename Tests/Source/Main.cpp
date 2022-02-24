#include <Fibers/Fibers.h>

#include <iostream>

int main(int argc, char** argv)
{
	Fibers::Fiber fiber {
		65536,
		[](float arg0, std::uint64_t arg1)
		{
		    std::cout << "Nice " << arg0 << std::endl;
		    Fibers::yield();
		    std::cout << "Even nicer " << arg1 << std::endl;
		},
		54.0f,
		1005ULL
	};

	fiber.resume();
	std::cout << "Ok\n";
	fiber.resume();
	fiber.resume();
	std::cout << "Godlike\n";
}