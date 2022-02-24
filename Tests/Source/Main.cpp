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

		    Fibers::Fiber anotherFiber {
			    65536,
			    [](float a, float b, float c, float d, float e, float f, float g)
			    {
			        std::cout << a << " " << b << " " << c << " " << d << " " << e << " " << f << " " << g << std::endl;
			    },
			    1.0f,
			    2.0f,
			    3.0f,
			    4.0f,
			    5.0f,
			    6.0f,
			    7.0f
		    };

		    anotherFiber.resume();
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