/**
 * @file main.cpp
 * @author Jan Pánek (xpanek11@stud.fit.vut.cz)
 * @brief Isa-top runner.
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <thread>
#include <mutex>
#include <atomic>
#include <csignal>
#include <pcap.h>
#include <csignal>
#include <iostream>
#include <ncurses.h>
#include <tuple>
#include <string>
#include "flow_monitor.hpp"
#include "flow_table.hpp"
#include "ncurses_terminal_view.hpp"
#include "argument_parser.hpp"

// Shared data - application state
bool running = true;
void terminate(int signum)
{
    (void)signum;
    running = false;
}

int main(int argc, char *argv[])
{
    // Parse Args
    Config config;
    try
    {
        config = parseArgs(argc, argv);
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        help();
        return 1;
    }
    if (config.help)
    {
        help();
        return 0;
    }

    std::signal(SIGINT, terminate);
    std::list<std::pair<FlowKey, FlowStats>> view_data;
    
    try
    {
        FlowMonitor monitor(config.interface, config.sort_key);

        std::thread monitor_thread(&FlowMonitor::start, &monitor);

        startUI();

        while (running)
        {
            view_data = monitor.getData();
            updateView(view_data, config.refresh_time);
            if (config.out){
                writeWindowToFile(config.outDirector);
            }
            std::this_thread::sleep_for(std::chrono::seconds(config.refresh_time));
        }

        monitor.stop();
        monitor_thread.join();
        stopUI();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}