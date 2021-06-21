#ifndef __PARTITION_H_GUARD__
#define __PARTITION_H_GUARD__

#include "../winsys/common.hh"
#include "../winsys/screen.hh"

typedef class Partition final
{
public:
    Partition(winsys::Screen screen, Index index)
        : m_screen(screen),
          m_index(index)
    {}

    ~Partition() {}

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

private:
    winsys::Screen m_screen;
    Index m_index;

}* Partition_ptr;

#endif//__PARTITION_H_GUARD__
