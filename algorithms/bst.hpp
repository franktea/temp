/*
 * bst.hpp
 *
 *  Created on: Aug 21, 2016
 *      Author: frank
 */

#ifndef BST_HPP_
#define BST_HPP_

#include <cstdlib>
#include <string>
#include <sstream>
#include <functional>
#include <cassert>

template<class Key, class Value>
class TreeNode
{
public:
	TreeNode* parent = nullptr;
	TreeNode* left = nullptr;
	TreeNode* right = nullptr;
	Key key;
	Value value;
	TreeNode() {}
	TreeNode(Key key, const Value& value):key(key), value(value) {}
	bool IsLeaf() { return left == nullptr && right == nullptr; }
};

template<class Key, class Value>
class BinarySearchTree
{
public:
	typedef TreeNode<Key, Value> TN;
public:
	size_t Size() { return size_; }
	TN* Find(const Key key);
	TN* Insert(const Key key, const Value& value);
	void Delete(const Key key);
	void Delete(TN* node);
	std::string ToString();
private:
	TN* MinOf(TN* parent);
	TN* MaxOf(TN* parent);
	TN* Successor(TN* node); // next of
	TN* PreSuccessor(TN* node); // pre of
private:
	TN* root_ = nullptr;
	size_t size_ = 0;
};

template<class Key, class Value>
inline typename BinarySearchTree<Key, Value>::TN* BinarySearchTree<Key, Value>::Find(const Key key)
{
	if(!root_) return nullptr;
	TN* node = root_;
	while(node)
	{
		if(key == node->key)
		{
			return node;
		}
		else if(key < node->key)
		{
			node = node->left;
		}
		else // key > node->key_
		{
			node = node->right;
		}
	}
	return nullptr;
}

template<class Key, class Value>
inline typename BinarySearchTree<Key, Value>::TN* BinarySearchTree<Key, Value>::Insert(const Key key,
		const Value& value)
{
	if(! root_)
	{
		root_ = new TreeNode<Key, Value>(key, value);
		return root_;
	}

	TN* node = root_;
	TN* parent = nullptr;
	while(node)
	{
		if(key == node->key)
		{
			return nullptr;
		}
		else if(key < node->key)
		{
			parent = node;
			node = node->left;
		}
		else // key > node->key_
		{
			parent = node;
			node = node->right;
		}
	}

	// now node is null
	node = new TN(key, value);
	node->parent = parent;
	if(key < parent->key)
	{
		parent->left = node;

	}
	else
	{
		parent->right = node;
	}
	return node;
}

template<class Key, class Value>
inline void BinarySearchTree<Key, Value>::Delete(const Key key)
{
	TN* node = Find(key);
	if(node) Delete(node);
}

template<class Key, class Value>
inline void BinarySearchTree<Key, Value>::Delete(TN* node)
{
	if(node == nullptr) return;

	TN* parent = node->parent;

	if(node->left == nullptr && node->right == nullptr) //both null
	{
		if(parent)
		{
			TN** pp = parent->left == node ? & parent->left : & parent->right;
			*pp = nullptr;
		}
		else
		{
			root_ = nullptr;
		}
		// 下面3行和上面有什么区别？下面3行结果不对
//		if(parent && parent->left == node) parent->left = nullptr;
//		if(parent && parent->right == node) parent->right == nullptr;
//		if(!parent) root_ = nullptr;
		delete node;
	}
	else if(node->left == nullptr || node->right == nullptr) // one of left or right is not null
	{
		TN* child = node->left ? node->left : node->right;
		child->parent = node->parent;
		if(parent)
		{
			TN** p = parent->left == node ? &parent->left : &parent->right;
			*p = child;
		}
		else
		{
			root_ = child;
		}
		delete node;
	}
	else // left && right
	{
		TN* next = Successor(node);
		assert(next);
		TN** pp = next->parent->left == next ? & next->parent->left : & next->parent->right;
		if(next->IsLeaf())
		{
			*pp = nullptr;
		}
		else // 后继节点最多有一个子节点，用发证法可以证明，如果有二个子节点，则其中一个子节点一定更适合作为node的后继节点
		{
			TN* pc = next->left ? next->left : next->right;
			pc->parent = next->parent;
			*pp = pc;
		}
		node->key = next->key;
		node->value = next->value;
		delete next;
	}
}

template<class Key, class Value>
inline typename BinarySearchTree<Key, Value>::TN* BinarySearchTree<Key, Value>::MinOf(TN* parent)
{
	if(nullptr == parent) return nullptr;

	while(nullptr != parent->left)
	{
		parent = parent->left;
	}

	return parent;
}

template<class Key, class Value>
inline std::string BinarySearchTree<Key, Value>::ToString()
{
	std::function<void(const TN*, std::string, std::string&)> f = [&](const TN* node, std::string tabs, std::string& result)
		{
			if(node == nullptr) return;
			std::stringstream ss;
			ss<<tabs<<node->key<<"\n";
			result.append(ss.str());
			f(node->left, tabs + "    ", result);
			f(node->right, tabs + "    ", result);
		};
	std::string result;
	f(root_, std::string(""), result);
	return result;
}

template<class Key, class Value>
inline typename BinarySearchTree<Key, Value>::TN* BinarySearchTree<Key, Value>::MaxOf(TN* parent)
{
	if(nullptr == parent) return nullptr;

	while(nullptr != parent->right)
	{
		parent = parent->right;
	}

	return parent;
}

template<class Key, class Value>
inline typename BinarySearchTree<Key, Value>::TN* BinarySearchTree<Key, Value>::Successor(TN* node)
{
	if(node == nullptr) return nullptr;
	if(node->right)
		return MinOf(node->right);

	TN* parent = node->parent;
	while(parent && node == parent->right)
	{
		node = parent;
		parent = node->parent;
	}
	return parent;
}

template<class Key, class Value>
inline typename BinarySearchTree<Key, Value>::TN* BinarySearchTree<Key, Value>::PreSuccessor(TN* node)
{
	if(node == nullptr) return nullptr;
	if(node->left)
		return MaxOf(node->left);

	TN* parent = node->parent;
	while(parent && node == parent->left)
	{
		node = parent;
		parent = node->parent;
	}
	return parent;
}

#endif /* BST_HPP_ */
