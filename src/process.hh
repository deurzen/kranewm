#ifndef __KRANEWM__PROCESS__GUARD__
#define __KRANEWM__PROCESS__GUARD__

#include "client.hh"
#include "x-wrapper/key.hh"

#include <unordered_map>


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
    processjumplist_t() = default;

    void activate_process_name(const ::std::string&);
    void deactivate_process_name(const ::std::string&);

    void add_process(client_ptr_t);
    void remove_process(client_ptr_t);

    void relayer_process(client_ptr_t);

    processjump_ptr_t find_process(client_ptr_t);
    processjump_ptr_t get_process(const ::std::string&);

private:
    ::std::unordered_map<::std::string, ::std::deque<processjump_ptr_t>> m_jumps;

};


struct processshortcut_t
{
    processshortcut_t(KeySym _keysym, unsigned _mask)
        : keysym(_keysym),
          mask(_mask)
    {}

    processshortcut_t(XKeyEvent event)
        : keysym(x_wrapper::get_keysym(event.keycode)),
          mask(event.state)
    {}

    inline bool operator==(const processshortcut_t& ps) const
    {
        return ps.keysym == keysym && ps.mask == mask;
    }

    KeySym keysym;
    unsigned mask;
};

namespace std
{
    template <>
    struct hash<processshortcut_t>
    {
        std::size_t operator()(const processshortcut_t& ps) const
        {
            return ps.keysym + 10000 * ps.mask;
        }
    };
}

typedef ::std::unordered_map<processshortcut_t, ::std::string> processbinds_t;

#endif//__KRANEWM__PROCESS__GUARD__
