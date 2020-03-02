#ifndef __KRANEWM_COMMANDS_GUARD__
#define __KRANEWM_COMMANDS_GUARD__

#include "floating.hh"
#include "layout.hh"

#include <variant>
#include <optional>


// fwd decls
class client_model_t;
class windowstack_t;
class sidebar_t;
class processjumplist_t;
typedef class client_t* client_ptr_t;
typedef class command_t* command_ptr_t;


enum class commandop_t
{
    noop = 0,
    floatingconditional,
    quit,
    zoom,
    clientfloat, clientfullscreen, clientsticky, clientabove, clientbelow,
    clientcenter, clientsnapnorth, clientsnapeast, clientsnapsouth, clientsnapwest,
    clientkill,
    clientmoveforward, clientmovebackward,
    clientmarkset, clientmarkjump,
    clientmasterjump, clientstackjump, clientlastjump, clientpanejump, clientjumpindex,
    clientworkspace, clientnextworkspace, clientpreviousworkspace,
    clientcontext, clientnextcontext, clientpreviouscontext,
    clientgrow, clientshrink, clientmove, clientmovemouse, clientresizemouse,
    masterforward, masterbackward, stackforward, stackbackward, allforward, allbackward,
    clienticonify, clienticonifyindex, deiconifypop, clientdeiconifyindex,
    clientdisown, reclaimpop,
    profilesave, profileload,
    workspaceset, nextworkspace, previousworkspace,
    workspacemirror, workspacemfactor, workspacenmaster, workspacegapsize, workspacelayout,
    sidebarshow,
    contextset, nextcontext, previouscontext,
    focusforward, focusbackward,
    external,
};



typedef class command_t
{
public:
    command_t(bool internable)
        : m_internable(internable)
    {}

    virtual ~command_t() = default;

    virtual void execute() = 0;

    bool is_internable() const { return m_internable; }

protected:
    bool m_internable;

}* command_ptr_t;



class commandbind_t
{
public:
    typedef ::std::variant<int, float, layout_t, direction_t, ::std::string> vartype_t;
    typedef ::std::optional<vartype_t> argtype_t;

    commandbind_t() = default;

    commandbind_t(commandop_t operation)
        : m_operation(operation),
          m_argument(::std::nullopt)
    {}

    commandbind_t(commandop_t operation, int selector)
        : m_operation(operation),
          m_argument(selector)
    {}

    commandbind_t(commandop_t operation, float selector)
        : m_operation(operation),
          m_argument(selector)
    {}

    commandbind_t(commandop_t operation, layout_t selector)
        : m_operation(operation),
          m_argument(selector)
    {}

    commandbind_t(commandop_t operation, direction_t selector)
        : m_operation(operation),
          m_argument(selector)
    {}

    commandbind_t(::std::string command)
        : m_operation(commandop_t::external),
          m_argument(command)
    {}


    inline bool operator==(const commandbind_t& cb) const
    {
        return cb.get_op() == m_operation && cb.get_arg() == m_argument;
    }

    inline commandop_t get_op() const { return m_operation; }
    inline argtype_t get_arg() const { return m_argument; }

private:
    commandop_t m_operation;
    argtype_t m_argument;

};


namespace std
{
    template <>
    struct hash<commandbind_t>
    {
        ::std::size_t operator()(const commandbind_t& cb) const
        {
            size_t hashval = static_cast<int>(cb.get_op());
            if (cb.get_arg())
                hashval += ::std::hash<::commandbind_t::vartype_t>{}(*cb.get_arg());

            return hashval;
        }
    };
}



typedef class noopcommand_t : public command_t
{
public:
    explicit noopcommand_t()
        : command_t(false)
    {}

    void execute() {}

}* noopcommand_ptr_t;



