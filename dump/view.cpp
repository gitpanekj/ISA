/**
 * @file view.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-10-03
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <ncurses.h>
#include "Observer.hpp"
#include "exceptions.hpp"
#include "app_state.hpp"
#include "view.hpp"

namespace application
{
    View::View()
    {
        initscr();            // initialize ncurses
        noecho();             // do not echo user input
        cbreak();             // disable buffering
        keypad(stdscr, TRUE); // Enable f-keys, arrows, numpad
    }

    View::~View()
    {
        clear();   // clear the window buffer
        refresh(); // display cleared buffer
        endwin();  // release resources related to n curses
    }

    void View::display()
    {
        throw new exceptions::NotImplemented("void View.display()");
        /* TODO: implement display method and its overloads */
    }

    /* Observer related functionality */
    void View::update(const SharedState &state)
    {
        throw new exceptions::NotImplemented("void View.update()");
        /* TODO: implement view update after model notify about the state change */
    }
}