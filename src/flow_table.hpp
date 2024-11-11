#ifndef TST_H
#define TST_H

#include <string>
#include <unordered_map>
#include <list>
#include <stdint.h> // TODO: c++ equivalent??
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

// Structure for the statistics - #TODO: cite C++ documentation: https://en.cppreference.com/w/cpp/container/unordered_map/unordered_map
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
        return std::tie(src_address, src_port, dst_address, dst_port) ==
               std::tie(rhs.src_address, rhs.src_port, rhs.dst_address, rhs.dst_port);
    }

    std::string src_address;
    uint16_t src_port;
    std::string dst_address;
    uint16_t dst_port;
    std::string protocol;
    IpAddrClass ip;
};


// #TODO: cite C++ documentation: https://en.cppreference.com/w/cpp/container/unordered_map/unordered_map
template <>
struct std::hash<FlowKey>
{
    std::size_t operator()(const FlowKey &val) const
    {
        return (1 << std::hash<int>{}(val.src_port)) ^
               (3 << std::hash<int>{}(val.dst_port)) ^
               (5 << std::hash<std::string>{}(val.src_address)) ^
               (7 << std::hash<std::string>{}(val.dst_address));
    }
};

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
    void update_top_ten_with(FlowKey key, FlowStats value);
    void clear();
    void _add_or_update_record(FlowKey key, uint32_t);
    // Top 10 ordered by given key
    std::list<std::pair<FlowKey, FlowStats>> _get_statistics();
    
public:
    void set_sort_key(SortKey key);
    void add_or_update_record(FlowKey key, uint32_t);
    std::list<std::pair<FlowKey, FlowStats>> get_statistics();
};

#endif