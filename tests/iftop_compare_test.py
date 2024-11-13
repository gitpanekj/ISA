import subprocess
from typing import Optional, Sequence
import sys
import os
import time

def main(argv: Optional[Sequence[str]] = None) -> int:
    pcap_file = ""
    try:
        pcap_file = argv[1]
    except:
        print("Error: unspecified capture file")
        return 1

    with open(f"./test_results/{pcap_file.split('.')[0].split('/')[1]}_2sec/out.txt", "w") as f:
        iftop = subprocess.Popen(["sudo", "iftop", "-i", "lo", "-n", "-N", "-p", "-P", "-o" ,"2s", "-t"], stdout=f)
    isatop = subprocess.Popen(["sudo", "../isa-top", "-i", "lo", "-s", "b", "-t", "2", "-d", f"./test_results/{pcap_file.split('.')[0].split('/')[1]}_2sec"])
    time.sleep(1.95)
    tcpreplay = subprocess.Popen(["sudo", "tcpreplay", "--intf1=lo", pcap_file])

    tcpreplay.wait()
    
    time.sleep(4)
    iftop.terminate()
    isatop.terminate()
    
    # process results
    out_files = [file for file in os.listdir(f"./test_results/{pcap_file.split('.')[0].split('/')[1]}_2sec") if file.startswith('out-')]
    for file in out_files:
        os.system(f"sed -i 's/                                                                        //g' ./test_results/{pcap_file.split('.')[0].split('/')[1]}_2sec/{file}")
    
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))