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

#define ETHER_SIZE 14        // octets
#define IPV4_BASE_SIZE 20    // octets
#define IPV6_HEADER_SIZE 40  // octets
#define IPV6_EXT_HEADER_SIZE // octets
#define ETHERNET_PAYLOAD(packet) packet + ETHER_SIZE
#define IPv4_PAYLOAD(ip_packet, ihl) ip_packet + ihl * 4
#define IPv6_PAYLOAD(ip_packet) ip_packet + IPV6_HEADER_SIZE

struct capture
{
    capture(const u_char *data_,
            unsigned int pos_,
            unsigned int caplen_) : data(data_), pos(pos_), caplen(caplen_) {}
    const u_char * data;
    unsigned int pos;
    unsigned int caplen;
};

u_char *capture_from_pos(struct capture cap)
{
    return (u_char *)(cap.data + cap.pos);
}

void check_ether(struct capture cap)
{
    if ((cap.pos + ETHER_SIZE) > cap.caplen)
        throw std::runtime_error("Insufficient caplen to process ether frame.");
}

void skip_ether(struct capture *cap)
{
    cap->pos += ETHER_SIZE;
}

void check_ipv4_base_header(struct capture cap)
{
    if ((cap.pos + IPV4_BASE_SIZE) > cap.caplen)
        throw std::runtime_error("Insufficient caplen to process ipv4 base header.");
}

void check_ipv4_header(struct capture cap, u_char ihl)
{
    if ((cap.pos + ihl * 4) > cap.caplen)
        throw std::runtime_error("Insufficient caplen to process ipv4 header.");
}

void skip_ipv4_header(struct capture *cap, u_char ihl)
{
    cap->pos += ihl * 4;
}

void check_ipv6_base_header(struct capture cap)
{
    if ((cap.pos + IPV6_HEADER_SIZE) > cap.caplen)
        throw std::runtime_error("Insufficient caplen to process ipv6 base header.");
}

void skip_ipv6_base_header(struct capture *cap)
{
    cap->pos += IPV6_HEADER_SIZE;
}

void check_tcp_udp_ports(struct capture cap)
{
    if ((cap.pos + 4) > cap.caplen)
        throw std::runtime_error("Insufficient caplen to process tcp and udp port numbers.");
}


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

std::pair<uint16_t, uint16_t> get_port_numbers(struct capture cap)
{
    check_tcp_udp_ports(cap);

    const udphdr *udp_tcp_header = (const udphdr *)(capture_from_pos(cap));
    uint16_t src_port;
    uint16_t dst_port;
    // Save src, dst port
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
    return ntohs(ip_header->tot_len);
}

std::string ipv6_source_address(const struct ip6_hdr *ip6_header)
{
    char ip6_address[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET6, &(ip6_header->ip6_src), ip6_address, INET6_ADDRSTRLEN) == NULL){
        throw std::runtime_error("Insufficient caplen to process tcp and udp port numbers.");
    };
    return std::string(ip6_address);
}

std::string ipv6_destination_address(const struct ip6_hdr *ip6_header)
{
    char ip6_address[INET6_ADDRSTRLEN];

    if (inet_ntop(AF_INET6, &(ip6_header->ip6_dst), ip6_address, INET6_ADDRSTRLEN) == NULL){
        throw std::runtime_error("Insufficient caplen to process tcp and udp port numbers.");
    };
    return std::string(ip6_address);
}

std::string ipv6_protocol(const struct ip6_hdr *ip6_header)
{
    return get_protocol_name_by_number(ip6_header->ip6_ctlun.ip6_un1.ip6_un1_nxt);
}

uint16_t ipv6_total_length(const struct ip6_hdr *ip6_header)
{
    return ntohs(ip6_header->ip6_ctlun.ip6_un1.ip6_un1_plen);
}

std::pair<FlowKey, uint16_t> process_ipv4(struct capture cap)
{
    check_ipv4_base_header(cap);

    const iphdr *ip_header = (const iphdr *)(capture_from_pos(cap));
    std::string source_address = ipv4_source_address(ip_header);
    std::string destination_address = ipv4_destination_address(ip_header);
    std::string protocol = ipv4_protocol(ip_header);
    uint16_t length = ipv4_total_length(ip_header);
    std::tuple<uint16_t, uint16_t> src_dst_port = {0, 0}; // by default 0 - unused

    // TCP, UDP
    if ((ip_header->protocol == IPPROTO_TCP) || (ip_header->protocol == IPPROTO_UDP))
    {
        check_ipv4_header(cap, ip_header->ihl);
        skip_ipv4_header(&cap, ip_header->ihl);
        src_dst_port = get_port_numbers(cap);
    }

    return {FlowKey(source_address, std::get<0>(src_dst_port), destination_address, std::get<1>(src_dst_port), protocol, IpAddrClass::IPV4), length};
}

std::pair<FlowKey, uint16_t> process_ipv6(struct capture cap)
{
    check_ipv6_base_header(cap);

    const ip6_hdr *ip6_header = (const ip6_hdr *)(capture_from_pos(cap));
    std::string source_address = ipv6_source_address(ip6_header);
    std::string destination_address = ipv6_destination_address(ip6_header);
    std::string protocol = ipv6_protocol(ip6_header); // TODO: extension headers
    uint16_t length = ipv6_total_length(ip6_header);
    std::tuple<uint16_t, uint16_t> src_dst_port = {0, 0}; // by default 0 - unused

    uint8_t protocol_number = ip6_header->ip6_ctlun.ip6_un1.ip6_un1_nxt;
    if ((protocol_number == IPPROTO_TCP) || (protocol_number == IPPROTO_UDP))
    {
        skip_ipv6_base_header(&cap);
        src_dst_port = get_port_numbers(cap);
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
    void **args_ = (void **)args;
    void *arg1 = args_[0];
    void *arg2 = args_[1];
    FlowTable *table = (FlowTable *) arg1;
    pcap_t* handle = (pcap_t*) arg2;

    std::pair<FlowKey, uint16_t> capture({FlowKey("", 0, "", 0, "", IpAddrClass::IPV4), 0});

    struct capture cap(packet, 0, packet_header->caplen);

    try {
        // Check whether whole MAC header is contained
        check_ether(cap);
        struct ether_header *eth_header;
        eth_header = (struct ether_header *)capture_from_pos(cap);

        switch (ntohs(eth_header->ether_type))
        {
        case ETHERTYPE_IP:
            skip_ether(&cap);
            capture = process_ipv4(cap);
            break;
        case ETHERTYPE_IPV6:
            skip_ether(&cap);
            capture = process_ipv6(cap);
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
    table->add_or_update_record(capture.first, capture.second);
}


