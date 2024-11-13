/**
 * @file flow_monitor.cpp
 * @author Jan Pánek (xpanek11@stud.fit.vut.cz)
 * @brief Class implementing packet capturing and flow identification.
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <pcap.h>
#include <iostream>
#include <string.h>
#include <stdexcept>


#include "flow_table.hpp"
#include "flow_monitor.hpp"
#include "capturing_utils.hpp"

#define PROMISCUOUS 1
#define UNLIMITED 0


/**
 * @brief Construct a new Flow Monitor:: Flow Monitor object
 * 
 * @param interface interface to capture packets on
 * @param key key used to sort value in flow table - Bytes | Packets
 */
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
    table.setSortKey(key);
}

/**
 * @brief Start capturing loop with given packet_handler.
 * 
 */
void FlowMonitor::start()
{
    void *args[2] = {&table, &handle};
    
    int status = pcap_loop(handle, UNLIMITED, packet_handler, (u_char *)&args);
    if (status == -1)
    {
        throw std::runtime_error(std::string(pcap_geterr(handle)));
    }
}

/**
 * @brief Stop capturing loop.
 * 
 */
void FlowMonitor::stop()
{
    if (handle != nullptr)
    {
        pcap_breakloop(handle);
    }
    pcap_close(handle);
    handle = nullptr;
}

/**
 * @brief Get gathered flow statistics from the FlowMonitor FlowTable.
 * 
 * @return std::list<std::pair<FlowKey, FlowStats>> 
 */
std::list<std::pair<FlowKey, FlowStats>> FlowMonitor::getData()
{
    return table.getStatistics();
}