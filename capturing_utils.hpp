/**
 * @file capturing_utils.hpp
 * @author Jan Pánek (xpanek11@stud.fit.vut.cz)
 * @brief Utilities for processing of captured packet.
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef CAPTURING_UTILS_HPP
#define CAPTURING_UTILS_HPP

#include <pcap.h>

void packet_handler(u_char *, const struct pcap_pkthdr*, const u_char*);

#endif