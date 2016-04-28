#include <tchar.h>

#include "gtest/gtest.h"

int main(int argc, char* argv[])
{
	int i = 0;
	char* c = "Testing1234";

	testing::InitGoogleTest(&i, &c);

	return RUN_ALL_TESTS();

	// Place a breakpoint here to keep the console from closing automatically.
	return 0;
}