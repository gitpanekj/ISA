#ifndef ARG_H
#define ARG_H
#include <string>
#include "flow_table.hpp"

struct Config
{
    const char* interface; // required
    SortKey sort_key;
    bool help = false;
};


Config parseArgs(int, char *[]);
void help();

#endif