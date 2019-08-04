#ifndef __KRANEWM_FOCUS_CYCLE_GUARD__
#define __KRANEWM_FOCUS_CYCLE_GUARD__

#include "client.hh"
#include <deque>


class focus_cycle_t
{
    static const ::std::size_t MAX_STACK_SIZE = 50;

    typedef ::std::deque<client_ptr_t>::iterator fg_it;
    typedef ::std::deque<client_ptr_t>::reverse_iterator fg_rit;
    typedef ::std::deque<client_ptr_t>::size_type fg_sz;

    class focus_stack_t
    {
    public:
        focus_stack_t()
            : max_stack_size(MAX_STACK_SIZE) {}

        void push(client_ptr_t);
        client_ptr_t pop(client_ptr_t);

        bool empty() const;
        ::std::deque<client_ptr_t>::size_type size() const;

        void erase(client_ptr_t);

    private:
        const ::std::size_t max_stack_size;
        ::std::deque<client_ptr_t> stack;

    };

public:
    focus_cycle_t()
        : m_has_focus(false) {}

    void add(client_ptr_t);
    void remove(client_ptr_t);

    bool is_empty() const;
    ::std::deque<client_ptr_t>::size_type size() const;
    bool contains(client_ptr_t) const;

    const client_ptr_t get() const;
    const ::std::deque<client_ptr_t>& get_all() const;

    bool set(client_ptr_t, bool = false);
    bool set(fg_sz);
    void unset();

    ::std::size_t index_of(client_ptr_t);

    bool next_focus();
    bool prev_focus();

    void rotate_group_forward(::std::size_t, ::std::size_t);
    void rotate_group_backward(::std::size_t, ::std::size_t);

    ::std::pair<client_ptr_t, client_ptr_t> move_focus_forward();
    ::std::pair<client_ptr_t, client_ptr_t> move_focus_backward();

    ::std::pair<client_ptr_t, client_ptr_t> zoom();

private:
    fg_it m_focus;
    bool m_has_focus;

    ::std::deque<client_ptr_t> m_clients;
    focus_stack_t m_focusstack;

};

#endif//__KRANEWM_FOCUS_CYCLE_GUARD__
