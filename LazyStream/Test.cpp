#include "LazyStream.h"
#include <iostream>


std::shared_ptr<const Stream<int>> mkStream()
{
	auto s = std::make_shared<const Stream<int>> ([]()
	{
		return std::unique_ptr<Cell<int>>(new Cell<int>());
	});
	bool b = s->isEmpty();

	auto s1 = std::make_shared<const Stream<int>> ([=]()
	{
		return std::unique_ptr<Cell<int>>(new Cell<int>(13, s));
	});
	bool b1 = s1->isEmpty();

	int x = s1->get();
	bool b3 = s1->tail()->isEmpty();

	auto s2 = std::make_shared<const Stream<int>>([=]()
	{
		return std::unique_ptr<Cell<int>>(new Cell<int>(14, s1));
	});
	return s2;
}

template<class T>
void forcePrint(std::shared_ptr<const Stream<T>> const & s)
{
	if (s->isEmpty())
	{
		std::cout << std::endl;
	}
	else
	{
		T v = s->get();
		auto tail = s->tail();
		std::cout << v << " ";
		forcePrint(tail);
	}
}

void main()
{
	auto s = mkStream();
	auto s1 = concat<int>(s, s);
	forcePrint(s1);
}