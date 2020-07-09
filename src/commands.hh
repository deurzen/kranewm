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
typedef class user_workspace_t* user_workspace_ptr_t;
typedef class context_t* context_ptr_t;
typedef class command_t* command_ptr_t;


enum class commandop_t
{
    noop = 0,
    floatingconditional,
    quit,
    zoom,
    clientfloat, clientfullscreen, clientinwindow, clientsticky, clientabove, clientbelow,
    clientcenter, clientsnapmovenorth, clientsnapmoveeast, clientsnapmovesouth, clientsnapmovewest,
    clientsnapresizenorth, clientsnapresizeeast, clientsnapresizesouth, clientsnapresizewest,
    clientkill, workspacekill, contextkill,
    clientmoveforward, clientmovebackward,
    clientmarkset, clientmarkjump,
    clientmasterjump, clientstackjump, clientlastjump, clientpanejump, clientjumpindex,
    clientworkspace, clientnextworkspace, clientpreviousworkspace,
    clientcontext, clientnextcontext, clientpreviouscontext,
    clientgrow, clientshrink, clientinflate, clientdeflate,
    clientmove, clientmovemouse, clientresizemouse,
    masterforward, masterbackward, stackforward, stackbackward, allforward, allbackward,
    clienticonify, clienticonifyindex, deiconifypop, clientdeiconifyindex,
    clientdisown, reclaimpop,
    profilesave, profileload,
    workspaceset, nextworkspace, previousworkspace,
    workspacemirror, workspacemfactor, workspacenmaster, workspacegapsize,
    workspacelayout, workspacelayoutpreservedim,
    sidebarshow, sidebarshowall,
    contextset, nextcontext, previouscontext,
    focusforward, focusbackward,
    external,
};



typedef class command_t
{
public:
    command_t(bool internable = false)
      : m_internable(internable)
    {}

    virtual ~command_t() = default;

    virtual void execute() = 0;

    bool is_internable() const { return m_internable; }

protected:
    bool m_internable;

}* command_ptr_t;


typedef class commandbind_t* commandbind_ptr_t;
class commandbind_t final
{
public:
    typedef ::std::variant<int, float, layout_t, direction_t, ::std::string> vartype_t;
    typedef ::std::optional<vartype_t> argtype_t;
    typedef ::std::optional<::std::pair<commandbind_ptr_t, commandbind_ptr_t>> comptype_t;

    commandbind_t()
      : m_operation(commandop_t::noop),
        m_argument(::std::nullopt),
        m_composite(::std::nullopt)
    {}

    commandbind_t(commandop_t operation)
      : m_operation(operation),
        m_argument(::std::nullopt),
        m_composite(::std::nullopt)
    {}

    commandbind_t(commandop_t operation, int selector)
      : m_operation(operation),
        m_argument(selector),
        m_composite(::std::nullopt)
    {}

    commandbind_t(commandop_t operation, float selector)
      : m_operation(operation),
        m_argument(selector),
        m_composite(::std::nullopt)
    {}

    commandbind_t(commandop_t operation, layout_t selector)
      : m_operation(operation),
        m_argument(selector),
        m_composite(::std::nullopt)
    {}

    commandbind_t(commandop_t operation, direction_t selector)
      : m_operation(operation),
        m_argument(selector),
        m_composite(::std::nullopt)
    {}

    commandbind_t(::std::string command)
      : m_operation(commandop_t::external),
        m_argument(command),
        m_composite(::std::nullopt)
    {}

    commandbind_t(commandop_t operation,
      commandbind_ptr_t commandbind1, commandbind_ptr_t commandbind2)
      : m_operation(operation),
        m_argument(::std::nullopt),
        m_composite({commandbind1, commandbind2})
    {}


    inline bool operator==(const commandbind_t& cb) const
    {
        return cb.get_op() == m_operation && cb.get_arg() == m_argument;
    }

    inline commandop_t get_op() const { return m_operation; }
    inline argtype_t get_arg() const { return m_argument; }

    inline commandbind_ptr_t get_comp1() const { return m_composite->first; }
    inline commandbind_ptr_t get_comp2() const { return m_composite->second; }

private:
    commandop_t m_operation;

    argtype_t m_argument;
    comptype_t m_composite;

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



typedef class noopcommand_t final : public command_t
{
public:
    explicit noopcommand_t()
      : command_t(false)
    {}

    void execute() {}

}* noopcommand_ptr_t;



typedef class floatingconditionalcommand_t final : public command_t
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

    ~floatingconditionalcommand_t()
    {
        delete m_truecommand;
        delete m_falsecommand;
    }

