#ifndef __KRANEWM__UTIL__GUARD__
#define __KRANEWM__UTIL__GUARD__

#include <string>


void die(const ::std::string&);


template <typename T>
struct Range
{
    static bool contains(T min, T max, T i)
    {
        return min <= i  && i <= max;
    }

    Range(T _min = 0, T _max = 0)
      : min(_min),
        max(_max)
    {}

    inline bool operator==(const Range<T>& range) const
    {
        return min == range.min && max == range.max;
    }

    T min;
    T max;
};

#endif//__KRANEWM__UTIL__GUARD__
