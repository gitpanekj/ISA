#include <string>
#include <stdexcept>
#include <iostream>
#include "argument_parser.hpp"
#include "flow_table.hpp"




Config parseArgs(int argc, char *argv[])
{
    Config config;
    config.sort_key = SortKey::BYTES;
    bool sort_key_set = false;
    bool iface_set = false;
    bool out_set = false;
    config.outDirector = "";

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
            if (iface_set)
            {
                throw std::invalid_argument("Interface already specified");
            }
            if (i < (argc - 1))
            {
                config.interface = argv[++i];
                iface_set = true;
            }
            else
            {
                throw std::invalid_argument("Missing interface name after -i");
            }
        }
        else if (arg == "-s")
        {
            if (sort_key_set)
            {
                throw std::invalid_argument("Sort key already specified");
            }

            if (i < (argc - 1))
            {
                std::string key = argv[++i];
                if (key == "b")
                {
                    config.sort_key = SortKey::BYTES;
                }
                else if (key == "p")
                {
                    config.sort_key = SortKey::PACKETS;
                }
                sort_key_set = true;
            }
            else
            {
                throw std::invalid_argument("Missing sort key after -s");
            }
        }
        else if (arg == "-d")
        {
            if (out_set)
            {
                throw std::invalid_argument("Output directory already specified");
            }
            if (i < (argc - 1))
            {
                std::string key = argv[++i];
                config.outDirector = key;
                config.out = true;
                out_set = true;
            }
            else
            {
                throw std::invalid_argument("Missing directory path after -d");
            }
        }
    }

    if (!iface_set)
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