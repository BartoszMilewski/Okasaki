#include "../List/List.h"
#include "../PureStream/PureStream.h"
#include <thread>
#include <future>
#include <vector>
#include <algorithm>
#include <numeric>

template<class T>
std::future<T> make_ready_future(T val)
{
    std::promise<T> promise;
    promise.set_value(val);
    return promise.get_future();
}

template<class T>
std::vector<T> when_all_vec(std::vector<std::future<T>> & ftrs)
{
    std::vector<T> lst;
    while (!ftrs.empty())
    {
        std::future<T> f = std::move(ftrs.back());
        lst.push_back(f.get());
        ftrs.pop_back();
    }
    return lst;
}

template<class T>
std::vector<T> concatAll(std::vector<std::vector<T>> const & in)
{
    std::vector<T> res;
    std::for_each(in.begin(), in.end(), [&res](std::vector<T> const & v){
        std::copy(v.begin(), v.end(), std::back_inserter(res));
    });
    return res;
}


struct Pos
{
    Pos(int col, int row) : col(col), row(row) {}
    int col;
    int row;
};

std::ostream& operator<<(std::ostream& os, Pos const & p)
{
    os << "(" << p.col << ", " << p.row << ") ";
    return os;
}

// Partial Solution

class PartSol
{
public:
    typedef List<Pos> SolutionT;

    PartSol() : _curRow(0) {}
    PartSol(int row, List<Pos> const & qs)
        : _curRow(row), _queens(qs) {}
    List<Pos> getSolution() const { return _queens; }

    bool isFinished(int dim) const { return _curRow == dim; }
    Stream<PartSol> refine(int dim) const;

    friend std::ostream& operator<<(std::ostream& os, PartSol const & p);
private:
    Stream<PartSol> refineRow(int col, int dim) const;
    bool isAllowed(Pos const & pos) const;

    int       _curRow;
    List<Pos> _queens;
};

std::ostream& operator<<(std::ostream& os, PartSol const & p)
{
    os << "(" << p._curRow << ", " << p.getSolution() << ") ";
    return os;
}

bool isConflict(Pos const & p, Pos const & q)
{
    return p.col == q.col || p.row == q.row || abs(p.col - q.col) == abs(p.row - q.row);
}

bool PartSol::isAllowed(Pos const & pos) const
{
    for (auto it = std::begin(_queens); it != std::end(_queens); ++it)
    {
        if (isConflict(*it, pos))
            return false;
    }
    return true;
}

Stream<PartSol> PartSol::refineRow(int col, int dim) const
{
    while (col < dim && !isAllowed(Pos(col, _curRow)))
        ++col;
    if (col == dim)
        return Stream<PartSol>();
    return Stream<PartSol>([this, col, dim]() -> Cell<PartSol>
    {
        PartSol part(_curRow + 1, _queens.push_front(Pos(col, _curRow)));
        Stream<PartSol> tail = refineRow(col + 1, dim);
        return Cell<PartSol>(part, tail);
    });
}

Stream<PartSol> PartSol::refine(int dim) const
{
    return refineRow(0, dim);
}


template<class Partial, class Constraint>
std::vector<typename Partial::SolutionT> generate(Partial const & part, Constraint constr)
{
    using SolutionVec = std::vector<typename Partial::SolutionT>;

    if (part.isFinished(constr))
    {
        SolutionVec result{ part.getSolution() };
        return result;
    }
    else
    {
        Stream<Partial> partList = part.refine(constr);

        SolutionVec result;
        forEach(std::move(partList), [&](Partial const & part){
            SolutionVec lst = generate(part, constr);
            std::copy(lst.begin(), lst.end(), std::back_inserter(result));
        });
        return result;
    }
}

template<class Partial, class Constraint>
std::vector<typename Partial::SolutionT> generatePar(int depth, Partial const & part, Constraint constr)
{
    using SolutionVec = std::vector<typename Partial::SolutionT>;

    if (depth == 0)
    {
        return generate(part, constr);
    }
    else if (part.isFinished(constr))
    {
        SolutionVec result{ part.getSolution() };
        return result;
    }
    else
    {
        Stream<Partial> partList = part.refine(constr);

        std::vector<std::future<SolutionVec>> futResult;
        forEach(std::move(partList), [&constr, &futResult, depth](Partial const & part)
        {
            std::future<SolutionVec> futLst =
                std::async([constr, part, depth]() {
                return generatePar(depth - 1, part, constr);
            });
            futResult.push_back(std::move(futLst));
        });
        std::vector<SolutionVec> all = when_all_vec(futResult);
        return concatAll(all);
    }
}

void main()
{
    std::vector<List<Pos>> sol = generatePar(3, PartSol(), 4);
    //std::vector<List<Pos>> sol = generate(PartSol(), 4);
    for_each(sol.begin(), sol.end(), [](List<Pos> const & queens)
    {
        std::cout << queens << std::endl;
    });
}