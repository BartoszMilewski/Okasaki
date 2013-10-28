#include "Tree.h"
#include <iostream>

template<class T>
void print(Tree<T> const & t, int offset = 0)
{
	if (!t.isEmpty())
	{
		for (int i = 0; i < offset; ++i)
			std::cout << " ";
		std::cout << t.root() << std::endl;
		print(t.left(), offset);
		print(t.right(), offset + 4);
	}
}

void testHigher()
{
	Tree<int> t0;
	auto t = t0.insert(2).insert(4).insert(3).insert(5).insert(6).insert(1);
	forEach(t, [](int x)
	{
		std::cout << x << " ";
	});
	std::cout << std::endl;
}

void main()
{
	testHigher();
}