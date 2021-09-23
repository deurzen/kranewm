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

    const winsys::Screen&
    const_screen() const
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
            "partition must contain valid context");

        if (mp_context)
            mp_context->set_partition(nullptr);

        context->set_partition(this);
        mp_context = context;
    }

    Context_ptr
    context()
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

private:
    winsys::Screen m_screen;
    Index m_index;

    Context_ptr mp_context;

}* Partition_ptr;

#endif//__PARTITION_H_GUARD__
