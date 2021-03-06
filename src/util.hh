#ifndef __KRANEWM__UTIL__GUARD__
#define __KRANEWM__UTIL__GUARD__

#include <algorithm>
#include <deque>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>


void die(const ::std::string&&);
void warn(const ::std::string&&);
::std::string uppercase(const ::std::string);


template <typename T>
struct range_t final
{
    static bool
    contains(T min, T max, T i)
    {
        return min <= i  && i <= max;
    }

    range_t(T _min = 0, T _max = 0)
      : min(_min),
        max(_max)
    {}

    inline bool
    operator==(const range_t<T>& range) const
    {
        return min == range.min && max == range.max;
    }

    T min;
    T max;

};

namespace std {
    template <typename T1, typename T2>
    struct hash<::std::pair<T1, T2>>
    {
        ::std::size_t operator()(const ::std::pair<T1, T2>& p) const
        {
            return ::std::hash<T1>{}(p.first) + ::std::hash<T2>{}(p.second);
        }
    };
}

template <typename Keytype, typename Valtype>
inline void
erase_find(::std::unordered_map<Keytype, Valtype>& m, Keytype key)
{
    auto i = m.find(key);
    if (i == m.end())
        return;

    m.erase(i);
}

template <typename Valtype>
inline void
erase_find(::std::list<Valtype>& v, Valtype val)
{
    auto i = ::std::find(v.begin(), v.end(), val);
    if (i == v.end())
        return;

    v.erase(i);
}

template <typename Valtype>
inline void
erase_remove(::std::vector<Valtype>& v, Valtype val)
{
    auto i = ::std::remove(v.begin(), v.end(), val);
    if (i == v.end())
        return;

    v.erase(i, v.end());
}

template <typename Valtype>
inline void
erase_remove(::std::deque<Valtype>& v, Valtype val)
{
    auto i = ::std::remove(v.begin(), v.end(), val);
    if (i == v.end())
        return;

    v.erase(i, v.end());
}

template <typename Valtype>
inline void
splice_back(::std::list<Valtype>& v, Valtype val)
{
    auto i = ::std::find(v.begin(), v.end(), val);
    if (i == v.end())
        return;

    v.splice(v.end(), v, i);
}

template <typename Valtype1, typename Valtype2>
inline void
insert_container(::std::vector<Valtype1>& v, ::std::list<Valtype2>& l)
{
    for (auto& le : l)
        v.push_back(le.get());
}

template <typename T>
struct revertible final
{
    T& iterable;
};

template <typename T>
inline auto
begin(revertible<T> r)
{
    return ::std::rbegin(r.iterable);
}

template <typename T>
inline auto
end(revertible<T> r)
{
    return ::std::rend(r.iterable);
}

template <typename T>
revertible<T> reverse(T&& iterable)
{
    return {iterable};
}

#endif//__KRANEWM__UTIL__GUARD__
