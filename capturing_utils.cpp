/**
 * @file capturing_utils.cpp
 * @author Jan Pánek (xpanek11@stud.fit.vut.cz)
 * @brief Utilities for processing of captured packet.
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "capturing_utils.hpp"
#include "flow_table.hpp"

// Packet structures and networking utils
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <pcap.h>

#include <memory>
#include <stdexcept>
#include <string.h>
#include <iostream>

// Packet header sizes
#define ETHER_SIZE 14        // octets
#define IPV4_BASE_SIZE 20    // octets
#define IPV6_HEADER_SIZE 40  // octets
#define IPV6_EXT_HEADER_SIZE // octets

/**
 * @brief Capture structure
 * 
 * Holds captured data, current position in the captured data and total size of captured data.
 */
struct capture
{
    capture(const u_char *data_,
            unsigned int pos_,
            unsigned int caplen_) : data(data_), pos(pos_), caplen(caplen_) {}
    const u_char * data;
    unsigned int pos;
    unsigned int caplen;
};

/**
 * @brief Get pointer to the capture data with offset pos
 * 
 * @param cap capture structure
 * @return u_char* pointer to the captured data with offset pos
 */
u_char *captureFromPos(struct capture cap)
{
    return (u_char *)(cap.data + cap.pos);
}

/**
 * @brief Check whether there is enough data left in the capture for ethernet header.
 * 
 * @param cap capture structure
 */
void checkEther(struct capture cap)
{
    if ((cap.pos + ETHER_SIZE) > cap.caplen)
        throw std::runtime_error("Insufficient caplen to process ether frame.");
}

/**
 * @brief Move to the ethernet payload in the capture.
 * 
 * @param cap 
 */
void skipEther(struct capture *cap)
{
    cap->pos += ETHER_SIZE;
}


/**
 * @brief Check whether there is enough data left in the capture for ipv4 header without options.
 * 
 * @param cap 
 */
void checkIPv4BaseHeader(struct capture cap)
{
    if ((cap.pos + IPV4_BASE_SIZE) > cap.caplen)
        throw std::runtime_error("Insufficient caplen to process ipv4 base header.");
}


/**
 * @brief Check whether there is enough data left in the capture for ipv4 header including options.
 * 
 * @param cap 
 */
void checkIPv4Header(struct capture cap, u_char ihl)
{
    if ((cap.pos + ihl * 4) > cap.caplen)
        throw std::runtime_error("Insufficient caplen to process ipv4 header.");
}

/**
 * @brief Move to the ipv4 payload in the capture.
 * 
 * @param cap 
 * @param ihl internet header length
 */
void skipIPv4Header(struct capture *cap, u_char ihl)
{
    cap->pos += ihl * 4;
}

/**
 * @brief Check whether there is enough data left in the capture for ipv6 header.
 * 
 * @param cap 
 */
void checkIPv6BaseHeader(struct capture cap)
{
    if ((cap.pos + IPV6_HEADER_SIZE) > cap.caplen)
        throw std::runtime_error("Insufficient caplen to process ipv6 base header.");
}

/**
 * @brief Move to the ipv6 payload in the capture.
 * 
 * @param cap 
 */
void skipIPv6BaseHeader(struct capture *cap)
{
    cap->pos += IPV6_HEADER_SIZE;
}

/**
 * @brief Check whether there is enough data left in the capture for tcp/udp source and destination port numbers.
 * 
 * @param cap 
 */
void checkTcpUdpPorts(struct capture cap)
{
    if ((cap.pos + 4) > cap.caplen)
        throw std::runtime_error("Insufficient caplen to process tcp and udp port numbers.");
}

/**
 * @brief Convert protocol number into the string representation.
 * 
 * @param protocol_number 
 * @return std::string 
 */
std::string getProtocolNameByNumber(uint8_t protocol_number)
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
}

/**
 * @brief Extract port numbers from the captured data.
 * 
 * @param cap 
 * @return std::pair<uint16_t, uint16_t> 
 */
