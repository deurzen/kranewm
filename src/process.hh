#ifndef __KRANEWM__PROCESS__GUARD__
#define __KRANEWM__PROCESS__GUARD__

#include "client.hh"

typedef struct processjump_t
{
    processjump_t(client_ptr_t _target, client_ptr_t _prev)
        : target(_target), prev(_prev)
    {}

    inline bool operator==(const processjump_t& pj) const
    {
        return target == pj.target && prev == pj.prev;
    }

    client_ptr_t target;
    client_ptr_t prev;

}* processjump_ptr_t;


class processjumplist_t
{
public:
    processjumplist_t()
        : m_process_names({
              "qutebrowser",
          })
    {
        for (auto&& name : m_process_names)
            activate_process_name(name);
    }

    void add_process(client_ptr_t);
    void remove_process(client_ptr_t);

    void relayer_process(client_ptr_t);

    processjump_ptr_t find_process(client_ptr_t);
    processjump_ptr_t get_process(const ::std::string&);

private:
    void activate_process_name(const ::std::string&);
    void deactivate_process_name(const ::std::string&);

    ::std::vector<::std::string> m_process_names;
    ::std::unordered_map<::std::string, ::std::deque<processjump_ptr_t>> m_jumps;

};

#endif//__KRANEWM__PROCESS__GUARD__
