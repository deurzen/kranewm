#include "config.hh"
#include "defaults.hh"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

Config::Config()
{
    { // set config directory
        std::stringstream configdir_ss;

        if (const char* env_xdgconf = std::getenv("XDG_CONFIG_HOME"))
            configdir_ss << env_xdgconf << "/" << WM_NAME << "/";
        else
            configdir_ss << "$HOME/.config/" << WM_NAME << "/";

        directory = configdir_ss.str();
    }

    { // produce vector of to-ignore-{producers,consumers}
        std::ifstream in(directory + std::string("consumeignore"));

        if (in.good()) {
            std::string line;

            while (std::getline(in, line)) {
                std::string::size_type pos = line.find('#');

                if (pos != std::string::npos)
                    line = line.substr(0, pos);

                if (line.length() < 5)
                    continue;

                line.erase(4, line.find_first_not_of(" \t\n\r\f\v"));
                line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

                if (line.length() < 5)
                    continue;

                std::vector<SearchSelector_ptr>* ignored;
                switch (line[0]) {
                case '<': ignored = &ignored_producers; break;
                case '>': ignored = &ignored_consumers; break;
                default: continue;
                }

                SearchSelector::SelectionCriterium criterium;

                switch (line[2]) {
                case 'N':
                {
                    switch (line[1]) {
                    case '=': criterium = SearchSelector::SelectionCriterium::ByNameEquals;   break;
                    case '~': criterium = SearchSelector::SelectionCriterium::ByNameContains; break;
                    default: continue;
                    }

                    break;
                }
                case 'C':
                {
                    switch (line[1]) {
                    case '=': criterium = SearchSelector::SelectionCriterium::ByClassEquals;   break;
                    case '~': criterium = SearchSelector::SelectionCriterium::ByClassContains; break;
                    default: continue;
                    }

                    break;
                }
                case 'I':
                {
                    switch (line[1]) {
                    case '=': criterium = SearchSelector::SelectionCriterium::ByInstanceEquals;   break;
                    case '~': criterium = SearchSelector::SelectionCriterium::ByInstanceContains; break;
                    default: continue;
                    }

                    break;
                }
                default: continue;
                }

                ignored->push_back(new SearchSelector{criterium, line.substr(4)});
            }
        }
    }
}

Config::~Config()
{
    for (std::size_t i = 0; i < ignored_producers.size(); ++i)
        delete ignored_producers[i];

    for (std::size_t i = 0; i < ignored_consumers.size(); ++i)
        delete ignored_consumers[i];
}
