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

    isatop = subprocess.Popen(["sudo", "../isa-top", "-i", "lo", "-s", "b", "-t", "1", "-d", f"./test_results/{pcap_file.split('.')[0].split('/')[1]}_1sec"])
    tcpreplay = subprocess.Popen(["sudo", "tcpreplay", "--intf1=lo", pcap_file])

    
    tcpreplay.wait()
    
    time.sleep(2)
    isatop.terminate()
    
    # process results
    out_files = [file for file in os.listdir(f"./test_results/{pcap_file.split('.')[0].split('/')[1]}_1sec") if file.startswith('out-')]
    for file in out_files:
        os.system(f"sed -i 's/                                                                        //g' ./test_results/{pcap_file.split('.')[0].split('/')[1]}_1sec/{file}")
    
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
