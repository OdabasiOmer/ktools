#!/home/omer/jupyter/environment/bin/python

import numpy as np
import pandas as pd
import os
import subprocess

def generate_gulcalci(items_file=r"./input/items.csv", 
                        occurrence_file=r"./static/occurrence.csv", 
                        coverage_file=r"./input/coverages.csv", 
                        out_dir=r"./work/",
                        nSample=2):

    dItems  = pd.read_csv(items_file)
    items = dItems['item_id'].to_list()
    
    dOcc    = pd.read_csv(occurrence_file)
    events  = dOcc['event_id'].to_list()

    dCov    = pd.read_csv(coverage_file)
    coverages = dCov['coverage_id'].to_numpy(dtype=int)
    tivs = dCov['coverage_id'].to_numpy(dtype=float)
    
    lines = []
    lines.append(','.join(['event_id', 'item_id', 'sidx', 'loss']) + '\n')

    idx = 0

    for e in events:
        print("Event: " + str(e))
        for i in items:
            #print(i)
            idx = np.where(coverages == i)[0][0]
            #print(idx)
            tiv = tivs[idx]
            lines.append(','.join([str(e), str(i), "-3", str(tiv)]) + '\n')
            lines.append(','.join([str(e), str(i), "-1", str(tiv*0.1*np.random.random())]) + '\n')
            for s in range(nSample):
                rand_loss = tiv * np.random.random() * 0.1
                lines.append(','.join([str(e), str(i), str(s+1), str(rand_loss)]) + '\n')
    
    
    with open(out_dir + "gulcalci.csv", "w") as myFile:
        for line in lines:
            myFile.write(line)

    # Showcase list
    xx = 0
    while xx < 10:
        print(lines[xx])
        xx += 1
    
    # Done
    print ("List written successfully at " + out_dir)

def generate_fmcalci(gul_file_path):
    procs = []

    cmd="gultobin < %s.csv > %s.bin " % (gul_file_path, gul_file_path)
    print(cmd)
    p1 = subprocess.Popen(cmd,shell=True)
    procs.append(p1)

    cmd="fmcalc < %s.bin > ./work/fmcalci.bin " % (gul_file_path)
    print(cmd)
    p1 = subprocess.Popen(cmd,shell=True)
    procs.append(p1)

    cmd="fmtocsv < ./work/fmcalci.bin > ./work/fmcalci.csv "
    print(cmd)
    p1 = subprocess.Popen(cmd,shell=True)
    procs.append(p1)

    for p in procs:
	    p.wait()
    

  
gul_file_dir = r'./work/gulcalci.csv'

generate_gulcalci(gul_file_dir)
generate_fmcalci(gul_file_dir)


            
            
            
            

    