std::pair<uint16_t, uint16_t> getPortNumbers(struct capture cap)
{
    checkTcpUdpPorts(cap);

    const udphdr *udp_tcp_header = (const udphdr *)(captureFromPos(cap));
    uint16_t src_port;
    uint16_t dst_port;
    // Save src, dst port
    src_port = ntohs(udp_tcp_header->source);
    dst_port = ntohs(udp_tcp_header->dest);
    return std::pair<uint16_t, uint16_t>(src_port, dst_port);
}


/**
 * @brief Extract string representation of ipv4 source address from the captured data.
 * 
 * @param ip_header 
 * @return std::string 
 */
std::string ipv4SourceAddress(const struct iphdr *ip_header)
{
    char *address;
    struct in_addr ipv4_address;
    char ip4_source[INET_ADDRSTRLEN];

    ipv4_address.s_addr = ip_header->saddr;
    address = inet_ntoa(ipv4_address);
    memcpy(ip4_source, address, 15);
    ip4_source[15] = 0;
    return std::string(ip4_source);
}

/**
 * @brief Extract string representation of ipv4 destination address from the ipv4 header
 * 
 * @param ip_header 
 * @return std::string 
 */
std::string ipv4DestinationAddress(const struct iphdr *ip_header)
{
    char *address;
    struct in_addr ipv4_address;
    char ip4_destination[INET_ADDRSTRLEN];

    ipv4_address.s_addr = ip_header->daddr;
    address = inet_ntoa(ipv4_address);
    memcpy(ip4_destination, address, 15);
    ip4_destination[15] = 0;
    return std::string(ip4_destination);
}


/**
 * @brief Extract protocol name from the ipv4 header.
 * 
 * @param ip_header 
 * @return std::string 
 */
std::string ipv4Protocol(const struct iphdr *ip_header)
{
    return getProtocolNameByNumber(ip_header->protocol);
}

/**
 * @brief Extract total length name from the ipv4 packet;
 * 
 * @param ip_header 
 * @return std::string 
 */
uint16_t ipv4TotalLength(const struct iphdr *ip_header)
{
    return ntohs(ip_header->tot_len);
}


/**
 * @brief Extract string representation of ipv6 source address from the ipv6 header
 * 
 * @param ip_header 
 * @return std::string 
 */
std::string ipv6SourceAddress(const struct ip6_hdr *ip6_header)
{
    char ip6_address[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET6, &(ip6_header->ip6_src), ip6_address, INET6_ADDRSTRLEN) == NULL){
        throw std::runtime_error("Insufficient caplen to process tcp and udp port numbers.");
    };
    return std::string(ip6_address);
}

/**
 * @brief Extract string representation of ipv6 destination address from the ipv6 header
 * 
 * @param ip_header 
 * @return std::string 
 */
std::string ipv6DestinationAddress(const struct ip6_hdr *ip6_header)
{
    char ip6_address[INET6_ADDRSTRLEN];

    if (inet_ntop(AF_INET6, &(ip6_header->ip6_dst), ip6_address, INET6_ADDRSTRLEN) == NULL){
        throw std::runtime_error("Insufficient caplen to process tcp and udp port numbers.");
    };
    return std::string(ip6_address);
}


/**
 * @brief Extract protocol name from the ipv6 header.
 * 
 * @param ip6_header 
 * @return std::string 
 */
std::string ipv6Protocol(const struct ip6_hdr *ip6_header)
{
    return getProtocolNameByNumber(ip6_header->ip6_ctlun.ip6_un1.ip6_un1_nxt);
}

/**
 * @brief Extract total length name from the ipv6 packet;
 * 
 * @param ip_header 
 * @return std::string 
 */
uint16_t ipv6TotalLength(const struct ip6_hdr *ip6_header)
{
    return ntohs(ip6_header->ip6_ctlun.ip6_un1.ip6_un1_plen) + 40;
}

/**
 * @brief Create pair of flow identification and length of data. ((IPsrc:port, IPdst:port), length)
 * 
 * @param cap 
 * @return std::pair<FlowKey, uint16_t> 
 */
