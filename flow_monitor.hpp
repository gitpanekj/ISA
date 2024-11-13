/**
 * @file flow_monitor.hpp
 * @author Jan Pánek (xpanek11@stud.fit.vut.cz)
 * @brief Class implementing packet capturing and flow identification.
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef CAPTURE_HPP
#define CAPTURE_HPP

#include <pcap.h>
#include "flow_table.hpp"


class FlowMonitor
{
private:
    pcap_t *handle;
    FlowTable table;
public:
    FlowMonitor(const char *, SortKey key);
    void start();
    void stop();
    std::list<std::pair<FlowKey, FlowStats>> getData();
};

#endif