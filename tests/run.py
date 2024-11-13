import os


os.system("find test_results/* -type f -name '*.txt'  -delete")


files = os.listdir('captures')
for file in files:
    for period in [1,2]:
        # 1,2 sec pcap processing
        os.system(f"python capture_test.py captures/{file} {period} > ./test_results/{file.split('.')[0]}_{period}sec/{file.split('.')[0]}_{period}.txt")
        
    # 2 sec isatop + if top
    os.system(f"sudo python iftop_compare_test.py captures/{file}")
    
    # 1 sec isatop
    os.system(f"sudo python isatop_single.py captures/{file}")
        