    void execute() override;

private:
    client_model_t& m_clients;
    command_ptr_t m_truecommand;
    command_ptr_t m_falsecommand;
    client_ptr_t m_client;

}* floatingconditionalcommand_ptr_t;



typedef class quitcommand_t final : public command_t
{
public:
    explicit quitcommand_t(bool* running)
      : command_t(true),
        m_running(running)
    {}

    void execute() override;

private:
    bool* m_running;

}* quitcommand_ptr_t;



typedef class zoomcommand_t final : public command_t
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



typedef class clientfloatcommand_t final : public command_t
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



typedef class clientfullscreencommand_t final : public command_t
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



typedef class clientinwindowcommand_t final : public command_t
{
public:
    explicit clientinwindowcommand_t(client_model_t& clients,
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

}* clientinwindowcommand_ptr_t;



typedef class clientstickycommand_t final : public command_t
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



typedef class clientabovecommand_t final : public command_t
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



typedef class clientbelowcommand_t final : public command_t
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



typedef class clientcentercommand_t final : public command_t
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



typedef class clientsnapmovenorthcommand_t final : public command_t
{
public:
    explicit clientsnapmovenorthcommand_t(client_model_t& clients, client_ptr_t client)
      : command_t(false),
        m_clients(clients),
        m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientsnapmovenorthcommand_ptr_t;



typedef class clientsnapmoveeastcommand_t final : public command_t
{
public:
    explicit clientsnapmoveeastcommand_t(client_model_t& clients, client_ptr_t client)
      : command_t(false),
        m_clients(clients),
        m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientsnapmoveeastcommand_ptr_t;



typedef class clientsnapmovesouthcommand_t final : public command_t
{
public:
    explicit clientsnapmovesouthcommand_t(client_model_t& clients, client_ptr_t client)
      : command_t(false),
        m_clients(clients),
        m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientsnapmovesouthcommand_ptr_t;



typedef class clientsnapmovewestcommand_t final : public command_t
{
public:
    explicit clientsnapmovewestcommand_t(client_model_t& clients, client_ptr_t client)
      : command_t(false),
        m_clients(clients),
        m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientsnapmovewestcommand_ptr_t;



typedef class clientsnapresizenorthcommand_t final : public command_t
{
public:
    explicit clientsnapresizenorthcommand_t(client_model_t& clients, client_ptr_t client)
      : command_t(false),
        m_clients(clients),
        m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientsnapresizenorthcommand_ptr_t;



typedef class clientsnapresizeeastcommand_t final : public command_t
{
public:
    explicit clientsnapresizeeastcommand_t(client_model_t& clients, client_ptr_t client)
      : command_t(false),
        m_clients(clients),
        m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientsnapresizeeastcommand_ptr_t;



typedef class clientsnapresizesouthcommand_t final : public command_t
{
public:
    explicit clientsnapresizesouthcommand_t(client_model_t& clients, client_ptr_t client)
      : command_t(false),
        m_clients(clients),
        m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientsnapresizesouthcommand_ptr_t;



typedef class clientsnapresizewestcommand_t final : public command_t
{
public:
    explicit clientsnapresizewestcommand_t(client_model_t& clients, client_ptr_t client)
      : command_t(false),
        m_clients(clients),
        m_client(client)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;

}* clientsnapresizewestcommand_ptr_t;



typedef class clientkillcommand_t final : public command_t
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



typedef class workspacekillcommand_t final : public command_t
{
public:
    explicit workspacekillcommand_t(user_workspace_ptr_t workspace)
      : command_t(false),
        m_workspace(workspace)
    {}

    void execute() override;

private:
    user_workspace_ptr_t m_workspace;

}* workspacekillcommand_ptr_t;



typedef class contextkillcommand_t final : public command_t
{
public:
    explicit contextkillcommand_t(context_ptr_t context)
      : command_t(false),
        m_context(context)
    {}

    void execute() override;

private:
    context_ptr_t m_context;

}* contextkillcommand_ptr_t;



typedef class clientmoveforwardcommand_t final : public command_t
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



typedef class clientmovebackwardcommand_t final : public command_t
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



typedef class clientmarksetcommand_t final : public command_t
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



typedef class clientmarkjumpcommand_t final : public command_t
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



typedef class clientmasterjumpcommand_t final : public command_t
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



typedef class clientstackjumpcommand_t final : public command_t
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



typedef class clientlastjumpcommand_t final : public command_t
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



typedef class clientpanejumpcommand_t final : public command_t
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



typedef class clientjumpindexcommand_t final : public command_t
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



typedef class clientworkspacecommand_t final : public command_t
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



typedef class clientnextworkspacecommand_t final : public command_t
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



typedef class clientpreviousworkspacecommand_t final : public command_t
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



typedef class clientcontextcommand_t final : public command_t
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



typedef class clientnextcontextcommand_t final : public command_t
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



typedef class clientpreviouscontextcommand_t final : public command_t
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



typedef class clientgrowcommand_t final : public command_t
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



typedef class clientshrinkcommand_t final : public command_t
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



typedef class clientinflatecommand_t final : public command_t
{
public:
    explicit clientinflatecommand_t(client_model_t& clients, client_ptr_t client,
        unsigned increment = 5)
      : command_t(false),
        m_clients(clients),
        m_client(client),
        m_increment(increment)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;
    unsigned m_increment;

}* clientinflatecommand_ptr_t;



typedef class clientdeflatecommand_t final : public command_t
{
public:
    explicit clientdeflatecommand_t(client_model_t& clients, client_ptr_t client,
        unsigned increment = 5)
      : command_t(false),
        m_clients(clients),
        m_client(client),
        m_increment(increment)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    client_ptr_t m_client;
    unsigned m_increment;

}* clientdeflatecommand_ptr_t;



typedef class clientmovecommand_t final : public command_t
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



typedef class clientmovemousecommand_t final : public command_t
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



typedef class clientresizemousecommand_t final : public command_t
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



typedef class masterforwardcommand_t final : public command_t
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



typedef class masterbackwardcommand_t final : public command_t
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



typedef class stackforwardcommand_t final : public command_t
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



typedef class stackbackwardcommand_t final : public command_t
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



typedef class allforwardcommand_t final : public command_t
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



typedef class allbackwardcommand_t final : public command_t
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



typedef class clienticonifycommand_t final : public command_t
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



typedef class clienticonifyindexcommand_t final : public command_t
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



typedef class deiconifypopcommand_t final : public command_t
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



typedef class clientdeiconifyindexcommand_t final : public command_t
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



typedef class clientdisowncommand_t final : public command_t
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



typedef class reclaimpopcommand_t final : public command_t
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



typedef class profilesavecommand_t final : public command_t
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



typedef class profileloadcommand_t final : public command_t
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



typedef class workspacesetcommand_t final : public command_t
{
public:
    explicit workspacesetcommand_t(client_model_t& clients, sidebar_t& sidebar, unsigned number)
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

}* workspacesetcommand_ptr_t;



typedef class nextworkspacecommand_t final : public command_t
{
public:
    explicit nextworkspacecommand_t(client_model_t& clients, sidebar_t& sidebar)
      : command_t(true),
        m_clients(clients),
        m_sidebar(sidebar)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    sidebar_t& m_sidebar;

}* nextworkspacecommand_ptr_t;



typedef class previousworkspacecommand_t final : public command_t
{
public:
    explicit previousworkspacecommand_t(client_model_t& clients, sidebar_t& sidebar)
      : command_t(true),
        m_clients(clients),
        m_sidebar(sidebar)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    sidebar_t& m_sidebar;

}* previousworkspacecommand_ptr_t;



typedef class workspacemirrorcommand_t final : public command_t
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



typedef class workspacemfactorcommand_t final : public command_t
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



typedef class workspacenmastercommand_t final : public command_t
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



typedef class workspacegapsizecommand_t final : public command_t
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



typedef class workspacelayoutcommand_t final : public command_t
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



typedef class workspacelayoutpreservedimcommand_t final : public command_t
{
public:
    explicit workspacelayoutpreservedimcommand_t(client_model_t& clients, windowstack_t& windowstack,
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

}* workspacelayoutpreservedimcommand_ptr_t;



typedef class sidebarshowcommand_t final : public command_t
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



typedef class sidebarshowallcommand_t final : public command_t
{
public:
    explicit sidebarshowallcommand_t(client_model_t& clients, sidebar_t& sidebar)
      : command_t(true),
        m_clients(clients),
        m_sidebar(sidebar)
    {}

    void execute() override;

private:
    client_model_t& m_clients;
    sidebar_t& m_sidebar;

}* sidebarshowallcommand_ptr_t;



typedef class contextsetcommand_t final : public command_t
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



typedef class nextcontextcommand_t final : public command_t
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



typedef class previouscontextcommand_t final : public command_t
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



typedef class focusforwardcommand_t final : public command_t
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



typedef class focusbackwardcommand_t final : public command_t
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



typedef class externalcommand_t final : public command_t
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
