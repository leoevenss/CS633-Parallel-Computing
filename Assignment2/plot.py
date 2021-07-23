#!/usr/bin/env python
# coding: utf-8

import pandas as pd
import seaborn as sns
import numpy as np
import matplotlib.pyplot as plt
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

sns.set()

def plotfig(file):
    df = pd.read_csv(file+'.txt')
    df.columns = ["D","P","ppn","mode","time","(P, ppn)"]
    sns.catplot(x="(P, ppn)", y="time", data=df, kind="bar", col="D", hue="mode")
    plt.savefig('plot_' + file + '.jpg')

for i in ['Bcast', 'Reduce', 'Gather', 'Alltoallv']:
    plotfig(i)
