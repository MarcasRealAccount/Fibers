#include <Fibers/Fibers.h>

#include <iostream>

int main(int argc, char** argv)
{
	Fibers::Fiber fiber {
		65536,
		[]()
		{
		    std::cout << "Nice\n";
		    Fibers::yield();
		    std::cout << "Even nicer\n";

		    Fibers::Fiber anotherFiber {
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