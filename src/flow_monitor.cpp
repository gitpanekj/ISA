#include <pcap.h>
#include <iostream>
#include <arpa/inet.h>
#include <pcap.h>
#include <string.h>
#include <netdb.h>
#include <tuple>
#include <stdexcept>

// Packet structures
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>

#include "flow_table.hpp"
#include "flow_monitor.hpp"

#define PROMISCUOUS 1
#define UNLIMITED 0
#define ETHER_SIZE 14         // octets
#define IPV4_BASE_SIZE 20     // octets
#define IPV6_HEADER_SIZE 40   // octets
#define IPV6_EXT_HEADER_SIZE  //octets
#define ETHERNET_PAYLOAD(packet) packet + ETHER_SIZE
#define IPv4_PAYLOAD(ip_packet, ihl) ip_packet + ihl * 4
#define IPv6_PAYLOAD(ip_packet) ip_packet + IPV6_HEADER_SIZE

struct capture {
    const u_char* capture;
    unsigned int pos;
    unsigned int caplen;
};


std::string get_protocol_name_by_number(uint8_t protocol_number)
{
    switch (protocol_number)
    {
    case 6:
        return "tcp";
    case 17:
        return "udp";
    case 1:
        return "icmp";
    case 58:
        return "icmp6";
    default:
        return "Unknown";
    }
    // Return the protocol name as a string
}

std::pair<uint16_t, uint16_t> get_port_numbers(const u_char *packet)
{
    const udphdr *udp_tcp_header = (const udphdr *)packet;
    uint16_t src_port;
    uint16_t dst_port;
    // Save src, dst port
    // TODO: check retvals
    src_port = ntohs(udp_tcp_header->source);
    dst_port = ntohs(udp_tcp_header->dest);
    return std::pair(src_port, dst_port);
}

std::string ipv4_source_address(const struct iphdr *ip_header)
{
    char *address;
    struct in_addr ipv4_address;
    char ip4_source[INET_ADDRSTRLEN];

    ipv4_address.s_addr = ip_header->saddr;
    // TODO: check retvals
    address = inet_ntoa(ipv4_address);
    memcpy(ip4_source, address, 15);
    ip4_source[15] = 0;
    return std::string(ip4_source);
}

std::string ipv4_destination_address(const struct iphdr *ip_header)
{
    char *address;
    struct in_addr ipv4_address;
    char ip4_destination[INET_ADDRSTRLEN];

    ipv4_address.s_addr = ip_header->daddr;
    // TODO: check retvals
    address = inet_ntoa(ipv4_address);
    memcpy(ip4_destination, address, 15);
    ip4_destination[15] = 0;
    return std::string(ip4_destination);
}

std::string ipv4_protocol(const struct iphdr *ip_header)
{
    return get_protocol_name_by_number(ip_header->protocol);
}

uint16_t ipv4_total_length(const struct iphdr *ip_header)
{
    // TODO: check retvals
    return ntohs(ip_header->tot_len);
}

std::string ipv6_source_address(const struct ip6_hdr *ip6_header)
{
    char ip6_address[INET6_ADDRSTRLEN];
    // TODO: check retvals
    inet_ntop(AF_INET6, &(ip6_header->ip6_src), ip6_address, INET6_ADDRSTRLEN);
    return std::string(ip6_address);
}

std::string ipv6_destination_address(const struct ip6_hdr *ip6_header)
{
    char ip6_address[INET6_ADDRSTRLEN];
    // TODO: check retvals
    inet_ntop(AF_INET6, &(ip6_header->ip6_dst), ip6_address, INET6_ADDRSTRLEN);
    return std::string(ip6_address);
}

std::string ipv6_protocol(const struct ip6_hdr *ip6_header)
{
    return get_protocol_name_by_number(ip6_header->ip6_ctlun.ip6_un1.ip6_un1_nxt);
}

