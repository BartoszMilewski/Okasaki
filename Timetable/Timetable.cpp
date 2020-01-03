#include "../List/List.h"
#include "../RBMap/RBMap.h"
#include "../RBTree/RBTree.h"
#include "../Helper/Futures.h"
#include "../Helper/Utils.h"

#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <string>
#include <future>
#include <chrono>

// Eager Backtracking a

using Talk = int;

struct Person
{
    std::string _name;
    List<Talk>  _talks;
};

std::ostream& operator<<(std::ostream& os, Person const & p)
{
    os << p._name << ", " << p._talks << std::endl;
    return os;
}

using Persons = List<Person>;

using TalkList = List<Talk>;

using TalkSet = RBTree<Talk>;

using TimeTable = List<TalkList>;

// Constraints for the solution search

struct Constr
{
    Constr(int maxSlots, int maxTracks, List<Person> const & people)
    : _maxSlots(maxSlots), _maxTracks(maxTracks)
    {
        forEach(people, [&](Person const & person)
        {
            TalkList talks = person._talks;
            forEach(talks, [this, talks](Talk tk)
            {
                TalkList otherTalks = talks.removed(tk);
                TalkSet set(std::begin(otherTalks), std::end(otherTalks));
                _clashMap = _clashMap.insertedWith(tk, set, &treeUnion<Talk>);

            });
        });
    }
    bool isMaxTracks(int trackNo) const { return trackNo == _maxTracks; }
    bool isMaxSlots(int slotNo) const { return slotNo == _maxSlots; }
    TalkSet clashesWith(Talk t) const
    {
        return _clashMap.findWithDefault(TalkSet(), t);
    }

private:
    int _maxSlots;
    int _maxTracks;
    RBMap<Talk, TalkSet> _clashMap;
};

struct PartSol
{
    PartSol(TalkList const & talks)
    : _curSlotNo(0), _remainingTalks(talks)
    {}
    PartSol(int curSlotNo, TalkList const & remainingTalks, TimeTable tableSoFar)
        : _curSlotNo(curSlotNo), _remainingTalks(remainingTalks), _tableSoFar(tableSoFar)
    {}
    PartSol removeTalk(Talk tk) const
    {
        return PartSol(_curSlotNo, _remainingTalks.removed1(tk), _tableSoFar);
    }
    PartSol fillSlot(TalkList const & talks) const
    {
        return PartSol(_curSlotNo + 1, _remainingTalks, _tableSoFar.pushed_front(talks));
    }
    int _curSlotNo;
    TalkList  _remainingTalks;
    TimeTable _tableSoFar;
};

struct PartSlot
{
    typedef TimeTable SolutionT;

    PartSlot(PartSol const & partSol)
        : _curTrackNo(0), _talksForSlot(partSol._remainingTalks), _partSol(partSol)
    {}
    PartSlot(int curTrackNo, TalkList const & talksInSlot, TalkList const & talksForSlot, PartSol const & partSol)
        : _curTrackNo(curTrackNo), _talksInSlot(talksInSlot), _talksForSlot(talksForSlot), _partSol(partSol)
    {}
    List<PartSlot> refineSlot(Constr const & constr) const;
    List<PartSlot> refine(Constr const & constr) const;
    bool isFinished(Constr const & constr) const
    {
        return constr.isMaxSlots(_partSol._curSlotNo);
    }
    TimeTable getSolution() const { return _partSol._tableSoFar; }

    int       _curTrackNo;
    TalkList  _talksInSlot;
    TalkList  _talksForSlot;
    PartSol   _partSol;
};

List<PartSlot> PartSlot::refineSlot(Constr const & constr) const
{
    List<PartSlot> candts;
    _talksForSlot.forEach([this, &constr, &candts](Talk tk)
    {
        TalkList otherTalks = _talksForSlot.removed1(tk);
        TalkSet clashesWithT = constr.clashesWith(tk);
        candts = candts.pushed_front(
            PartSlot(_curTrackNo + 1
            , _talksInSlot.pushed_front(tk)
            , rem_from_list(otherTalks, clashesWithT)
            , _partSol.removeTalk(tk))
            );
    });
    return candts;
}

