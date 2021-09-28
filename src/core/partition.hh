#ifndef __PARTITION_H_GUARD__
#define __PARTITION_H_GUARD__

#include "../winsys/common.hh"
#include "../winsys/screen.hh"
#include "context.hh"

typedef class Partition final
{
public:
    Partition(winsys::Screen screen, Index index)
        : m_screen(screen),
          m_index(index),
          mp_context(nullptr)
    {}

    winsys::Screen&
    screen()
    {
        return m_screen;
    }

    winsys::Screen const&
    screen() const
    {
        return m_screen;
    }

    Index
    index() const
    {
        return m_index;
    }

    void
    set_context(Context_ptr context)
    {
        Util::assert(context != nullptr,
            "partition must contain a valid context");

        if (mp_context)
            mp_context->set_partition(nullptr);

        context->set_partition(this);
        mp_context = context;
    }

    Context_ptr
    context() const
    {
        return mp_context;
    }

    winsys::Region
    full_region() const
    {
        return m_screen.full_region();
    }

    winsys::Region
    placeable_region() const
    {
        return m_screen.placeable_region();
    }

    bool
    contains(winsys::Pos pos) const
    {
        return m_screen.contains(pos);
    }

    bool
    contains(winsys::Region region) const
    {
        return m_screen.contains(region);
    }

private:
    winsys::Screen m_screen;
    Index m_index;

    Context_ptr mp_context;

}* Partition_ptr;

#endif//__PARTITION_H_GUARD__
