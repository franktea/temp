/*
 * test_main.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: frank
 */

#include <cstdlib> // for std::srand
#include <ctime> // std::time
#include "gtest/gtest.h"

int main(int argc, char **argv)
{
	std::srand(size_t(std::time(0)));
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


