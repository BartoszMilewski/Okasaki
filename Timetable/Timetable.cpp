#include <string>
#include <thread>
#include <future>
#include <vector>

#include "../List/List.h"
#include "../RBMap/RBMap.h"

template<class T, class U>
std::ostream& operator<<(std::ostream& os, std::pair<T, U> const & p)
{
    os << "(" << p.first << ", " << p.second << ") ";
    return os;
}

template<class T>
std::future<T> make_ready_future(T val)
{
    std::promise<T> promise;
    promise.set_value(val);
    return promise.get_future();
}

template<class T>
List<T> when_all_vec(std::vector<std::future<T>> & ftrs)
{
    List<T> lst;
    while (!ftrs.empty())
    {
        std::future<T> f = std::move(ftrs.back());
        lst = lst.push_front(f.get());
        ftrs.pop_back();
    }
    return lst;
}

using Talk = int;

struct Person
{
    std::string _name;
    List<Talk>  _talks;
};

using Persons = List<Person>;

using TalkList = List<Talk>;

using TimeTable = List<TalkList>;

using Selections = List<std::pair<Talk, List<Talk>>>;

template<class T>
List<std::pair<T, List<T>>> selAccum(List<T> const & leftLst, List<T> const & rightLst)
{
    if (rightLst.isEmpty())
        return List<std::pair<T, List<T>>>();
    else
    {
        // make recursive call moving one element from right to left list
        auto rest = selAccum(leftLst.push_front(rightLst.front()), rightLst.pop_front());
        // pair head of right list with left ++ right.tail
        auto p = std::make_pair(rightLst.front(), concat(leftLst, rightLst.pop_front()));
        // prepend pair to rest
        return rest.push_front(p);
    }
}

// Create list of pairs: (element, rest of elements)
template<class T>
List<std::pair<T, List<T>>> selects(List<T> const & lst)
{
    return selAccum(List<T>(), lst);
}

// Constraints for the solution search

struct Constr
{
    Constr(int maxSlots, int maxTracks, List<Person> const & people)
        : _maxSlots(maxSlots), _maxTracks(maxTracks)
    {
        forEach(people, [&](Person const & person)
        {
            Selections sels = selects<Talk>(person._talks);
            forEach(sels, [&](std::pair<Talk, List<Talk>> const & p)
            {
                _clashMap = _clashMap.insertWith(p.first, p.second, &unionize<Talk>);
            });
        });
    }
    bool isMaxTracks(int trackNo) const { return trackNo == _maxTracks; }
    bool isMaxSlots(int slotNo) const { return slotNo == _maxSlots; }
    List<Talk> clashesWith(Talk t) const 
    { 
        return _clashMap.findWithDefault(List<Talk>(), t); 
    }

private:
    int _maxSlots;
    int _maxTracks;
    RBMap<Talk, List<Talk>> _clashMap;
};

struct PartSol
{
    typedef TimeTable SolutionT;

    PartSol(TalkList const & allTalks)
        : _slotNo(0)
        , _trackNo(0)
        , _talksForSlot(allTalks)
        , _remainingTalks(allTalks)
    {}
    PartSol(int slotNo
        , int trackNo
        , TimeTable const & tableSoFar
        , TalkList const & talksInSlot
        , TalkList const & talksForSlot
        , TalkList const & remainingTalks)
        : _slotNo(slotNo)
        , _trackNo(trackNo)
        , _tableSoFar(tableSoFar)
        , _talksInSlot(talksInSlot)
        , _talksForSlot(talksForSlot)
        , _remainingTalks(remainingTalks)
    {}
    bool isFinished(Constr const & constr) const
    {
        return constr.isMaxSlots(_slotNo);
    }
    List<PartSol> refine(Constr const & constr) const;
    TimeTable getSolution() const { return _tableSoFar; }
    friend std::ostream& operator<<(std::ostream& os, PartSol const & p);
private:
    int       _slotNo;
    int       _trackNo;
    TimeTable _tableSoFar;
    TalkList  _talksInSlot;
    TalkList  _talksForSlot;
    TalkList  _remainingTalks;

};

