#ifndef __KRANEWM_GROUP_GUARD__
#define __KRANEWM_GROUP_GUARD__

#include <unordered_map>
#include <vector>

extern "C" {
#include <X11/Xlib.h>
}


typedef class client_t* client_ptr_t;

typedef class group_t
{
public:
    group_t() = default;

    void add_client(client_ptr_t);
    void remove_client(client_ptr_t);

    void diffuse_event(XKeyEvent*) const;

private:
    ::std::vector<client_ptr_t> m_clients;

}* group_ptr_t;


class grouphandler_t
{
public:
    grouphandler_t()
      : m_groups(5, new group_t{})
    {}

    ~grouphandler_t()
    {
        for (auto group : m_groups)
            delete group;
    }

    void group_client(client_ptr_t, size_t);
    void degroup_client(client_ptr_t);

    bool is_grouped(client_ptr_t) const;

private:
    ::std::vector<group_ptr_t> m_groups;
    ::std::unordered_map<client_ptr_t, group_ptr_t> m_clientgroups;

};

#endif//__KRANEWM_GROUP_GUARD__
