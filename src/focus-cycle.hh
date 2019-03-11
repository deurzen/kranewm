#ifndef __KRANEWM_FOCUS_CYCLE_GUARD__
#define __KRANEWM_FOCUS_CYCLE_GUARD__
//TODO anything to do with place manipulation within cycle

#include "client.hh"
#include <deque>


class focus_cycle
{
private:
    typedef ::std::deque<client_ptr_t>::iterator  fg_it;
    typedef ::std::deque<client_ptr_t>::reverse_iterator  fg_rit;
    typedef ::std::deque<client_ptr_t>::size_type fg_sz;


public:
    void add(client_ptr_t);
    void remove(client_ptr_t);

    bool is_empty() const;
    ::std::deque<client_ptr_t>::size_type size() const;
    bool contains(client_ptr_t) const;

    const client_ptr_t get() const;
    const ::std::deque<client_ptr_t>& get_all() const;

    bool set(client_ptr_t);
    bool set(fg_sz);
    void unset();

    unsigned index_of(client_ptr_t);

    bool next_focus();
    bool prev_focus();

    void rotate_group_forward(unsigned, unsigned);
    void rotate_group_backward(unsigned, unsigned);

    ::std::pair<client_ptr_t, client_ptr_t> move_focused_client_forward();
    ::std::pair<client_ptr_t, client_ptr_t> move_focused_client_backward();

    ::std::pair<client_ptr_t, client_ptr_t> zoom();

private:
    bool m_has_focus = false;
    client_ptr_t m_previously_focused_client = nullptr;
    fg_it m_focus;
    ::std::deque<client_ptr_t> m_clients;

};

#endif//__KRANEWM_FOCUS_CYCLE_GUARD__
