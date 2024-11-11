#include "flow_table.hpp"
#include <string>
#include <unordered_map>
#include <list>
#include <stdexcept>
#include <stdint.h> // TODO: c++ equivalent??
#include <iostream>

void FlowTable::_add_or_update_record(FlowKey key, uint32_t bytes)
{
    // Try direction 1
    auto it = table.find(key);
    if (it != table.end())
    {
        // tx_bytes += value, tx_packets += 1
        it->second.tx_bytes += bytes;
        it->second.tx_packets += 1;
        update_top_ten_with(it->first, it->second);
        return;
    }

    // Try direction 2 -> rx_bytes += value, rx_packets += 1
    FlowKey swapped_directions(key.dst_address, key.dst_port, key.src_address, key.src_port, key.protocol, key.ip);
    it = table.find(swapped_directions);
    if (it != table.end())
    {
        // rx_bytes += value, rx_packets += 1
        it->second.rx_bytes += bytes;
        it->second.rx_packets += 1;
        update_top_ten_with(it->first, it->second);
        return;
    }

    // Key not present in the table
    FlowStats new_record(0, 0, bytes, 1);
    table[key] = new_record;
    update_top_ten_with(key, new_record);
}

std::list<std::pair<FlowKey, FlowStats>> FlowTable::_get_statistics()
{
    return top_ten_records;
}

void FlowTable::update_top_ten_with(FlowKey key,
                                                 FlowStats value)
{
    // Update the value if present
    bool present = false;
    for (auto it = top_ten_records.begin(); it != top_ten_records.end(); it++)
    {
        if (it->first == key)
        {
            it->second = value;
            present = true;
            break;
        }
    }
    
    if (!present)
    {
        top_ten_records.push_front({key, value});
    }

    if (sort_key == SortKey::BYTES){
        top_ten_records.sort([](const std::pair<FlowKey, FlowStats> &a,
                                const std::pair<FlowKey, FlowStats> &b) {
                                    return std::max(a.second.rx_bytes, a.second.tx_bytes) < std::max(b.second.rx_bytes, b.second.tx_bytes);
        });
    } else { // PACKETS
        top_ten_records.sort([](const std::pair<FlowKey, FlowStats> &a,
                                const std::pair<FlowKey, FlowStats> &b) {
                                    return std::max(a.second.rx_packets, a.second.tx_packets) < std::max(b.second.rx_packets, b.second.tx_packets);
        });
    }

    if (top_ten_records.size() > 10)
    {
        top_ten_records.pop_front();
    }
}


void FlowTable::clear()
{
    table.clear();
    top_ten_records.clear();
}

// Public methods
void FlowTable::add_or_update_record(FlowKey key, uint32_t bytes)
{
    std::lock_guard<std::mutex> lock(m);
    return _add_or_update_record(key, bytes);
}

std::list<std::pair<FlowKey, FlowStats>> FlowTable::get_statistics()
{
    std::lock_guard<std::mutex> lock(m);
    std::list<std::pair<FlowKey, FlowStats>> stats = _get_statistics();
    clear();
    return stats;
}

void FlowTable::set_sort_key(SortKey key)
{
    sort_key = key;
}