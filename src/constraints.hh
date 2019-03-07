#ifndef __KRANEWM_CLIENT_MODEL_GUARD__
#define __KRANEWM_CLIENT_MODEL_GUARD__

#include "util.hh"
#include "x_wrapper/window.hh"


struct SizeConstraints
{
    SizeConstraints() = default;

    SizeConstraints(Size _base, Size _inc, Size _max, Size _min, Range<float> _aspect)
        : base(_base),
          inc(_inc),
          max(_max),
          min(_min),
          aspect(_aspect)
    {}

    inline bool operator==(const SizeConstraints& size_constraints) const
    {
        return size_constraints.base == base
            && size_constraints.inc == inc
            && size_constraints.max == max
            && size_constraints.min == min
            && size_constraints.aspect == aspect;
    }

    Size base;
    Size inc;
    Size max;
    Size min;
    Range<float> aspect;
};

#endif//__KRANEWM_CLIENT_MODEL_GUARD__
