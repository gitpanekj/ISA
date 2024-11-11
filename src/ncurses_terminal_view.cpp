#include "flow_table.hpp"
#include "ncurses_terminal_view.hpp"

#include <ncurses.h>
#include <tuple>
#include <string>
#include <fstream>

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

//  SRC      DST     Proto     Rx            Tx
#define SRC_DST_PROTO_RX_TX "%-*.*s  %-*.*s   %-5s   %-6s   %-6s   %-6s   %-6s"
#define PROTO_RX_TX "%-*.*s%-*.*s%-5s   %-6s   %-6s   %-6s   %-6s"
#define RX_TX "%-*.*s%-*.*s%.0s%.-6s   %-6s   %-6s   %-6s"
#define TX "%-*.*s%-*.*s%.0s%.0s%.0s%-6s   %-6s"
#define CLEAR "%-*.*s%-*.*s%.0s%.0s%.0s%.0s"

int cnt = 0;

void write_window_to_file(WINDOW *win, const std::string &filename) {
    std::ofstream outfile(filename, std::ios::app);
    if (!outfile) {
        return;
    }

    char buffer[2048];
    int rows, cols;
    getmaxyx(win, rows, cols);

    // Write each line to the file
    for (int i = 0; i < rows; ++i) {
        mvwinstr(win, i, 0, buffer);
        outfile << buffer << std::endl;
    }

    outfile << "----------\n";  // Separator between captures
    outfile.close();
}

std::string add_order_of_magnitude(std::string num)
{
    const char* orders_of_magnitude[] = {"","k", "M", "G", "T", "P"};
    if (num.size() == 0) num = "0";

    int order = (num.size() - 1) / 3;
    std::string str_order = order < 6 ? orders_of_magnitude[order] : "X";

    if (order == 0)
    {
        return num;
    }
    else
    {
        std::string whole_part = num.substr(0, (num.size()-order*3));
        std::string decimal_part = num.substr(num.size()-order*3, 1);
        return whole_part + "." + decimal_part + str_order;
    }
}


std::tuple<std::string, std::string> get_address_and_port(FlowKey record)
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

void print_records(std::list<std::pair<FlowKey, FlowStats>> records, const char *fmt, int src_dst_width)
{
    int line = 3;
    for (auto it = records.rbegin(); it != records.rend(); it++)
    {
        std::tuple<std::string, std::string> addresses = get_address_and_port(it->first);

        mvprintw(line, 1, fmt,
                 src_dst_width, src_dst_width, std::get<0>(addresses).c_str(),
                 src_dst_width, src_dst_width, std::get<1>(addresses).c_str(),
                 (it->first.protocol).c_str(),
                 add_order_of_magnitude(std::to_string(it->second.rx_bytes)).c_str(),
                 add_order_of_magnitude(std::to_string(it->second.rx_packets)).c_str(),
                 add_order_of_magnitude(std::to_string(it->second.tx_bytes)).c_str(),
                 add_order_of_magnitude(std::to_string(it->second.tx_packets)).c_str());
        line++;
    }
}

void print_header(const char *fmt, int src_dst_width)
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

void print_table(std::list<std::pair<FlowKey, FlowStats>> records, const char *fmt, int src_dst_width)
{
    print_header(fmt, src_dst_width);
    print_records(records, fmt, src_dst_width);
}

void update_view(std::list<std::pair<FlowKey, FlowStats>> records)
{
    clear();
    int screen_width = getmaxx(stdscr);
    if (screen_width < 16) // empty
    {
        print_table(records, CLEAR, 0);
    }
    else if (screen_width < 34) // RX
    {
        print_table(records, TX, 0);
    }
    else if (screen_width < 42) //  TX RX
    {
        print_table(records, RX_TX, 0);
    }
    else if ((screen_width - 48) / 2 < 2) //  PROTO TX RX
    {
        print_table(records, PROTO_RX_TX, 0);
    }
    else // Full
    {
        print_table(records, SRC_DST_PROTO_RX_TX, (screen_width - 48) / 2);
    }
    refresh();

    std::string filename = "out-" + std::to_string(cnt++) + ".txt";
    write_window_to_file(stdscr, filename);
}

int start_ui()
{
    initscr();
    noecho();
    curs_set(0);

    return 0;
}

int stop_ui()
{
    endwin();

    return 0;
}