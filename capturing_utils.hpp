#ifndef CAPTURING_UTILS_HPP
#define CAPTURING_UTILS_HPP

#include <pcap.h>

void packet_handler(u_char *, const struct pcap_pkthdr*, const u_char*);

#endif