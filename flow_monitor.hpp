#ifndef CAPTURE_H
#define CAPTURE_H

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
    std::list<std::pair<FlowKey, FlowStats>> get_data();
};

#endif