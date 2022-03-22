#include <Fibers/Fiber.h>
#include <Fibers/FiberLocal.h>

#include <iostream>

struct Object
{
	~Object()
	{
		std::cout << "Woot woot\n";
	}

	std::uint64_t arr[1020] { 0 };

	void operator()()
	{
		std::cout << "Extremely funky!!\n";
	}
};

Fibers::FiberLocal<int> fiberLocal;

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	Fibers::Fiber otherFiber {
		Fibers::ECallingConvention::Native,
		65536,
		[]()
		{
		    int j = 0;
		    Fibers::Yield(
		        [&j]()
		        {
			        std::cout << j << '\n';
			        ++j;
			        return j <= 2;
		        });
		}
	};

	Fibers::Fiber fiber {
		Fibers::ECallingConvention::Native,
		65536,
		[](int a, float b, int c, float d, Object func, int e, int f, int g, int h, int i, Fibers::Fiber& otherFiber)
		{
		    fiberLocal = c;

		    std::cout << func.arr[0] << '\n';
		    std::cout << a << ", " << b << ", " << fiberLocal << ", " << d << ", " << e << ", " << f << ", " << g << ", " << h << ", " << i << '\n';
		    std::cout << "Nice\n";
		    func();
		    Fibers::Yield();

		    Fibers::Fiber anotherFiber {
			    Fibers::ECallingConvention::Native,
			    65536,
			    []()
			    {
			        fiberLocal = 15;
			        std::cout << fiberLocal << " Very cool\n";
			    }
		    };

		    std::cout << (anotherFiber.resume() ? "true" : "false") << '\n';

		    Fibers::WaitFor(otherFiber);

		    std::cout << a << ", " << b << ", " << fiberLocal << ", " << d << ", " << e << ", " << f << ", " << g << ", " << h << ", " << i << '\n';
		    std::cout << "Even nicer\n";
		    func();
		},
		1, 2.2f, 3, 4.4f, Object { 1 }, 6, 7, 8, 9, 10, otherFiber
	};

	fiber.resume();
	std::cout << "Ok\n";
	while (!fiber.resume())
		otherFiber.resume();
	std::cout << "Godlike\n";
}