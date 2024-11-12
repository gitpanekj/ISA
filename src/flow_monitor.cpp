#include <pcap.h>
#include <iostream>
#include <string.h>
#include <stdexcept>


#include "flow_table.hpp"
#include "flow_monitor.hpp"
#include "capturing_utils.hpp"

#define PROMISCUOUS 1
#define UNLIMITED 0


FlowMonitor::FlowMonitor(const char *interface, SortKey key)
{
    char error_buffer[PCAP_ERRBUF_SIZE];
    int timeout_limit = 1000; // 1s

    // live capture
    handle = pcap_open_live(
        interface,
        BUFSIZ,
        PROMISCUOUS,
        timeout_limit,
        error_buffer);

    if (handle == nullptr)
    {
        std::string err = std::string(error_buffer, PCAP_ERRBUF_SIZE);
        throw std::invalid_argument(err);
    }
    table.set_sort_key(key);
}

void FlowMonitor::start()
{
    void *args[2] = {&table, &handle};
    
    int status = pcap_loop(handle, UNLIMITED, packet_handler, (u_char *)&args);
    if (status == -1)
    {
        throw std::runtime_error(std::string(pcap_geterr(handle)));
    }
}

void FlowMonitor::stop()
{
    if (handle != nullptr)
    {
        pcap_breakloop(handle);
    }
    pcap_close(handle);
    handle = nullptr;
}

std::list<std::pair<FlowKey, FlowStats>> FlowMonitor::get_data()
{
    return table.get_statistics();
}