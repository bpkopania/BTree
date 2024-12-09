#include <iostream>
#include "BTree.h"

class IntegerBTree
{
public:
	int value;
	void print()
	{
		std::cout << value << std::endl;
	
	}
};

void f(const BTreeNode &node)
{
	std::cout << "Node: " << node.keys[0] << std::endl;
}

int main()
{
	BTree<IntegerBTree> b_tree;
	IntegerBTree record;
	record.value = 10;

	b_tree.insert(1, record);
	b_tree.insert(20, record);
	b_tree.insert(50, record);
	b_tree.insert(30, record);

	b_tree.insert(35, record);

	b_tree.insert(36, record);
	b_tree.insert(37, record);

	/*b_tree.print();
	printf("\n\n\n");*/

	b_tree.insert(38, record);

	/*b_tree.print();
	printf("\n\n\n");*/

	b_tree.insert(39, record);
	b_tree.insert(41, record);

	b_tree.insert(42, record);
	b_tree.insert(43, record);

	b_tree.print();
	printf("\n\n\n");

	b_tree.insert(44, record);

	b_tree.print();

}
