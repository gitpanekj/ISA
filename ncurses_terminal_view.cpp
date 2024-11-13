/**
 * @file ncurses_terminal_view.cpp
 * @author Jan Pánek (xpanek11@stud.fit.vut.cz)
 * @brief Implementation of ncurses terminal interface.
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "flow_table.hpp"
#include "ncurses_terminal_view.hpp"

#include <ncurses.h>
#include <tuple>
#include <string>
#include <fstream>
#include <cmath>
#include <iostream>

/* Capture Table

Src IP:port   Dst IP:port           Proto         Rx                Tx
                                            b/s      p/s      b/s     p/s
                                    tcp     999.9U   999.9U   999.9U  999.9U
                                    udp     999.9U   999.9U   999.9U  999.9U
                                    icmp    999.9U   999.9U   999.9U  999.9U
                                    icmp6   999.9U   999.9U   999.9U  999.9U
<-----...-----><-><-----...-----><-><---><-><----><-><----><-><----><-><---->
(width - 48)/2  3 (width - 48)/2  3   5   3    6   3    6   3    6   3    6    WIDTHS NOTE: -47 + -1 cause starts at 1 form the left

IPv4:port -> 255.255.255.255:65535 -> max width 21
[IPv6]:port -> [ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff]:65535 -> max width 47
NOTE: Columns proto, Rx, Tx have static size until the total window size decreases below 41
NOTE: Src and Dst columns have width (width - 47)/2 - only part of the string is printed if necessary
NOTE: if (width - 48) <= 1 -> dont display Src, Dst part
NOTE: if width < 42 dont show proto
NOTE: if width < 34 dont show proto and Rx
NOTE: if width < 16 dont show proto and Tx
*/


// Macros defining row layout for different screen widths
//  SRC      DST     Proto     Rx            Tx
#define SRC_DST_PROTO_RX_TX "%-*.*s  %-*.*s   %-5s   %-6s   %-6s   %-6s   %-6s"
#define PROTO_RX_TX "%-*.*s%-*.*s%-5s   %-6s   %-6s   %-6s   %-6s"
#define RX_TX "%-*.*s%-*.*s%.0s%.-6s   %-6s   %-6s   %-6s"
#define TX "%-*.*s%-*.*s%.0s%.0s%.0s%-6s   %-6s"
#define CLEAR "%-*.*s%-*.*s%.0s%.0s%.0s%.0s"

/**
 * @brief Write current ncurses buffer to the file.
 * 
 * @param out output directory path
 */
void writeWindowToFile(const std::string &out)
{
    static int cnt = 0;
    std::string filename = out + "/" + "out-" + std::to_string(cnt) + ".txt";
    cnt++;
    std::ofstream outfile(filename, std::ios::app);
    if (!outfile)
    {
        return;
    }

    char buffer[2048];
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    for (int i = 0; i < rows; ++i)
    {
        mvwinstr(stdscr, i, 0, buffer);
        outfile << buffer << std::endl;
    }
    outfile.close();
}

/**
 * @brief Convert number of captured bytes in period in number of bits per second.
 * 
 * @param bytes number of captured bytes in period
 * @param period period length
 * @return double bandwidth
 */
double toBitsPerSecond(unsigned long long bytes, unsigned int period)
{
    return bytes * 8.0 / (double)period;
}

/**
 * @brief Convert number of captured packets in period in number of packets per second.
 * 
 * @param bytes number of captured bytes in period
 * @param period period length
 * @return double bandwidth
 */
double toPacketsPerSecond(unsigned long long packets, unsigned int period)
{
    return (double)packets / (double)period;
}

/**
 * @brief Format measured bandwidth into human readable format
 * 
 * @param bandwidth number of bits per second
 * @return std::string 
 */
std::string toOrderOfMagnitudeFormat(double bandwidth)
{
    const char *orders_of_magnitude[] = {"", "K", "M", "G", "T", "P"};
    int order = 0;
    while (bandwidth >= 1000.0)
    {
        bandwidth = bandwidth / 1000.0;
        order++;
    }

    int int_val = std::round(bandwidth * 10.0);
    if (int_val == 0)
        return "0";

    std::string str_val = std::to_string(int_val);
    std::string str_order = order < 6 ? orders_of_magnitude[order] : "X"; // Orders greater than petabytes are ignored

    
    if (str_val.size() == 4) // xxx.x[KMGTP]
    {
        return str_val.substr(0, 3) + "." + str_val.substr(3,1) + str_order;
    }
    else if (str_val.size() == 3) // xx.x[KMGTP]
    {
        return str_val.substr(0, 2) + "." + str_val.substr(2,1) + str_order;
    }
    else if (str_val.size() == 2) // x.x[KMGTP]
    {
        return str_val.substr(0,1) + "." + str_val.substr(1,1) + str_order;
    }
    else if (str_val.size() == 1) // 0.x
    {
        return "0." + str_val;
    }
    else                          // 0.0  
    {
        return "0.0";
    }
}

