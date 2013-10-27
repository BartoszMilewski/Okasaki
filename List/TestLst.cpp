#include "List.h"
#include <iostream>

List<char> test1()
{
	List<char> lst0;
	auto lst1 = lst0.prepend('a');
	// a: ref 1
	print(lst1);
	auto lst2 = lst1.prepend('b');
	// b: ref 1
	// a: ref 2
	print(lst2);
	return lst2;
    // lst1 is deleted decrementing ref count on a
}

List<char> test2()
{
	auto lst3 = test1(); // {b, a}
	// b: ref 1
	// a: ref 1
	print(lst3);
	auto lst4 = lst3.insertAt(1, 'x'); // {b, x, a}
	// x: ref 1
	// b: ref 2
	// a: ref 1
	print (lst4);
	auto lst5 = concat(lst3, lst4);
	print(lst5);
	return lst5;
}

void testLst()
{
	auto lst = test2();
	print(lst);
}

void main()
{
	testLst();
}

