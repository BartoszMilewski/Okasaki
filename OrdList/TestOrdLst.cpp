#include "OrdList.h"
#include <iostream>

void testOrdLst()
{
	OrdList<char> lst0;
	auto lst1 = lst0.insert('m');
	print(lst1);
	auto lst2 = lst1.insert('p');
	print(lst2);
	auto lst3 = lst2.insert('c');
	auto lst4 = lst1.insert('d');
	auto lst5 = lst1.insert('e');
	std::cout << "Merging\n";
	print(lst3);
	print(lst5);
	auto lst = merge(lst3, lst5);
	print(lst);
}

void main() 
{
	testOrdLst();
}

