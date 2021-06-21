#ifndef __CONTEXT_H_GUARD__
#define __CONTEXT_H_GUARD__

#include "../winsys/common.hh"

#include <string>

typedef class Context final
{
public:
    Context(Index index, std::string name)
        : m_index(index),
          m_name(name)
    {}

    ~Context() {}

    Index
    index() const
    {
        return m_index;
    }

private:
    Index m_index;
    std::string m_name;

}* Context_ptr;

#endif//__CONTEXT_H_GUARD__