std::ostream& operator<<(std::ostream& os, PartSol const & p)
{
    os << p._slotNo << ", " << p._trackNo << "| in: " << p._talksInSlot << ", for: " << p._talksForSlot << ", left: " << p._remainingTalks << std::endl;
    return os;
}


List<PartSol> PartSol::refine(Constr const & constr) const
{
    List<PartSol> candts;
    if (constr.isMaxTracks(_trackNo))
    {
        candts =    candts.push_front(PartSol( _slotNo + 1
                                             , 0
                                             , _tableSoFar.push_front(_talksInSlot)
                                             , List<Talk>()
                                             , _remainingTalks
                                             , _remainingTalks));
    }
    else
    {
        List<std::pair<Talk, List<Talk>>> pairs = selects(_talksForSlot);
        forEach(pairs, [&](std::pair<Talk, List<Talk>> const & p) {
            List<Talk> clashesWithT = constr.clashesWith(p.first);
            List<Talk> slotTalks = diff(p.second, clashesWithT);
            List<Talk> talks = _remainingTalks.remove(p.first);
            candts = candts.push_front(PartSol( _slotNo
                                              , _trackNo + 1
                                              , _tableSoFar
                                              , _talksInSlot.push_front(p.first)
                                              , slotTalks
                                              , talks));
        });
    }
    return candts;
}

template<class Partial, class Constraint>
List<typename Partial::SolutionT> generate(Partial const & part, Constraint const & constr)
{
    if (part.isFinished(constr))
    {
        return List<typename Partial::SolutionT>(part.getSolution());
    }
    else
    {
        List<typename Partial::SolutionT> result;
        List<Partial> partList = part.refine(constr);
        forEach(partList, [&](Partial const & part){
            List<typename Partial::SolutionT> lst = generate(part, constr);
            result = concat(result, lst);
        });
        return result;
    }
}

template<class Partial, class Constraint>
List<typename Partial::SolutionT> generatePar(int depth, Partial const & part, Constraint const & constr)
{
    if (depth == 0)
    {
        return generate(part, constr);
    }
    else if (part.isFinished(constr))
    {
        return List<typename Partial::SolutionT>(part.getSolution());
    }
    else
    {
        std::vector<std::future<List<typename Partial::SolutionT>>> futResult;
        List<Partial> partList = part.refine(constr);
        forEach(partList, [&constr, &futResult, depth](Partial const & part)
        {
            std::future<List<typename Partial::SolutionT>> futLst =
                std::async([constr, part, depth]() {
                    return generatePar(depth - 1, part, constr);
                });
            futResult.push_back(std::move(futLst));
        });
        List<List<typename Partial::SolutionT>> all = when_all_vec(futResult);
        return concatAll(all);
    }
}

List<TimeTable> timeTable(Persons const & persons, TalkList const & allTalks, int maxTracks, int maxSlots)
{
    Constr constr(maxSlots, maxTracks, persons);
    PartSol emptySol(allTalks);
    //return generate(emptySol, constr);
    return generatePar(3, emptySol, constr);
}

List<TimeTable> test()
{
    List<Talk> l1 = { 1, 2 };
    List<Talk> l2 = { 2, 3 };
    List<Talk> l3 = { 3, 4 };
    List<Person> persons = { { "P", l1 }, { "Q", l2 }, { "R", l3 } };
    List<Talk> talks = { 1, 2, 3, 4 };
    return timeTable(persons, talks, 2, 2);
}

List<TimeTable> test2(int n, int m)
{
    List<Talk> talks = iterateN([](int i) { return i + 1; }, 1, n * m);
    List<Person> persons = { { "1", talks.take(n) } };
    return timeTable(persons, talks, m, n);
}

void main()
{
    test();
    /*
    forEach(test(), [](TimeTable const & table){
        forEach(table, [](TalkList const & talks){
            std::cout << talks;
        });
        std::cout << std::endl;
    });
    */
}