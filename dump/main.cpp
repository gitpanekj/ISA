/**
 * @file main.cpp
 * @author Jan Pánek (xpanek11@stud.fit.vut.cz)
 * @brief Application runner file
 * @version 0.1
 * @date 2024-10-03
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <string>
#include <stdexcept>
#include <iostream>
#include "model.hpp"
#include "view.hpp"
#include "controller.hpp"

enum class SortKey
{
    none = 0,
    bytes = 1,
    packets_per_second = 2
};

struct Config
{
    std::string interface; // required
    SortKey sort_key = SortKey::none;
    bool help = false;
};

Config parseArgs(int argc, char *argv[])
{
    Config config;
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h")
        {
            config.help = true;
            return config;
        }
        else if (arg == "-i")
        {
            if (!config.interface.empty())
            {
                throw std::invalid_argument("Interface already specified");
            }
            if (i < (argc - 1))
            {
                config.interface = argv[++i];
            }
            else
            {
                throw std::invalid_argument("Missing interface name after -i");
            }
        }
        else if (arg == "-s")
        {
            if (config.sort_key != SortKey::none)
            {
                throw std::invalid_argument("Sort key already specified");
            }
            if (i < (argc - 1))
            {
                std::string key = argv[++i];
                if (key == "b")
                {
                    config.sort_key = SortKey::bytes;
                }
                else if (key == "p")
                {
                    config.sort_key = SortKey::packets_per_second;
                }
                else
                {
                    throw std::invalid_argument("Unknown sort key");
                }
            }
            else
            {
                throw std::invalid_argument("Missing sort key after -s");
            }
        }
    }

    if (config.interface.empty())
    {
        throw std::invalid_argument("Missing interface");
    }

    return config;
}

void help()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "isa-top -i int [-s b|p]" << std::endl;
    std::cout << "  * -i int: interface to be listened" << std::endl;
    std::cout << "  * -s b|p: output is sorted by bytes/packets/s" << std::endl;
}

int main(int argc, char *argv[])
{
    // Parse args
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

    
    // Run app with given config
    std::cout << "Interface: " << config.interface << std::endl;
    std::cout << "Sort key: " << int(config.sort_key) << std::endl;

    application::Model model;
    application::View view;
    application::Controller controller(model, view);

    int status_code = controller.run_app();

    return status_code;



    return 0;
}