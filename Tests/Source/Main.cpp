#include <Fibers/Fiber.h>

#include <iostream>

int main(int argc, char** argv)
{
	Fibers::Fiber fiber {
		Fibers::ECallingConvention::Native,
		65536,
		[]()
		{
		    std::cout << "Nice\n";
		    Fibers::Yield();
		    std::cout << "Even nicer\n";

		    Fibers::Fiber anotherFiber {
			    Fibers::ECallingConvention::Native,
			    65536,
			    []()
			    {
			        std::cout << "Very cool\n";
			    }
		    };

		    anotherFiber.resume();
		}
	};

	fiber.resume();
	std::cout << "Ok\n";
	fiber.resume();
	fiber.resume();
	std::cout << "Godlike\n";
}