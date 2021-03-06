#ifndef __KRANEWM__PROCESS__GUARD__
#define __KRANEWM__PROCESS__GUARD__

#include "x-data/mask.hh"
#include "x-data/key.hh"

#include <deque>
#include <string>
#include <unordered_map>

// fwd decls
typedef class client_t* client_ptr_t;
typedef class processjump_t* processjump_ptr_t;

typedef struct processjump_t final
{
    processjump_t(client_ptr_t _target, client_ptr_t _prev)
      : target(_target),
        prev(_prev)
    {}

    inline bool
    operator==(const processjump_t& pj) const
    {
        return target == pj.target && prev == pj.prev;
    }

    client_ptr_t target;
    client_ptr_t prev;

}* processjump_ptr_t;


class processjumplist_t final
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


struct processshortcut_t final
{
    processshortcut_t(KeySym _keysym, unsigned _mask)
      : keysym(_keysym),
        mask(_mask)
    {}

    processshortcut_t(XKeyEvent event)
      : keysym(x_data::get_keysym(event.keycode)),
        mask(event.state)
    {}

    inline bool
    operator==(const processshortcut_t& ps) const
    {
        return ps.keysym == keysym && x_data::clean_mask(ps.mask) == x_data::clean_mask(mask);
    }

    KeySym keysym;
    unsigned mask;

};

namespace std
{
    template <>
    struct hash<processshortcut_t>
    {
        ::std::size_t
        operator()(const processshortcut_t& ps) const
        {
            return ps.keysym + 10000 * x_data::clean_mask(ps.mask);
        }
    };
}

typedef ::std::unordered_map<processshortcut_t, ::std::string> processbinds_t;

#endif//__KRANEWM__PROCESS__GUARD__
