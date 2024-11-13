/**
 * @file argument_parser.hpp
 * @author Jan Pánek (xpanek11@stud.fit.vut.cz)
 * @brief Argument parsing.
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef ARG_HPP
#define ARG_HPP
#include <string>
#include "flow_table.hpp"

struct Config
{
    const char* interface; // required
    SortKey sort_key;
    bool help = false;
    bool out = false;
    std::string outDirector;
    int refresh_time;
};


Config parseArgs(int, char *[]);
void help();

#endif