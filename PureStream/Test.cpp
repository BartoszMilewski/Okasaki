#include "PureStream.h"
#include "../List/List.h"
#include <iostream>

template<class T>
void lazyPrint(Stream<T> const & s)
{
	if (s.isEmpty())
	{
		std::cout << std::endl;
	}
	else if (s.isForced())
	{
		T v = s.get();
		auto tail = s.pop_front();
		std::cout << v << " ";
		lazyPrint(tail);
	}
	else
	{
		std::cout << "?\n";
	}
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
		auto tail = s.pop_front();
		std::cout << v << " ";
		forcePrint(tail);
	}
}

void test0()
{
    Susp<int> i([]() 
    { 
        return 5; 
    });
    std::cout << i.get() << std::endl;
    std::cout << i.get() << std::endl;
}

Stream<int> test1()
{
	Stream<int> s0;
	Stream<int> s1(1, s0);
	lazyPrint(s1);
	Stream<int> s2(2, s1);
	Stream<int> s3(3, s2);
	Stream<int> s4(4, s3);
	std::cout << s1.get() << std::endl;
	lazyPrint(s4);
	return s4;
}

Stream<int> test3()
{
	auto s4 = test1();
	auto rv = s4.reverse();
	std::cout << "Reversed\n";
	lazyPrint(rv);
	forcePrint(rv);
	auto cat = concat(s4, rv);
	return cat;
}

Stream<int> test()
{
	auto cat = test3();
	Stream<int> s(10, cat);
	std::cout << "Cat + 10\n";
	lazyPrint(s);
	forcePrint(s);
	auto t2 = cat.take(5);
	return t2;
}

template<class T>
void printQ(Queue<T> const & q)
{
	std::cout << "Front " << q.lenF() << ": ";
	lazyPrint(q.fStream());
	std::cout << "Rear  " << q.lenR() << ": ";
	lazyPrint(q.rStream());
	std::cout << std::endl;
}

void testQ()
{
	std::cout << "Test Lazy Queue\n";

	Queue<int> q0;
	auto q1 = q0.push_back(10);
	printQ(q1);
	auto q2 = q1.push_back(20);
	printQ(q2);
	auto q3 = q2.push_back(30);
	
	std::cout << "Three element queue\n";

	printQ(q3);
	auto x = q3.front();
	std::cout << "Pop: " << x << std::endl;
	printQ(q3);
	std::cout << "Tail\n";
	auto t1 = q3.pop_front();
	printQ(t1);
	std::cout << std::endl;
	std::cout << "Tail of tail\n";
	auto t2 = t1.pop_front();
	printQ(t2);
	std::cout << "Original\n";
	printQ(q3);
}

template<class T>
Stream<T> concatAll(List<Stream<T>> const & in)
{
    if (in.isEmpty())
        return Stream<T>();
    Stream<T> strm = in.front();
    List<Stream<T>> tailStrm = in.pop_front();
    if (strm.isEmpty())
        return concatAll(tailStrm);
    return Stream<T>([=]()
    {
        T val = strm.get();
        auto tail = strm.pop_front();
        return Cell<T>(val, concatAll<T>(tailStrm.push_front(tail)));
    });
}

template<class T>
Stream<T> mkStream(List<T> lst)
{
    Stream<T> strm;
    lst.forEach([&](T val){
        strm = Stream<T>(val, strm);
    });
    return strm;
}

Stream<int> testCat()
{
    List<int> lst1 = { 1, 3, 5, 7 };
    Stream<int> s1 = mkStream<int>(lst1);
    List<int> lst2 = {10, 20, 30 };
    Stream<int> s2 = mkStream<int>(lst2);
    List<int> lst3 = { 100, 200, 300 };
    Stream<int> s3 = mkStream<int>(lst3);
    List<Stream<int>> lss;
    lss = lss.push_front(s3);
    lss = lss.push_front(s2);
    lss = lss.push_front(s1);
    return concatAll(lss);
}


void main()

{
    auto s = testCat();
    lazyPrint(s);
    forcePrint(s);
}
