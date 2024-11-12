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

        start_ui();

        while (running)
        {
            view_data = monitor.get_data();
            update_view(view_data);
            if (config.out){
                write_window_to_file(config.outDirector);
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        monitor.stop();
        monitor_thread.join();
        stop_ui();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}