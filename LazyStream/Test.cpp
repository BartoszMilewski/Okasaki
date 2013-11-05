#include "LazyStream.h"
#include <iostream>

Stream<int> mkStream(int a, int b)
{
	//Stream<int> s([]()
	//{
	//	return std::unique_ptr<Cell<int>>(new Cell<int>());
	//});

	EmptyCellGen<int> e;
	Stream<int> s(e);

	//Stream<int> s1([=]()
	//{
	//	return std::unique_ptr<Cell<int>>(new Cell<int>(13, s));
	//});

	Stream<int> s1(CellGen<int>(b, s));

	// int x = s1.get();
	// bool b3 = s1.tail().isEmpty();

	//Stream<int> s2([=]()
	//{
	//	return std::unique_ptr<Cell<int>>(new Cell<int>(14, s1));
	//});

	Stream<int> s2(CellGen<int>(a, s1));
	return s2;
}

template<class T>
void forcePrint(Stream<T> const & s)
{
	if (s.isEmpty())
	{
		std::cout << std::endl;
	}
	else
	{
		T v = s.get();
		auto tail = s.tail();
		std::cout << v << " ";
		forcePrint(tail);
	}
}

void main()
{
	auto s1 = mkStream(5, 10);
	auto s2 = mkStream(15, 20);
	auto s = concat<int>(s1, s2);
	forcePrint(s);
}