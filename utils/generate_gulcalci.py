#!/home/omer/jupyter/environment/bin/python

import numpy as np
import pandas as pd

def generate_gulcalci(items_file=r"./input/items.csv", 
                        occurrence_file=r"./static/occurrence.csv", 
                        coverage_file=r"./input/coverages", 
                        out_dir=r"./work/",
                        nSample=2):

    dItems  = pd.read_csv(items_file)
    items = dItems['item_id']
    
    dOcc    = pd.read_csv(occurrence_file)
    events  = dOcc['event_id']

    dCov    = pd.read_csv(coverage_file)

    e = 0
    i = 0

    lines = []
    line = ['event_id', 'item_id', 'sidx', 'loss']
    lines.append(line)
    while e < len(events):
        while i < len(items):
            tiv = dCov.loc[dCov['coverage_id'] == i, 'tiv']
            lines.append(','.join([str(e), str(i), "-3", str(tiv)]))
            lines.append(','.join([str(e), str(i), "-1", str(tiv*0.1*np.random.random())]))
            for s in range(nSample):
                rand_loss = tiv * np.random.random() * 0.1
                lines.append(','.join([str(e), str(i), str(s+1), str(rand_loss)]))
            
            i += 1
        e += 1
    
    with open(out_dir + "gulcalci.csv", "w") as myFile:
        for line in lines:
            myFile.write(line + '\n')

    print ("List written successfully at " + out_dir)
    

            
            
            
            

    
