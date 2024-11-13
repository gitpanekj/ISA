/**
 * @file ncurses_terminal_view.hpp
 * @author Jan Pánek (xpanek11@stud.fit.vut.cz)
 * @brief Implementation of ncurses terminal interface.
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef NCURSES_TERMINAL_VIEW_HPP
#define NCURSES_TERMINAL_VIEW_HPP

#include <list>
#include "flow_table.hpp"

int  startUI();
void updateView(std::list<std::pair<FlowKey, FlowStats>> data, unsigned int period);
void writeWindowToFile(const std::string &filename);
int  stopUI();

#endif