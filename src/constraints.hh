#ifndef __KRANEWM_CONSTRAINTS_GUARD__
#define __KRANEWM_CONSTRAINTS_GUARD__

#include "util.hh"
#include "x_wrapper/window.hh"


struct sizeconstraints_t
{
    sizeconstraints_t() = default;

    sizeconstraints_t(Size _base, Size _inc, Size _max, Size _min, Range<float> _aspect)
        : base(_base), inc(_inc), max(_max), min(_min), aspect(_aspect) {}

    inline bool operator==(const sizeconstraints_t& size_constraints) const
    {
        return size_constraints.base == base && size_constraints.inc == inc
            && size_constraints.max == max && size_constraints.min == min
            && size_constraints.aspect == aspect;
    }

    inline bool is_fixed() const { return !(max == Size{0, 0}) && max == min; }

    void apply(Pos&, Size&) const;

    Size base;
    Size inc;
    Size max;
    Size min;
    Range<float> aspect;
};

#endif//__KRANEWM_CONSTRAINTS_GUARD__
