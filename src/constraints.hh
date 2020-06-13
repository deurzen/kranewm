#ifndef __KRANEWM_CONSTRAINTS_GUARD__
#define __KRANEWM_CONSTRAINTS_GUARD__

#include "util.hh"

#include "x-data/window.hh"


struct sizeconstraints_t final
{
    sizeconstraints_t() = default;

    sizeconstraints_t(dim_t _base, dim_t _inc, dim_t _max, dim_t _min, range_t<float> _aspect)
      : base(_base),
        inc(_inc),
        max(_max),
        min(_min),
        aspect(_aspect)
    {}

    inline bool
    operator==(const sizeconstraints_t& size_constraints) const
    {
        return size_constraints.base == base && size_constraints.inc == inc
            && size_constraints.max == max && size_constraints.min == min
            && size_constraints.aspect == aspect;
    }

    inline bool
    is_fixed() const
    {
        return !(max == dim_t{0, 0}) && max == min;
    }

    void apply(pos_t&, dim_t&) const;

    dim_t base;
    dim_t inc;
    dim_t max;
    dim_t min;
    range_t<float> aspect;
};

#endif//__KRANEWM_CONSTRAINTS_GUARD__
