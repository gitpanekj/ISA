#ifndef ARG_H
#define ARG_H
#include <string>
#include "flow_table.hpp"

struct Config
{
    const char* interface; // required
    SortKey sort_key;
    bool help = false;
    bool out = false;
    std::string outDirector; 
};


Config parseArgs(int, char *[]);
void help();

#endif