/**
 * @brief 
 * 
 * @param record 
 * @return std::tuple<std::string, std::string> 
 */
std::tuple<std::string, std::string> toAddressColumnFormat(FlowKey record)
{
    std::string src;
    std::string dst;

    if (record.ip == IpAddrClass::IPV4)
    {
        src = record.src_address + (record.src_port != 0 ? (":" + std::to_string(record.src_port)) : "");
        dst = record.dst_address + (record.dst_port != 0 ? (":" + std::to_string(record.dst_port)) : "");
    }
    else
    { // IPv6
        src = "[" + record.src_address + "]" + (record.src_port != 0 ? (":" + std::to_string(record.src_port)) : "");
        dst = "[" + record.dst_address + "]" + (record.dst_port != 0 ? (":" + std::to_string(record.dst_port)) : "");
    }

    return {src, dst};
}

/**
 * @brief Print body of the bandwidth table containing top 10 most communication flows
 * 
 * @param records list of top ten communicating flows
 * @param fmt print format
 * @param src_dst_width width of address column
 * @param period capture period
 */
void printRecords(std::list<std::pair<FlowKey, FlowStats>> records, const char *fmt, int src_dst_width, unsigned int period)
{
    int line = 3; // first two rows are header
    for (auto it = records.rbegin(); it != records.rend(); it++) // from max to min
    {
        std::tuple<std::string, std::string> addresses = toAddressColumnFormat(it->first);

        mvprintw(line, 1, fmt,
                 src_dst_width, src_dst_width, std::get<0>(addresses).c_str(),
                 src_dst_width, src_dst_width, std::get<1>(addresses).c_str(),
                 (it->first.protocol).c_str(),
                 (toOrderOfMagnitudeFormat(toBitsPerSecond(it->second.rx_bytes, period))).c_str(),
                 (toOrderOfMagnitudeFormat(toPacketsPerSecond(it->second.rx_packets, period))).c_str(),
                 (toOrderOfMagnitudeFormat(toBitsPerSecond(it->second.tx_bytes, period))).c_str(),
                 (toOrderOfMagnitudeFormat(toPacketsPerSecond(it->second.tx_packets, period))).c_str());
        line++;
    }
}

/**
 * @brief Print table header
 * 
 * @param fmt print format
 * @param src_dst_width width of address column
 */
void printHeader(const char *fmt, int src_dst_width)
{

    mvprintw(0, 1, fmt, src_dst_width, src_dst_width, "Src IP:port",
             src_dst_width, src_dst_width, "Dst IP:port",
             "Proto",
             "Rx", "", "Tx", "");
    mvprintw(1, 1, fmt, src_dst_width, src_dst_width, "",
             src_dst_width, src_dst_width, "",
             "",
             "b/s", "p/s", "b/s", "p/s");
}

/**
 * @brief Print header and table body.
 * 
 * @param records list of top ten communicating flows
 * @param fmt print format
 * @param src_dst_width width of address column
 * @param period capture period
 */
void printTable(std::list<std::pair<FlowKey, FlowStats>> records, const char *fmt, int src_dst_width, unsigned int period)
{
    printHeader(fmt, src_dst_width);
    printRecords(records, fmt, src_dst_width, period);
}


/**
 * @brief Update ncurses view with table.
 * 
 * @param records list of top ten communicating flows
 * @param period capture period
 */
void updateView(std::list<std::pair<FlowKey, FlowStats>> records, unsigned int period)
{
    clear();
    int screen_width = getmaxx(stdscr);
    if (screen_width < 16) // empty
    {
        printTable(records, CLEAR, 0, period);
    }
    else if (screen_width < 34) // RX
    {
        printTable(records, TX, 0, period);
    }
    else if (screen_width < 42) //  TX RX
    {
        printTable(records, RX_TX, 0, period);
    }
    else if ((screen_width - 48) / 2 < 2) //  PROTO TX RX
    {
        printTable(records, PROTO_RX_TX, 0, period);
    }
    else // Full
    {
        printTable(records, SRC_DST_PROTO_RX_TX, (screen_width - 48) / 2, period);
    }
    refresh();
}

/**
 * @brief Initialize ncurses.
 * 
 * @return int 
 */
int startUI()
{
    initscr();
    noecho();
    curs_set(0);

    return 0;
}

/**
 * @brief Release ncurses resources.
 * 
 * @return int 
 */
int stopUI()
{
    endwin();

    return 0;
}