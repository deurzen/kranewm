#ifndef __KRANEWM__RULE__GUARD__
#define __KRANEWM__RULE__GUARD__

#include <map>
#include <tuple>

// fwd decl

enum autoclosemethod {
    OFF,
    ONCE,
    PERSIST
};


struct rulespec_t
{
    rulespec_t(bool _floating, bool _center, bool _iconify,
        autoclosemethod _autoclose, bool _nohint, unsigned _workspace)
        : floating(_floating),
          center(_center),
          iconify(_iconify),
          autoclose(_autoclose),
          nohint(_nohint),
          workspace(_workspace) {}

    bool floating;
    bool center;
    bool iconify;
    autoclosemethod autoclose;
    bool nohint;
    unsigned workspace;
};

struct rule_t
{
    rule_t(bool _floating, bool _center, bool _fullscreen, bool _iconify,
        bool _autoclose, bool _nohint, unsigned _workspace)
        : floating(_floating),
          center(_center),
          fullscreen(_fullscreen),
          iconify(_iconify),
          autoclose(_autoclose),
          nohint(_nohint),
          workspace(_workspace) {}

    bool floating;
    bool center;
    bool fullscreen;
    bool iconify;
    bool autoclose;
    bool nohint;
    unsigned workspace;
};

inline bool
operator<(const rulespec_t& r1, const rulespec_t& r2)
{
    return (r1.floating + r1.center + r1.iconify + r1.autoclose + r1.nohint + r1.workspace)
        == (r2.floating + r2.center + r2.iconify + r2.autoclose + r2.nohint + r2.workspace);
}

typedef ::std::tuple<::std::string, ::std::string, ::std::string> ruleid_t;

struct rulecomp
{
    bool
    operator()(const ruleid_t& rid1, const ruleid_t& rid2) const
    {
        ::std::string rid1_cls, rid1_inst, rid1_title;
        ::std::string rid2_cls, rid2_inst, rid2_title;
        ::std::string rid1_comp, rid2_comp;

        rid1_cls   = ::std::get<0>(rid1);
        rid1_inst  = ::std::get<1>(rid1);
        rid1_title = ::std::get<2>(rid1);

        rid2_cls   = ::std::get<0>(rid2);
        rid2_inst  = ::std::get<1>(rid2);
        rid2_title = ::std::get<2>(rid2);

        rid1_comp  = rid1_cls + rid1_inst + rid1_title;
        rid2_comp  = rid2_cls + rid2_inst + rid2_title;

        return rid1_comp > rid2_comp;
    }
};

typedef ::std::map<ruleid_t, rulespec_t, rulecomp> rules_t;


#endif//__KRANEWM__RULE__GUARD__
