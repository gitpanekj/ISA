import pyshark
from datetime import datetime, timedelta
import math
import sys
from typing import Optional, Sequence

def to_oder_of_magnitude_format(number):
    orders = ["", "K", "M", "G", "T", "P"]
    order = 0
    while number >= 1000:
        number /= 1000.0
        order += 1
    str_order = orders[order] if order < 6 else "X"

    return f"{number:.1f}{str_order}"


def parse_packet(packet):
    src_ip = ""
    src_port =  0
    dst_ip = ""
    dst_port = 0
    protocol = ""
    no_bytes = 0

    try:
        # Process IP
        ip = None
        if 'IP' in packet:
            ip = packet.ip
        elif 'IPV6' in packet:
            ip = packet.ipv6 
        else:
            # skip other that ipv4, ipv6
            return None

        src_ip = ip.src
        dst_ip = ip.dst
        

        if 'TCP' in packet:
            protocol = 'TCP'
            src_port = packet.tcp.srcport
            dst_port = packet.tcp.dstport
        elif 'UDP' in packet:
            protocol = 'UDP'
            src_port = packet.udp.srcport
            dst_port = packet.udp.dstport     
        elif 'ICMP' in packet:
            protocol = 'ICMP'
        elif 'ICMPV6' in packet:
            protocol = 'ICMPv6'

            
        capture_timestamp = datetime.fromtimestamp(float(packet.sniff_timestamp)).replace(microsecond=0)
        no_bytes = int(packet.length) - 14 if int(packet.length) - 14 >= 0 else 0 # minus ether header
        flow_key = (src_ip, src_port, dst_ip, dst_port, protocol)
        swapped_flow_key = (dst_ip, dst_port,src_ip, src_port, protocol)

        return (capture_timestamp, flow_key, swapped_flow_key, no_bytes)
    except AttributeError as e:
        return None
        
def parse_capture(file):
    stats = dict()
    
    capture = pyshark.FileCapture(file)
    
    for packet in capture:
        
        result = parse_packet(packet)
        if result == None:
            continue
        
        (capture_timestamp, flow_key,swapped_flow_key, no_bytes) = result
        
        window = stats.get(capture_timestamp, None)
        if (not window):
            window = dict()
            stats[capture_timestamp] = window
        
        flow = window.get(flow_key, None)
        if (flow):
            window[flow_key]["tx_bytes"] += no_bytes
            window[flow_key]["tx_packets"] += 1
            continue
        
        flow = window.get(swapped_flow_key)    
        if (flow):
            window[swapped_flow_key]["rx_bytes"] += no_bytes
            window[swapped_flow_key]["rx_packets"] += 1
            continue
        
        window[flow_key] = {"rx_bytes": 0, "rx_packets":0, "tx_bytes": no_bytes, "tx_packets":1}

    capture.close()
    return stats

def display(stats, period=1):
    # process period
    if period != 1:
        start = min(list(stats.keys()))
        end = max(list(stats.keys()))
        step = timedelta(seconds=period)
        nwins = int(math.ceil((end - start).seconds / period))
        

        
        windows = [[value for (key, value) in stats.items() if ( (start + i*step) <= key and key <= (start + (i+1)*step))] for i in range(0, nwins)]

        stats = dict()
        
        for (i, window) in enumerate(windows):
            stats[i] = {}

            for sec_win in window:

                for (flow_key, data) in sec_win.items():

                    flow = stats[i].get(flow_key, None)
                    if (flow):
                        stats[i][flow_key]["tx_bytes"] += data["tx_bytes"]
                        stats[i][flow_key]["tx_packets"] += data['tx_packets']
                        continue
                    swapped_flow_key = (flow_key[2], flow_key[3], flow_key[0], flow_key[1], flow_key[4])
                    flow = stats[i].get(swapped_flow_key, None)    
                    if (flow):
                        stats[i][swapped_flow_key]["rx_bytes"] += data['rx_bytes']
                        stats[i][swapped_flow_key]["rx_packets"] += data['rx_packets']
                        continue
                    
                    stats[i][flow_key] = {"rx_bytes": data['rx_bytes'], "rx_packets":data['rx_packets'], "tx_bytes": data['tx_bytes'], "tx_packets":1}
        
    for key in sorted(stats.keys()):
        print(f"=========================={key}=============================")
        for flow_key, data in stats[key].items():
            rbps = to_oder_of_magnitude_format(data['rx_bytes'] * 8 / period)
            tbps = to_oder_of_magnitude_format(data['tx_bytes'] * 8 / period)
            rpps = to_oder_of_magnitude_format(data['rx_packets'] / period)
            tpbs = to_oder_of_magnitude_format(data['tx_packets'] / period)
            print(f"{flow_key=} {rbps=} {tbps=} {rpps=} {tpbs=}")
        print("=============================================================")
        
def main(argv: Optional[Sequence[str]] = None) -> int:
    try:
        file = argv[1]
        period = argv[2]
    except:
        print("run as capture_test.py pcapfile period")
        return 1
    stats = parse_capture(file)
    display(stats, int(period))
    
    return 0


if __name__ == "__main__":  
    sys.exit(main(sys.argv))