std::pair<FlowKey, uint16_t> processIPv4(struct capture cap)
{
    checkIPv4BaseHeader(cap);

    const iphdr *ip_header = (const iphdr *)(captureFromPos(cap));
    std::string source_address = ipv4SourceAddress(ip_header);
    std::string destination_address = ipv4DestinationAddress(ip_header);
    std::string protocol = ipv4Protocol(ip_header);
    uint16_t length = ipv4TotalLength(ip_header);
    std::tuple<uint16_t, uint16_t> src_dst_port = {0, 0}; // by default 0 - unused

    // TCP, UDP
    if ((ip_header->protocol == IPPROTO_TCP) || (ip_header->protocol == IPPROTO_UDP))
    {
        checkIPv4Header(cap, ip_header->ihl);
        skipIPv4Header(&cap, ip_header->ihl);
        src_dst_port = getPortNumbers(cap);
    }

    return {FlowKey(source_address, std::get<0>(src_dst_port), destination_address, std::get<1>(src_dst_port), protocol, IpAddrClass::IPV4), length};
}

/**
 * @brief Create pair of flow identification and length of data. ((IPsrc:port, IPdst:port), length)
 * 
 * @param cap 
 * @return std::pair<FlowKey, uint16_t> 
 */
std::pair<FlowKey, uint16_t> processIPv6(struct capture cap)
{
    checkIPv6BaseHeader(cap);

    const ip6_hdr *ip6_header = (const ip6_hdr *)(captureFromPos(cap));
    std::string source_address = ipv6SourceAddress(ip6_header);
    std::string destination_address = ipv6DestinationAddress(ip6_header);
    std::string protocol = ipv6Protocol(ip6_header); // TODO: extension headers
    uint16_t length = ipv6TotalLength(ip6_header);
    std::tuple<uint16_t, uint16_t> src_dst_port = {0, 0}; // by default 0 - unused

    uint8_t protocol_number = ip6_header->ip6_ctlun.ip6_un1.ip6_un1_nxt;
    if ((protocol_number == IPPROTO_TCP) || (protocol_number == IPPROTO_UDP))
    {
        skipIPv6BaseHeader(&cap);
        src_dst_port = getPortNumbers(cap);
    }

    return {FlowKey(source_address, std::get<0>(src_dst_port), destination_address, std::get<1>(src_dst_port), protocol, IpAddrClass::IPV6), length};
}

/**
 * @brief Processes the captured packet.
 * 
 * Identifies flow the packet belongs to and update the flow table accordingly.
 * 
 * @param args 
 * @param packet_header 
 * @param packet 
 */
void packet_handler(u_char *args, const struct pcap_pkthdr *packet_header, const u_char *packet)
{
    if (args == nullptr)
    {
        // TODO: better termination
        std::cerr << "Error: args passed to the packet_handler is NULL" << std::endl;
        exit(1);
    }
    void **args_ = (void **)args;
    if (args_[0] == nullptr || args_[1] == nullptr)
    {
        std::cerr << "Error: invalid content of args" << std::endl;
        exit(1);
    }
    void *arg1 = args_[0];
    void *arg2 = args_[1];
    FlowTable *table = (FlowTable *) arg1;
    pcap_t* handle = (pcap_t*) arg2;
    if (table == nullptr || handle == nullptr)
    {
        std::cerr << "Error: either of passed arguments is null" << std::endl;
        exit(1);
    }

    std::pair<FlowKey, uint16_t> capture({FlowKey("", 0, "", 0, "", IpAddrClass::IPV4), 0});

    struct capture cap(packet, 0, packet_header->caplen);

    try {
        checkEther(cap);
        struct ether_header *eth_header;
        eth_header = (struct ether_header *)captureFromPos(cap);

        switch (ntohs(eth_header->ether_type))
        {
        case ETHERTYPE_IP:
            skipEther(&cap);
            capture = processIPv4(cap);
            break;
        case ETHERTYPE_IPV6:
            skipEther(&cap);
            capture = processIPv6(cap);
            break;
        default:
            return;
        }
    } catch (const std::runtime_error &err)
    {
        // Skip packets which cannot be analyzed
        return;
    }
    catch (const std::exception &ex)
    {
        // Any other error causes program termination.
        pcap_breakloop(handle);
        return;
    }

    if (capture.first.protocol == "Unknown")
        return;

    // Update the table
    table->addOrUpdateRecord(capture.first, capture.second);
}