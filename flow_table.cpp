/**
 * @file flow_table.cpp
 * @author Jan Pánek (xpanek11@stud.fit.vut.cz)
 * @brief Implementation of flow table for storing flow statistics.
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "flow_table.hpp"
#include <string>
#include <unordered_map>
#include <list>
#include <stdexcept>
#include <cstdint>
#include <iostream>

/**
 * @brief Update existing record with key in top ten flow list or insert new, then sort.
 * 
 * @param key flow identification (src:port, dst:port, protocol)
 * @param bytes number of transferred bytes
 */
void FlowTable::_addOrUpdateRecord(FlowKey key, uint32_t bytes)
{
    // Try direction 1
    auto it = table.find(key);
    if (it != table.end())
    {
        it->second.tx_bytes += bytes;
        it->second.tx_packets += 1;
        updateTopTenWith(it->first, it->second);
        return;
    }

    // Try direction 2
    FlowKey swapped_directions(key.dst_address, key.dst_port, key.src_address, key.src_port, key.protocol, key.ip);
    it = table.find(swapped_directions);
    if (it != table.end())
    {
        it->second.rx_bytes += bytes;
        it->second.rx_packets += 1;
        updateTopTenWith(it->first, it->second);
        return;
    }

    // Key not present in the table
    FlowStats new_record(0, 0, bytes, 1);
    table[key] = new_record;
    updateTopTenWith(key, new_record);
}

/**
 * @brief Return list of top ten communication flows.
 * 
 * @return std::list<std::pair<FlowKey, FlowStats>> 
 */
std::list<std::pair<FlowKey, FlowStats>> FlowTable::_getStatistics()
{
    return top_ten_records;
}

/**
 * @brief Update list of top ten communication flows.
 * 
 * @param key   (src:port, dst:port, protocol)
 * @param value update number of bytes fore the flow
 */
void FlowTable::updateTopTenWith(FlowKey key,
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

    // Push new if not present    
    if (!present)
    {
        top_ten_records.push_front({key, value});
    }

    // Order the list
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

    // Pop last elem if size exceeds 10
    if (top_ten_records.size() > 10)
    {
        top_ten_records.pop_front();
    }
}

/**
 * @brief Clear table and top ten record structures.
 * 
 */
void FlowTable::clear()
{
    table.clear();
    top_ten_records.clear();
}

// Public methods
/**
 * @brief _addOrUpdateRecord with lock
 * 
 * @param key 
 * @param bytes 
 */
void FlowTable::addOrUpdateRecord(FlowKey key, uint32_t bytes)
{
    std::lock_guard<std::mutex> lock(m);
    return _addOrUpdateRecord(key, bytes);
}

/**
 * @brief _getStatistics with lock
 * 
 * @return std::list<std::pair<FlowKey, FlowStats>> 
 */
std::list<std::pair<FlowKey, FlowStats>> FlowTable::getStatistics()
{
    std::lock_guard<std::mutex> lock(m);
    std::list<std::pair<FlowKey, FlowStats>> stats = _getStatistics();
    clear();
    return stats;
}

/**
 * @brief Sort key for sorting the records in the top ten table
 * 
 * @param key 
 */
void FlowTable::setSortKey(SortKey key)
{
    sort_key = key;
}