typedef class floatingconditionalcommand_t : public command_t
{
public:
    explicit floatingconditionalcommand_t(client_model_t& clients,
        command_ptr_t truecommand, command_ptr_t falsecommand, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_truecommand(truecommand),
          m_falsecommand(falsecommand),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    command_ptr_t m_truecommand;
    command_ptr_t m_falsecommand;
    client_ptr_t m_client;

}* floatingconditionalcommand_ptr_t;



typedef class quitcommand_t : public command_t
{
public:
    explicit quitcommand_t(bool& running)
        : command_t(true),
          m_running(running)
    {}

    void execute() override;

private:
    bool& m_running;

}* quitcommand_ptr_t;



typedef class zoomcommand_t : public command_t
{
public:
    explicit zoomcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* zoomcommand_ptr_t;



typedef class clientfloatcommand_t : public command_t
{
public:
    explicit clientfloatcommand_t(client_model_t& clients, windowstack_t& windowstack,
        sidebar_t& sidebar, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_windowstack(windowstack),
          m_sidebar(sidebar),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    windowstack_t& m_windowstack;
    sidebar_t& m_sidebar;
    client_ptr_t m_client;

}* clientfloatcommand_ptr_t;



typedef class clientfullscreencommand_t : public command_t
{
public:
    explicit clientfullscreencommand_t(client_model_t& clients,
        sidebar_t& sidebar, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_sidebar(sidebar),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    sidebar_t& m_sidebar;
    client_ptr_t m_client;

}* clientfullscreencommand_ptr_t;



typedef class clientstickycommand_t : public command_t
{
public:
    explicit clientstickycommand_t(client_model_t& clients,
        sidebar_t& sidebar, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_sidebar(sidebar),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    sidebar_t& m_sidebar;
    client_ptr_t m_client;

}* clientstickycommand_ptr_t;



typedef class clientabovecommand_t : public command_t
{
public:
    explicit clientabovecommand_t(client_model_t& clients,
        sidebar_t& sidebar, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_sidebar(sidebar),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    sidebar_t& m_sidebar;
    client_ptr_t m_client;

}* clientabovecommand_ptr_t;



typedef class clientbelowcommand_t : public command_t
{
public:
    explicit clientbelowcommand_t(client_model_t& clients,
        sidebar_t& sidebar, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_sidebar(sidebar),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    sidebar_t& m_sidebar;
    client_ptr_t m_client;

}* clientbelowcommand_ptr_t;



typedef class clientcentercommand_t : public command_t
{
public:
    explicit clientcentercommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientcentercommand_ptr_t;



typedef class clientsnapnorthcommand_t : public command_t
{
public:
    explicit clientsnapnorthcommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientsnapnorthcommand_ptr_t;



typedef class clientsnapeastcommand_t : public command_t
{
public:
    explicit clientsnapeastcommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientsnapeastcommand_ptr_t;



typedef class clientsnapsouthcommand_t : public command_t
{
public:
    explicit clientsnapsouthcommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientsnapsouthcommand_ptr_t;



typedef class clientsnapwestcommand_t : public command_t
{
public:
    explicit clientsnapwestcommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientsnapwestcommand_ptr_t;



typedef class clientkillcommand_t : public command_t
{
public:
    explicit clientkillcommand_t(client_ptr_t client)
        : command_t(false),
          m_client(client)
    {}

    void execute() override;

private:
    client_ptr_t m_client;

}* clientkillcommand_ptr_t;



typedef class clientmoveforwardcommand_t : public command_t
{
public:
    explicit clientmoveforwardcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* clientmoveforwardcommand_ptr_t;



typedef class clientmovebackwardcommand_t : public command_t
{
public:
    explicit clientmovebackwardcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* clientmovebackwardcommand_ptr_t;



typedef class clientmarksetcommand_t : public command_t
{
public:
    explicit clientmarksetcommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientmarksetcommand_ptr_t;



typedef class clientmarkjumpcommand_t : public command_t
{
public:
    explicit clientmarkjumpcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* clientmarkjumpcommand_ptr_t;



typedef class clientmasterjumpcommand_t : public command_t
{
public:
    explicit clientmasterjumpcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* clientmasterjumpcommand_ptr_t;



typedef class clientstackjumpcommand_t : public command_t
{
public:
    explicit clientstackjumpcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* clientstackjumpcommand_ptr_t;



typedef class clientlastjumpcommand_t : public command_t
{
public:
    explicit clientlastjumpcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* clientlastjumpcommand_ptr_t;



typedef class clientpanejumpcommand_t : public command_t
{
public:
    explicit clientpanejumpcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* clientpanejumpcommand_ptr_t;



typedef class clientjumpindexcommand_t : public command_t
{
public:
    explicit clientjumpindexcommand_t(client_model_t& clients, unsigned index)
        : command_t(true),
          m_clients(clients),
          m_index(index)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    unsigned m_index;

}* clientjumpindexcommand_ptr_t;



typedef class clientworkspacecommand_t : public command_t
{
public:
    explicit clientworkspacecommand_t(client_model_t& clients,
        client_ptr_t client, unsigned number)
        : command_t(false),
          m_clients(clients),
          m_client(client),
          m_number(number)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;
    unsigned m_number;

}* clientworkspacecommand_ptr_t;



typedef class clientnextworkspacecommand_t : public command_t
{
public:
    explicit clientnextworkspacecommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientnextworkspacecommand_ptr_t;



typedef class clientpreviousworkspacecommand_t : public command_t
{
public:
    explicit clientpreviousworkspacecommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientpreviousworkspacecommand_ptr_t;



typedef class clientcontextcommand_t : public command_t
{
public:
    explicit clientcontextcommand_t(client_model_t& clients,
        client_ptr_t client, unsigned number)
        : command_t(false),
          m_clients(clients),
          m_client(client),
          m_number(number)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;
    unsigned m_number;

}* clientcontextcommand_ptr_t;



typedef class clientnextcontextcommand_t : public command_t
{
public:
    explicit clientnextcontextcommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientnextcontextcommand_ptr_t;



typedef class clientpreviouscontextcommand_t : public command_t
{
public:
    explicit clientpreviouscontextcommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientpreviouscontextcommand_ptr_t;



typedef class clientgrowcommand_t : public command_t
{
public:
    explicit clientgrowcommand_t(client_model_t& clients, client_ptr_t client,
        direction_t direction, unsigned increment = KB_RESIZE_INCREMENT)
        : command_t(false),
          m_clients(clients),
          m_client(client),
          m_direction(direction),
          m_increment(increment)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;
    direction_t m_direction;
    unsigned m_increment;

}* clientgrowcommand_ptr_t;



typedef class clientshrinkcommand_t : public command_t
{
public:
    explicit clientshrinkcommand_t(client_model_t& clients, client_ptr_t client,
        direction_t direction, unsigned increment = KB_RESIZE_INCREMENT)
        : command_t(false),
          m_clients(clients),
          m_client(client),
          m_direction(direction),
          m_increment(increment)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;
    direction_t m_direction;
    unsigned m_increment;

}* clientshrinkcommand_ptr_t;



typedef class clientmovecommand_t : public command_t
{
public:
    explicit clientmovecommand_t(client_model_t& clients, client_ptr_t client,
        direction_t direction, unsigned increment = KB_MOVE_INCREMENT)
        : command_t(false),
          m_clients(clients),
          m_client(client),
          m_direction(direction),
          m_increment(increment)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;
    direction_t m_direction;
    unsigned m_increment;

}* clientmovecommand_ptr_t;



typedef class clientmovemousecommand_t : public command_t
{
public:
    explicit clientmovemousecommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientmovemousecommand_ptr_t;



typedef class clientresizemousecommand_t : public command_t
{
public:
    explicit clientresizemousecommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientresizemousecommand_ptr_t;



typedef class masterforwardcommand_t : public command_t
{
public:
    explicit masterforwardcommand_t(client_model_t& clients, windowstack_t& windowstack)
        : command_t(true),
          m_clients(clients),
          m_windowstack(windowstack)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    windowstack_t& m_windowstack;

}* masterforwardcommand_ptr_t;



typedef class masterbackwardcommand_t : public command_t
{
public:
    explicit masterbackwardcommand_t(client_model_t& clients, windowstack_t& windowstack)
        : command_t(true),
          m_clients(clients),
          m_windowstack(windowstack)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    windowstack_t& m_windowstack;

}* masterbackwardcommand_ptr_t;



typedef class stackforwardcommand_t : public command_t
{
public:
    explicit stackforwardcommand_t(client_model_t& clients, windowstack_t& windowstack)
        : command_t(true),
          m_clients(clients),
          m_windowstack(windowstack)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    windowstack_t& m_windowstack;

}* stackforwardcommand_ptr_t;



typedef class stackbackwardcommand_t : public command_t
{
public:
    explicit stackbackwardcommand_t(client_model_t& clients, windowstack_t& windowstack)
        : command_t(true),
          m_clients(clients),
          m_windowstack(windowstack)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    windowstack_t& m_windowstack;

}* stackbackwardcommand_ptr_t;



typedef class allforwardcommand_t : public command_t
{
public:
    explicit allforwardcommand_t(client_model_t& clients, windowstack_t& windowstack)
        : command_t(true),
          m_clients(clients),
          m_windowstack(windowstack)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    windowstack_t& m_windowstack;

}* allforwardcommand_ptr_t;



typedef class allbackwardcommand_t : public command_t
{
public:
    explicit allbackwardcommand_t(client_model_t& clients, windowstack_t& windowstack)
        : command_t(true),
          m_clients(clients),
          m_windowstack(windowstack)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    windowstack_t& m_windowstack;

}* allbackwardcommand_ptr_t;



typedef class clienticonifycommand_t : public command_t
{
public:
    explicit clienticonifycommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clienticonifycommand_ptr_t;



typedef class clienticonifyindexcommand_t : public command_t
{
public:
    explicit clienticonifyindexcommand_t(client_model_t& clients, unsigned index)
        : command_t(true),
          m_clients(clients),
          m_index(index)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    unsigned m_index;

}* clienticonifyindexcommand_ptr_t;



typedef class deiconifypopcommand_t : public command_t
{
public:
    explicit deiconifypopcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* deiconifypopcommand_ptr_t;



typedef class clientdeiconifyindexcommand_t : public command_t
{
public:
    explicit clientdeiconifyindexcommand_t(client_model_t& clients, unsigned index)
        : command_t(true),
          m_clients(clients),
          m_index(index)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    unsigned m_index;

}* clientdeiconifyindexcommand_ptr_t;



typedef class clientdisowncommand_t : public command_t
{
public:
    explicit clientdisowncommand_t(client_model_t& clients, client_ptr_t client)
        : command_t(false),
          m_clients(clients),
          m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientdisowncommand_ptr_t;



typedef class reclaimpopcommand_t : public command_t
{
public:
    explicit reclaimpopcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* reclaimpopcommand_ptr_t;



typedef class profilesavecommand_t : public command_t
{
public:
    explicit profilesavecommand_t(client_model_t& clients, unsigned number)
        : command_t(true),
          m_clients(clients),
          m_number(number)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    unsigned m_number;

}* profilesavecommand_ptr_t;



typedef class profileloadcommand_t : public command_t
{
public:
    explicit profileloadcommand_t(client_model_t& clients,
        sidebar_t& sidebar, unsigned number)
        : command_t(true),
          m_clients(clients),
          m_sidebar(sidebar),
          m_number(number)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    sidebar_t& m_sidebar;
    unsigned m_number;

}* profileloadcommand_ptr_t;



typedef class workspacesetcommand_t : public command_t
{
public:
    explicit workspacesetcommand_t(client_model_t& clients, unsigned number)
        : command_t(true),
          m_clients(clients),
          m_number(number)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    unsigned m_number;

}* workspacesetcommand_ptr_t;



typedef class nextworkspacecommand_t : public command_t
{
public:
    explicit nextworkspacecommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* nextworkspacecommand_ptr_t;



typedef class previousworkspacecommand_t : public command_t
{
public:
    explicit previousworkspacecommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* previousworkspacecommand_ptr_t;



typedef class workspacemirrorcommand_t : public command_t
{
public:
    explicit workspacemirrorcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* workspacemirrorcommand_ptr_t;



typedef class workspacemfactorcommand_t : public command_t
{
public:
    explicit workspacemfactorcommand_t(client_model_t& clients, float delta = 0.05)
        : command_t(true),
          m_clients(clients),
          m_delta(delta)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    float m_delta;

}* workspacemfactorcommand_ptr_t;



typedef class workspacenmastercommand_t : public command_t
{
public:
    explicit workspacenmastercommand_t(client_model_t& clients, int delta = 1)
        : command_t(true),
          m_clients(clients),
          m_delta(delta)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    int m_delta;

}* workspacenmastercommand_ptr_t;



typedef class workspacegapsizecommand_t : public command_t
{
public:
    explicit workspacegapsizecommand_t(client_model_t& clients, int delta = 1)
        : command_t(true),
          m_clients(clients),
          m_delta(delta)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    int m_delta;

}* workspacegapsizecommand_ptr_t;



typedef class workspacelayoutcommand_t : public command_t
{
public:
    explicit workspacelayoutcommand_t(client_model_t& clients, windowstack_t& windowstack,
        sidebar_t& sidebar, layout_t layout)
        : command_t(true),
          m_clients(clients),
          m_windowstack(windowstack),
          m_sidebar(sidebar),
          m_layout(layout)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    windowstack_t& m_windowstack;
    sidebar_t& m_sidebar;
    layout_t m_layout;

}* workspacelayoutcommand_ptr_t;



typedef class sidebarshowcommand_t : public command_t
{
public:
    explicit sidebarshowcommand_t(client_model_t& clients, sidebar_t& sidebar)
        : command_t(true),
          m_clients(clients),
          m_sidebar(sidebar)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    sidebar_t& m_sidebar;

}* sidebarshowcommand_ptr_t;



typedef class contextsetcommand_t : public command_t
{
public:
    explicit contextsetcommand_t(client_model_t& clients, unsigned number)
        : command_t(true),
          m_clients(clients),
          m_number(number)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    unsigned m_number;

}* contextsetcommand_ptr_t;



typedef class nextcontextcommand_t : public command_t
{
public:
    explicit nextcontextcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* nextcontextcommand_ptr_t;



typedef class previouscontextcommand_t : public command_t
{
public:
    explicit previouscontextcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* previouscontextcommand_ptr_t;



typedef class focusforwardcommand_t : public command_t
{
public:
    explicit focusforwardcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* focusforwardcommand_ptr_t;



typedef class focusbackwardcommand_t : public command_t
{
public:
    explicit focusbackwardcommand_t(client_model_t& clients)
        : command_t(true),
          m_clients(clients)
    {}

    void execute() override;

private:
    client_model_t& m_clients;

}* focusbackwardcommand_ptr_t;



typedef class externalcommand_t : public command_t
{
public:
    explicit externalcommand_t(::std::string&& command)
        : command_t(true),
          m_command(command)
    {}

    void execute() override;

private:
    ::std::string m_command;

}* externalcommand_ptr_t;



#endif//__KRANEWM_COMMANDS_GUARD__