List<PartSlot> PartSlot::refine(Constr const & constr) const
{
    if (constr.isMaxTracks(_curTrackNo))
    {
        return List<PartSlot>(PartSlot(_partSol.fillSlot(_talksInSlot)));
    }
    else
    {
        return refineSlot(constr);
    }
}

template<class Partial, class Constraint>
int generate(Partial const & part, Constraint const & constr)
{
    if (part.isFinished(constr))
    {
        return 1;
    }
    else
    {
        int candts = 0;
        List<Partial> partList = part.refine(constr);
        partList.forEach([&](Partial const & part){
            candts += generate(part, constr);
        });
        return candts;
    }
}

template<class Partial, class Constraint>
int generatePar(int depth, Partial const & part, Constraint const & constr)
{
    if (depth == 0)
    {
        return generate(part, constr);
    }
    else if (part.isFinished(constr))
    {
        return 1;
    }
    else
    {
        std::vector<std::future<int>> futResult;
        List<Partial> partList = part.refine(constr);
        forEach(std::move(partList), [&constr, &futResult, depth](Partial const & part)
        {
            std::future<int> futCount =
                std::async([constr, part, depth]() {
                return generatePar(depth - 1, part, constr);
            });
            futResult.push_back(std::move(futCount));
        });
        std::vector<int> all = when_all_vec(futResult);
        return std::accumulate(std::begin(all), std::end(all), 0);
    }
}

int timeTable(Persons const & persons
    , TalkList const & allTalks
    , int maxTracks
    , int maxSlots
    , int depth)
{
    Constr constr(maxSlots, maxTracks, persons);
    PartSol emptySol(allTalks);
    return generatePar(depth, PartSlot(emptySol), constr);
}

int test(int depth)
{
    List<Talk> l1 = { 1, 2 };
    List<Talk> l2 = { 2, 3 };
    List<Talk> l3 = { 3, 4 };
    List<Person> persons = { { "P", l1 }, { "Q", l2 }, { "R", l3 } };
    List<Talk> talks = { 1, 2, 3, 4 };
    return timeTable(persons, talks, 2, 2, depth);
}

int bench(int depth)
{
    List<Talk> talks = iterateN([](int i) { return i + 1; }, 1, 12);
    List<Person> persons = {
        { "P10", { 8, 9, 2 } },
        { "P9", { 4, 3, 6 } },
        { "P8", { 11, 9, 10 } },
        { "P7", { 8, 5, 3 } },
        { "P6", { 2, 10, 5 } },
        { "P5", { 7, 3, 8 } },
        { "P4", { 9, 6, 10 } },
        { "P3", { 8, 1, 6 } },
        { "P2", { 3, 8, 4 } },
        { "P1", { 10, 8, 6 } }
    };
    //std::cout << persons << std::endl;
    return timeTable(persons, talks, 4, 3, depth);
}

void testBench(int depth)
{
    std::cout << "Depth of parallelism: " << depth << std::endl;
    auto start = std::chrono::steady_clock::now();
    int solCount = bench(depth);
    auto end = std::chrono::steady_clock::now();
    std::cout << "Found " << solCount << " solutions" << std::endl;
    auto diff_sec = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::cout << diff_sec.count() << " s" << std::endl;
}

/*

Eager algorithm
Depth of parallelism: 0
Found 8356608 solutions
646 s
Depth of parallelism: 1
Found 8356608 solutions
250 s
Depth of parallelism: 2
Found 8356608 solutions
249 s
Depth of parallelism: 3
Found 8356608 solutions
248 s
Depth of parallelism: 0
Found 8356608 solutions
176 s

*/

void main()
{
    std::cout << "Eager algorithm\n";
    //std::cout << test(0) << std::endl;

    testBench(0);
    testBench(1);
    testBench(2);
    testBench(3);
    testBench(0);
}
