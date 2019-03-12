#ifndef __KRANEWM__UTIL__GUARD__
#define __KRANEWM__UTIL__GUARD__

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <list>


void die(const ::std::string&);


template <typename T>
struct range_t
{
    static bool contains(T min, T max, T i)
    {
        return min <= i  && i <= max;
    }

    range_t(T _min = 0, T _max = 0)
      : min(_min),
        max(_max)
    {}

    inline bool operator==(const range_t<T>& range) const
    {
        return min == range.min && max == range.max;
    }

    T min;
    T max;
};


template <typename Keytype, typename Valtype>
void
erase_find(::std::unordered_map<Keytype, Valtype>& m, Keytype key)
{
    auto i = m.find(key);
    if (i == m.end())
        return;

    m.erase(i);
}

template <typename Valtype>
void
erase_find(::std::list<Valtype>& v, Valtype val)
{
    auto i = ::std::find(v.begin(), v.end(), val);
    if (i == v.end())
        return;

    v.erase(i);
}

template <typename Valtype>
void
erase_remove(::std::vector<Valtype>& v, Valtype val)
{
    auto i = ::std::remove(v.begin(), v.end(), val);
    if (i == v.end())
        return;

    v.erase(i, v.end());
}

template <typename Valtype>
void
erase_remove(::std::deque<Valtype>& v, Valtype val)
{
    auto i = ::std::remove(v.begin(), v.end(), val);
    if (i == v.end())
        return;

    v.erase(i, v.end());
}

template <typename Valtype>
void
splice_back(::std::list<Valtype>& v, Valtype val)
{
    auto i = ::std::find(v.begin(), v.end(), val);
    if (i == v.end())
        return;

    v.splice(v.end(), v, i);
}

#endif//__KRANEWM__UTIL__GUARD__
