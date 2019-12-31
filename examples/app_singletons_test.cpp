
#include <app_singletons.h>

int main()
{
	std::cout << "main\n";
	es::init::args.for_each([](int index, auto& arg) { std::cout << "lambda arg[" << index << "]: '" << arg << "'" << std::endl; } );
	es::init::env.for_each([](int index, auto& earg) { std::cout << "lambda env[" << index << "]: '" << earg << "'" << std::endl; } );

	return 0;
}
