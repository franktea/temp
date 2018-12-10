/*
 * bst_test.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: frank
 */

#include <iostream>
#include <algorithm>
#include <vector>
#include <cstdlib> // std::random, std::srand
#include "gtest/gtest.h"
#include "bst.hpp"

TEST(BSTTEST, InsertFindTest)
{
	BinarySearchTree<int, int> bst;
	std::vector<int> v(100);
	int n = 0;
	std::generate(v.begin(), v.end(), [&n]{return n++;});
	std::random_shuffle(v.begin(), v.end(), [](int i){return std::rand() % i; });

	for(const int& i: v)
	{
		auto p = bst.Insert(i, i);
		EXPECT_EQ(p->value, i);
	}

	//std::cout<<bst.ToString();

	for(const int& i: v)
	{
		auto p = bst.Find(i);
		ASSERT_NE(p, nullptr)<<"not found key="<<i<<", and the tree is: "<<bst.ToString();
		EXPECT_EQ(p->value, i);
	}
}

TEST(BSTTEST, DeleteTest)
{
	BinarySearchTree<int, int> bst;
	std::vector<int> v(9999);
	int n = 0;
	std::generate(v.begin(), v.end(), [&n]{return n++;});
	std::random_shuffle(v.begin(), v.end(), [](int i){return std::rand() % i; });


	for(const int& i: v)
	{
		auto p = bst.Insert(i, i);
		EXPECT_EQ(p->value, i);
	}

	std::cout<<bst.ToString();

//	v = {4, 1, 3};
//	{
//		auto p = bst.Find(4);
//		ASSERT_TRUE(p->left == nullptr);
//		ASSERT_TRUE(p->right == nullptr);
//		ASSERT_TRUE(p->parent->key == 3);
//		ASSERT_TRUE(p->parent->right == p);
//		bst.Delete(4);
//		p = bst.Find(3);
//		ASSERT_TRUE(p->left->key == 1);
//		ASSERT_TRUE(p->right == nullptr);
//	}

	std::random_shuffle(v.begin(), v.end(), [](int i){return std::rand() % i; });
	for(auto it = v.begin(); it != v.end(); ++it)
	{
		auto p = bst.Find(*it);
		ASSERT_NE(p, nullptr)<<"value="<<*it<<" is not found";
		bst.Delete(*it);
		//std::cout<<"after delete "<<*it<<":"<<std::endl;
		//std::cout<<bst.ToString();
		p = bst.Find(*it);
		ASSERT_EQ(p, nullptr)<<"value = "<<*it<<" is not successfully deleted.";
		for(auto it2 = it + 1; it2 != v.end(); ++it2)
		{
			auto p2 = bst.Find(*it2);
			ASSERT_NE(p2, nullptr);
		}
		//std::cout<<"sussessfully delete item="<<*it<<"========================================="<<std::endl;
	}
}
