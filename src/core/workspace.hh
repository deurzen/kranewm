#ifndef __WORKSPACE_H_GUARD__
#define __WORKSPACE_H_GUARD__

#include "../winsys/geometry.hh"
#include "../winsys/input.hh"
#include "../winsys/window.hh"
#include "../winsys/util.hh"
#include "client.hh"
#include "cycle.hh"
#include "cycle.t.hh"
#include "layout.hh"
#include "placement.hh"

#include <cstdlib>
#include <deque>
#include <string>
#include <vector>

class Buffer final
{
public:
    enum class BufferKind
    {
        Move, Resize
    };

    Buffer(BufferKind kind)
        : m_kind(kind),
          m_client(nullptr),
          m_grip(std::nullopt),
          m_grip_pos(std::nullopt),
          m_client_region(std::nullopt)
    {}

    bool is_occupied() const;

    Client_ptr client() const;
    std::optional<winsys::Grip> grip() const;
    std::optional<winsys::Pos> grip_pos() const;
    std::optional<winsys::Region> client_region() const;

    void set_grip_pos(winsys::Pos);
    void set_client_region(winsys::Region);

    void set(Client_ptr, winsys::Grip, winsys::Pos, winsys::Region);
    void unset();

private:
    BufferKind m_kind;
    Client_ptr m_client;
    std::optional<winsys::Grip> m_grip;
    std::optional<winsys::Pos> m_grip_pos;
    std::optional<winsys::Region> m_client_region;

};

class Scratchpad final
{
public:
    Scratchpad() {}

private:

};

typedef class Workspace final
{
public:
    struct ClientSelector
    {
        enum class SelectionCriterium {
            AtFirst,
            AtLast,
            AtMain,
            AtIndex
        };

        ClientSelector(const SelectionCriterium criterium)
            : m_index(std::nullopt)
        {
            switch (criterium) {
            case SelectionCriterium::AtFirst: m_tag = ClientSelectorTag::AtFirst; return;
            case SelectionCriterium::AtLast:  m_tag = ClientSelectorTag::AtLast;  return;
            case SelectionCriterium::AtMain:  m_tag = ClientSelectorTag::AtMain;  return;
            default: return;
            }
        }

        ClientSelector(const std::size_t index)
            : m_tag(ClientSelectorTag::AtIndex),
              m_index(index)
        {}

        ~ClientSelector() = default;

        SelectionCriterium
        criterium() const
        {
            switch (m_tag) {
            case ClientSelectorTag::AtFirst: return SelectionCriterium::AtFirst;
            case ClientSelectorTag::AtLast:  return SelectionCriterium::AtLast;
            case ClientSelectorTag::AtMain:  return SelectionCriterium::AtMain;
            case ClientSelectorTag::AtIndex: return SelectionCriterium::AtIndex;
            default: Util::die("no associated criterium");
            }

            return {};
        }

        std::size_t
        index() const
        {
            return *m_index;
        }

    private:
        enum class ClientSelectorTag {
            AtFirst,
            AtLast,
            AtMain,
            AtIndex
        };

        ClientSelectorTag m_tag;
        std::optional<std::size_t> m_index;

    };

    Workspace(std::size_t index, std::string name, Context_ptr context)
        : m_index(index),
          m_name(name),
          m_layout_handler({}),
          mp_context(context),
          mp_active(nullptr),
          m_clients({}, true),
          m_icons({}, true),
          m_disowned({}, true)
    {}

    bool empty() const;
    bool contains(Client_ptr) const;

    bool layout_is_free() const;
    bool layout_has_margin() const;
    bool layout_has_gap() const;
    bool layout_is_persistent() const;
    bool layout_is_single() const;
    bool layout_wraps() const;

    std::size_t size() const;
    std::size_t length() const;

    Context_ptr context() const;

    Index index() const;
    std::string const& name() const;
    std::string identifier() const;
    Client_ptr active() const;

    std::deque<Client_ptr> const& clients() const;
    std::vector<Client_ptr> stack_after_focus() const;

    Client_ptr next_client() const;
    Client_ptr prev_client() const;

    std::optional<Client_ptr> find_client(ClientSelector const&) const;

    void cycle(winsys::Direction);
    void drag(winsys::Direction);
    void reverse();
    void rotate(winsys::Direction);
    void shuffle_main(winsys::Direction);
    void shuffle_stack(winsys::Direction);

    void activate_client(Client_ptr);

    void add_client(Client_ptr);
    void remove_client(Client_ptr);
    void replace_client(Client_ptr, Client_ptr);

    void client_to_icon(Client_ptr);
    void icon_to_client(Client_ptr);
    void add_icon(Client_ptr);
    void remove_icon(Client_ptr);
    std::optional<Client_ptr> pop_icon();

    void client_to_disowned(Client_ptr);
    void disowned_to_client(Client_ptr);
    void add_disowned(Client_ptr);
    void remove_disowned(Client_ptr);

    void toggle_layout_data();
    void cycle_layout_data(winsys::Direction);
    void copy_data_from_prev_layout();

    void change_gap_size(Util::Change<int>);
    void change_main_count(Util::Change<int>);
    void change_main_factor(Util::Change<float>);
    void change_margin(Util::Change<int>);
    void change_margin(winsys::Edge, Util::Change<int>);
    void reset_gap_size();
    void reset_margin();
    void reset_layout_data();

    void save_layout(std::size_t) const;
    void load_layout(std::size_t);

    void toggle_layout();
    void set_layout(LayoutHandler::LayoutKind);
    std::vector<Placement> arrange(winsys::Region) const;

    std::deque<Client_ptr>::iterator
    begin()
    {
        return m_clients.begin();
    }

    std::deque<Client_ptr>::const_iterator
    begin() const
    {
        return m_clients.begin();
    }

    std::deque<Client_ptr>::const_iterator
    cbegin() const
    {
        return m_clients.cbegin();
    }

    std::deque<Client_ptr>::iterator
    end()
    {
        return m_clients.end();
    }

    std::deque<Client_ptr>::const_iterator
    end() const
    {
        return m_clients.end();
    }

    std::deque<Client_ptr>::const_iterator
    cend() const
    {
        return m_clients.cend();
    }

    Client_ptr
    operator[](std::size_t i)
    {
        return m_clients[i];
    }

    Client_ptr
    operator[](std::size_t i) const
    {
        return m_clients[i];
    }

private:
    std::size_t m_index;
    std::string m_name;

    LayoutHandler m_layout_handler;

    Context_ptr mp_context;

    Client_ptr mp_active;
    Cycle<Client_ptr> m_clients;
    Cycle<Client_ptr> m_icons;
    Cycle<Client_ptr> m_disowned;

}* Workspace_ptr;

#endif//__WORKSPACE_H_GUARD__