uint16_t ipv6_total_length(const struct ip6_hdr *ip6_header)
{
    // TODO: check retvals
    return ntohs(ip6_header->ip6_ctlun.ip6_un1.ip6_un1_plen);
}

std::pair<FlowKey, uint16_t> process_ipv4(const u_char *packet)
{
    const iphdr *ip_header = (const iphdr *)(packet);
    std::string source_address = ipv4_source_address(ip_header);
    std::string destination_address = ipv4_destination_address(ip_header);
    std::string protocol = ipv4_protocol(ip_header);
    uint16_t length = ipv4_total_length(ip_header);
    std::tuple<uint16_t, uint16_t> src_dst_port = {0, 0}; // by default 0 - unused

    // TCP, UDP
    if ((ip_header->protocol == IPPROTO_TCP) || (ip_header->protocol == IPPROTO_UDP))
    {
        src_dst_port = get_port_numbers(IPv4_PAYLOAD(packet, ip_header->ihl));
    }

    return {FlowKey(source_address, std::get<0>(src_dst_port), destination_address, std::get<1>(src_dst_port), protocol, IpAddrClass::IPV4), length};
}

std::pair<FlowKey, uint16_t> process_ipv6(const u_char *packet)
{
    const ip6_hdr *ip6_header = (const ip6_hdr *)(packet);
    std::string source_address = ipv6_source_address(ip6_header);
    std::string destination_address = ipv6_destination_address(ip6_header);
    std::string protocol = ipv6_protocol(ip6_header); // TODO: extension headers
    uint16_t length = ipv6_total_length(ip6_header);
    std::tuple<uint16_t, uint16_t> src_dst_port = {0, 0}; // by default 0 - unused

    uint8_t protocol_number = ip6_header->ip6_ctlun.ip6_un1.ip6_un1_nxt;
    if ((protocol_number == IPPROTO_TCP) || (protocol_number == IPPROTO_UDP))
    {
        src_dst_port = get_port_numbers(IPv6_PAYLOAD(packet));
    }

    return {FlowKey(source_address, std::get<0>(src_dst_port), destination_address, std::get<1>(src_dst_port), protocol, IpAddrClass::IPV6), length};
}

void packet_handler(u_char *args, const struct pcap_pkthdr *packet_header, const u_char *packet)
{
    if (args == NULL)
    {
        // TODO: error msg
        exit(1);
    }
    void** args_ = (void**)args;
    void* arg1 = args_[0];
    //void* arg2 = args_[1];
    FlowTable* table = (FlowTable*)arg1; 
    //pcap_t* handle = (pcap_t*)arg2;

    std::pair<FlowKey, uint16_t> capture({FlowKey("", 0, "", 0, "", IpAddrClass::IPV4), 0});

    
    struct capture cap = {.capture = packet, .pos = 0, .caplen = packet_header->caplen};

   

    struct ether_header *eth_header;
    eth_header = (struct ether_header *)packet;

    // Check whether whole MAC header is contained

    // TODO: check retvals
    switch (ntohs(eth_header->ether_type))
    {
    case ETHERTYPE_IP:
        capture = process_ipv4(ETHERNET_PAYLOAD(packet));
        break;
    case ETHERTYPE_IPV6:
        capture = process_ipv6(ETHERNET_PAYLOAD(packet));
        break;
    default:
        return;
    }
    if (capture.first.protocol == "Unknown")
        return;

    // Update the table
    table->add_or_update_record(capture.first, capture.second);
}

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

    if (handle == nullptr){
        std::string err = std::string(error_buffer, PCAP_ERRBUF_SIZE);
        throw std::invalid_argument(err);
    }
    table.set_sort_key(key);
}

void FlowMonitor::start()
{
    // TODO: what does return code indicate
    void *args[2] = {&table, &handle};
    // TODO: check retvals
    pcap_loop(handle, UNLIMITED, packet_handler, (u_char *)&args);
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