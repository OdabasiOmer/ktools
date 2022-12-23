#!bin/python3

import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

filename = 'gL0.csv'
outdir = r"./out/"

def check_consistency(df):
    events = set(df['event_id'])
    items = set(df['item_id'])
    diff = []
    for e in events:
        for i in items:
            rows = df.loc[(df['item_id'] == i) & 
            (df['event_id'] == e)]

            if ~rows['loss'].any():
                continue

            r_loss_s = rows.loc[(df['sidx'] > 0)]['loss']
            
            #1
            actual_savg = np.mean(r_loss_s.iloc[:])
            
            #2
            integration_avg = float(rows.loc[(df['sidx'] == -1)]['loss'])

            diff.append(actual_savg - integration_avg)

    x = range(0,len(diff), 1)

    # Scatter of the residuals = sampled average - integration average
    fig = plt.figure()
    ax = fig.add_subplot()
    ax.scatter(x, diff)
    ax.plot([0, len(diff)], [0,0], color='black', linewidth=2.0)
    ax.set_title('Event loss residuals')
    ax.set_xlabel('Event ID')
    ax.set_ylabel('Residual (sampled avg - integration avg')
    plt.savefig(os.path.join(outdir, filename[:-4]) + '_residuals.png', dpi=450,format='png')

    # Over vs under estimation
    npos = sum(map(lambda x : x > 0, diff))
    nneg = sum(map(lambda x : x < 0, diff))

    fig = plt.figure()
    ax = fig.add_subplot()
    ax.bar(x=['Under', 'Over'], height=[nneg, npos])
    ax.set_ylabel('Number of times')
    plt.savefig(os.path.join(outdir, filename[:-4]) + '_bars.png', dpi=450,format='png')


if __name__=='__main__':
    df = pd.read_csv(r"./out/" + filename)
    check_consistency(df)