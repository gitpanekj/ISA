/**
 * @file flow_table.hpp
 * @author Jan Pánek (xpanek11@stud.fit.vut.cz)
 * @brief Implementation of flow table for storing flow statistics.
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef TST_HPP
#define TST_HPP

#include <string>
#include <unordered_map>
#include <list>
#include <cstdint>
#include <mutex>
#include <memory>

enum class IpAddrClass {
    IPV4 = 0,
    IPV6 = 1
};

enum class SortKey
{
    BYTES,
    PACKETS
};

/**
 * @brief Key uniquely identifying a flow - src_address, src_port, dst_address, dst_port, protocol
 * 
 */
struct FlowKey
{
    FlowKey(std::string src_address_,
                         uint16_t src_port_,
                         std::string dst_address_,
                         uint16_t dst_port_,
                         std::string protocol_,
                         IpAddrClass ip_) : src_address(src_address_),
                                                  src_port(src_port_),
                                                  dst_address(dst_address_),
                                                  dst_port(dst_port_),
                                                  protocol(protocol_),
                                                  ip(ip_) {}
    bool operator==(const FlowKey &rhs) const
    {
        return std::tie(    src_address,     src_port,     dst_address,     dst_port,     protocol) ==
               std::tie(rhs.src_address, rhs.src_port, rhs.dst_address, rhs.dst_port, rhs.protocol);
    }

    std::string src_address;
    uint16_t src_port;
    std::string dst_address;
    uint16_t dst_port;
    std::string protocol;
    IpAddrClass ip;
};


template <>
struct std::hash<FlowKey>
{
    std::size_t operator()(const FlowKey &val) const
    {
        return (1 << std::hash<int>{}(val.src_port)) ^
               (3 << std::hash<int>{}(val.dst_port)) ^
               (5 << std::hash<std::string>{}(val.src_address)) ^
               (7 << std::hash<std::string>{}(val.dst_address)) ^
               (9 << std::hash<std::string>{}(val.protocol));
    }
};

/**
 * @brief 
 * 
 */
struct FlowStats
{
    FlowStats() : rx_bytes(0), rx_packets(0), tx_bytes(0), tx_packets(0) {}
    FlowStats(unsigned long long rx_bytes_,
                           unsigned long long rx_packets_,
                           unsigned long long tx_bytes_,
                           unsigned long long tx_packets_) : rx_bytes(rx_bytes_), rx_packets(rx_packets_), tx_bytes(tx_bytes_), tx_packets(tx_packets_) {}
    unsigned long long rx_bytes;
    unsigned long long rx_packets;
    unsigned long long tx_bytes;
    unsigned long long tx_packets;
};


/**
 * @brief Table for storing statistics about captured flows.
 * 
 * Manages list of top ten communication flows based on provided sort key.
 * Thread-safe, table or top ten list access uses locks.
 * 
 */
class FlowTable
{
private:
    // Access Control
    std::mutex m;
    
    std::unordered_map<FlowKey, FlowStats> table;
    std::list<std::pair<FlowKey, FlowStats>> top_ten_records;
    SortKey sort_key;

    // If exists, update count, else create new record
    // Place to the right pos in th array of the top [10] or update existing
    void updateTopTenWith(FlowKey key, FlowStats value);
    void clear();
    void _addOrUpdateRecord(FlowKey key, uint32_t value);
    
    // Top 10 ordered by given key
    std::list<std::pair<FlowKey, FlowStats>> _getStatistics();
    
public:
    void setSortKey(SortKey key);
    void addOrUpdateRecord(FlowKey key, uint32_t value);
    std::list<std::pair<FlowKey, FlowStats>> getStatistics();
};

#endif