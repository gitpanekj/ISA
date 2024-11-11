#ifndef NCURSES_TERMINAL_VIEW_HPP
#define NCURSES_TERMINAL_VIEW_HPP

#include <list>
#include "flow_table.hpp"

int  start_ui();
void update_view(std::list<std::pair<FlowKey, FlowStats>> data);
int  stop_ui();

#endif