#include <stdio.h>
#include <stddef.h>

// 二叉树的定义
struct node {int value; struct node *left, *right; };
// 一个数组，数组长度不低于二叉树的高度，为了简便起见，这里假设是一百,
// 自己使用得时候可以改成动态数组(或链表)
int vec_left[100] = {0};

// 显示二叉树的函数，只要调用Display(root, 0)即可
void Display(struct node* root, int ident)
{
	if(ident > 0)
	{
		for(int i = 0; i < ident - 1; ++i)
		{
			printf(vec_left[i] ? "│   " : "    ");
		}
		printf(vec_left[ident-1] ? "├── " : "└── ");
	}

	if(! root)
	{
		printf("(null)\n");
		return;
	}

	printf("%d\n", root->value);
	if(!root->left && !root->right)
	{
		return;
	}

	vec_left[ident] = 1;
	Display(root->left, ident + 1);
	vec_left[ident] = 0;
	Display(root->right, ident + 1);
}

void display(struct node* root, int ident)
{
	printf("%d\n", root->value);
	if(!root->left && !root->right)
	{
		return;
	}

	for(int i = 0; i < ident; ++i)
	{
		printf(vec_left[i] ? "│   " : "    ");
	}
	printf("├── ");
	if(root->left)
	{
		vec_left[ident] = 1;
		display(root->left, ident + 1);
		vec_left[ident] = 0;
	}
	else
	{
		printf("(null)\n");
	}
	for(int i = 0; i < ident; ++i)
	{
		printf(vec_left[i] ? "│   " : "    ");
	}
	printf("└── ");
	if(root->right)
	{
		display(root->right, ident + 1);
	}
	else
	{
		printf("(null)\n");
	}
}

int main()
{
	struct node* root = new node{1, NULL, new node{2, new node{3, new node{4, new node{5}, new node{6}}, new node{7, NULL, new node{8, new node{9}}}}}};
	Display(root, 0);
	return 0;
}
