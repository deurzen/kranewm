#include "rule.hh"

#include "common.hh"
#include "util.hh"
#include "workspace.hh"

#include "x-data/window.hh"

rule_t
zip_rules(rule_t r1, rule_t r2)
{
    bool floating   = false;
    bool center     = false;
    bool fullscreen = false;
    bool autoclose  = false;
    bool nohint     = false;
    ::std::size_t workspace = 0;
    ::std::size_t context   = 0;

    floating   = r1.floating   || r2.floating;
    center     = r1.center     || r2.center;
    fullscreen = r1.fullscreen || r2.fullscreen;
    autoclose  = r1.autoclose  || r2.autoclose;
    nohint     = r1.nohint     || r2.nohint;
    context    = r1.context   ? r1.context   : r2.context;
    workspace  = r1.workspace ? r1.workspace : r2.workspace;

    return {floating, center, fullscreen, autoclose, nohint, context, workspace};
}

rule_t
retrieve_rule(rules_t& rules, x_data::window_t& win)
{
    rule_t global_rule = parse_global_rule(win);

    bool floating  = false;
    bool center    = false;
    bool autoclose = false;
    bool nohint    = false;
    ::std::size_t workspace = 0;
    ::std::size_t context   = 0;

    ::std::string cls   = win.get_class();
    ::std::string inst  = win.get_instance();
    ::std::string title = win.get_name();

    for (auto&& [rule_id,rule] : rules) {
        auto&[rule_cls, rule_inst, rule_title] = rule_id;
        bool same_cls, same_inst, same_title;
        same_cls   = !rule_cls.compare(cls)     || rule_cls.empty();
        same_inst  = !rule_inst.compare(inst)   || rule_inst.empty();
        same_title = !rule_title.compare(title) || rule_title.empty();

        if (same_cls && same_inst && same_title) {
            if (range_t<::std::size_t>::contains(1, 9, rule.workspace))
                workspace = rule.workspace;

            if (range_t<char>::contains('a', 'i', rule.context))
                context = rule.context - 'a' + 1;
            else if (range_t<::std::size_t>::contains(1, 9, rule.context))
                context = rule.context;

            floating = rule.floating;
            center   = rule.center;
            nohint   = rule.nohint;
            if (rule.autoclose != OFF) {
                autoclose = true;
                if (rule.autoclose == ONCE)
                    rule.autoclose = OFF;
            }
            return zip_rules(
                {floating, center, false, autoclose, nohint, context, workspace},
                global_rule
            );
        }
    }

    return zip_rules(
        {false, false, false, false, false, 0, 0},
        global_rule
    );
}

rule_t
parse_global_rule(x_data::window_t& win)
{
    bool floating   = false;
    bool center     = false;
    bool fullscreen = false;
    bool autoclose  = false;
    bool nohint     = false;
    ::std::size_t workspace = 0;
    ::std::size_t context   = 0;

    ::std::string cls   = win.get_class();
    ::std::string inst  = win.get_instance();
    ::std::string title = win.get_name();

    for (auto& n : {cls, inst, title}) {
        static ::std::string::size_type prefix_len = sizeof((WMNAME + ":").c_str());
        if (n.find((WMNAME + ":").c_str()) == 0 && n.size() >= prefix_len) {
            ::std::string rule_flags = n.substr(prefix_len - 1);
            ::std::string::const_iterator it = rule_flags.begin();

            while (it < rule_flags.end()) {
                switch(*it) {
                case 'f': floating   = true; break;
                case 'c': center     = true; break;
                case 'F': fullscreen = true; break;
                case 'a': autoclose  = true; break;
                case 'n': nohint     = true; break;
                case 'x':
                    {
                        if (it + 1 == rule_flags.end())
                            break;

                        char cx_flag = *++it;
                        if (range_t<char>::contains('a', 'i', cx_flag))
                            context = cx_flag - 'a' + 1;
                        else if (range_t<char>::contains('1', '9', cx_flag))
                            context = cx_flag - '0';
                    }
                    break;
                case 'w':
                    {
                        if (it + 1 == rule_flags.end())
                            break;

                        char ws_flag = *++it;
                        if (range_t<char>::contains('1', '9', ws_flag))
                            workspace = ws_flag - '0';
                    }
                    break;
                default: break;
                }
                ++it;
            }
        }
    }

    return {floating, center, fullscreen, autoclose, nohint, context, workspace};
}
