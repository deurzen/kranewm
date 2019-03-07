#ifndef __KRANEWM__RULE__GUARD__
#define __KRANEWM__RULE__GUARD__

#include <map>
#include <tuple>


enum AutocloseMethod {
    OFF,
    ONCE,
    PERSIST
};

struct Rule
{
    Rule(bool _floating, bool _center, bool _iconify,
        AutocloseMethod _autoclose, unsigned _workspace)
      : floating(_floating),
        center(_center),
        iconify(_iconify),
        autoclose(_autoclose),
        workspace(_workspace)
    {}

    bool floating;
    bool center;
    bool iconify;
    AutocloseMethod autoclose;
    unsigned workspace;
};

inline bool
operator<(const Rule& r1, const Rule& r2)
{
    return (r1.autoclose + r1.center + r1.floating + r1.iconify + r1.workspace)
        == (r2.autoclose + r2.center + r2.floating + r2.iconify + r2.workspace);
}

typedef ::std::tuple<::std::string, ::std::string, ::std::string> RuleId;

struct CompareRule {
    bool
    operator()(const RuleId& rid1, const RuleId& rid2) const
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

typedef ::std::map<RuleId, Rule, CompareRule> Rules;

#endif//__KRANEWM__X_EVENTS__GUARD__
