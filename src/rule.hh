#ifndef __KRANEWM__RULE__GUARD__
#define __KRANEWM__RULE__GUARD__

#include <map>
#include <optional>
#include <tuple>

// fwd decls
namespace x_data { class window_t; }


enum autoclosemethod {
    OFF,
    ONCE,
    PERSIST
};


struct rulespec_t
{
    rulespec_t(::std::optional<bool> _floating, ::std::optional<bool> _center,
        ::std::optional<autoclosemethod> _autoclose, ::std::optional<bool> _nohint,
        ::std::optional<::std::size_t> _workspace)
        : floating(_floating),
          center(_center),
          autoclose(_autoclose),
          nohint(_nohint),
          workspace(_workspace) {}

    ::std::optional<bool> floating;
    ::std::optional<bool> center;
    ::std::optional<autoclosemethod> autoclose;
    ::std::optional<bool> nohint;
    ::std::optional<::std::size_t> workspace;
};

struct rule_t
{
    rule_t(bool _floating, bool _center, bool _fullscreen,
        bool _autoclose, bool _nohint, ::std::size_t _workspace)
        : floating(_floating),
          center(_center),
          fullscreen(_fullscreen),
          autoclose(_autoclose),
          nohint(_nohint),
          workspace(_workspace) {}

    bool floating;
    bool center;
    bool fullscreen;
    bool autoclose;
    bool nohint;
    ::std::size_t workspace;
};

inline bool
operator<(const rulespec_t& r1, const rulespec_t& r2)
{
    auto zip1 = (r1.floating  ? (*r1.floating  << 0) : 0)
              + (r1.center    ? (*r1.center    << 1) : 0)
              + (r1.autoclose ? (*r1.autoclose << 2) : 0)
              + (r1.nohint    ? (*r1.nohint    << 3) : 0)
              + (r1.workspace ? (*r1.workspace << 4) : 0);

    auto zip2 = (r2.floating  ? (*r2.floating  << 0) : 0)
              + (r2.center    ? (*r2.center    << 1) : 0)
              + (r2.autoclose ? (*r2.autoclose << 2) : 0)
              + (r2.nohint    ? (*r2.nohint    << 3) : 0)
              + (r2.workspace ? (*r2.workspace << 4) : 0);

    return zip1 == zip2;
}

typedef ::std::tuple<::std::string, ::std::string, ::std::string> ruleid_t;

struct rulecomp
{
    bool
    operator()(const ruleid_t& rid1, const ruleid_t& rid2) const
    {
        ::std::string rid1_comp, rid2_comp;

        auto&[rid1_cls, rid1_inst, rid1_title] = rid1;
        auto&[rid2_cls, rid2_inst, rid2_title] = rid2;

        rid1_comp  = rid1_cls + rid1_inst + rid1_title;
        rid2_comp  = rid2_cls + rid2_inst + rid2_title;

        return rid1_comp > rid2_comp;
    }
};

typedef ::std::map<ruleid_t, rulespec_t, rulecomp> rules_t;

extern rule_t zip_rules(rule_t, rule_t);
extern rule_t retrieve_rule(rules_t&, x_data::window_t&);
extern rule_t parse_global_rule(x_data::window_t&);

#endif//__KRANEWM__RULE__GUARD__
