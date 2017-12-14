#Project4/bis: We want to see how much every single working channel influences the final results for lumi2
#Beg date 01/09/2017 (?)


import struct
import tables
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from os import listdir
from os.path import isfile, join

import sys

total_rows= 0
#This following part will loop over all the files in a given directory - the one corresponding to the fill
dir = "./6104/"
files = [join(dir, f) for f in listdir(dir) if isfile(join(dir, f))]
nfiles = len(files)

for f in files:
    h5file = tables.open_file(f, "r")
    agg_zero = h5file.root.pltaggzero
    n_rows = agg_zero.nrows
    total_rows += n_rows
total_nb4 = (total_rows/16)-nfiles   #from each file i discard the first nibble, so that's "number" nibbles less than the total


lumi_arr = np.empty([16, 3564]) #here lumi[ch][bx] for all the channels will be stored
sigma_vis = [338.3976, 325.7006, 296.6242, 256.5519, 260.3086, 280.232, 301.5987, 322.5363, 275.4835, 307.3605, 337.1033, 347.7954, 324.2083, 286.8872, 261.4798, 254.3162]
cal_const = [] #here the single channel cal consts are stored instead (lumi2)
for item in sigma_vis:
    item = 11246/item
    cal_const.append(item)

print total_rows
print total_nb4

lumi2 = []   #lumis per nibble ch -> bx
channel_excl = []


k=0
for f in files:
    h5file = tables.open_file(f, "r")
    agg_zero = h5file.root.pltaggzero

    for row in agg_zero.iterrows(16,):  #the first nibble is problematic, so I will just discard it
        channel=row["channelid"] 
        k+=1
        if k%10000==0:
            print "This is data_row" + str(k)

    #the following two lines are to fix the data
        binaryData = [struct.pack('i', i) for i in row["data"]]
        fixedData = [struct.unpack('f', b)[0] for b in binaryData]

        for bunch in range(3564):
            ln_arg=(fixedData[bunch])/(4.*4096)
            lumi_arr[channel][bunch]= -np.log(ln_arg)

        if channel==15:

        #(2): getting lumi2
            lumi_ch = [] #here we will store the total lumi per channel 
            for ch in range(16):
                if (ch!=0 and ch!=4):
                    tot=0
                    for bx in range(3564):
                        tot+=lumi_arr[ch][bx]
                    cal_tot = tot*cal_const[ch]
                    lumi_ch.append(cal_tot)
            lumi_par_tot=0
            for element in lumi_ch:
                lumi_par_tot+=element
            lumi_tot = lumi_par_tot/14.
            lumi2.append(lumi_tot)
            #print "The lumi2 for this nibble is " + str(lumi_tot)
        
            storage = []
            for ch_counter in range(16):
                if ch_counter!=0 and ch_counter!=4:
                     lumi_ch = [] #here we will store the total lumi per channel 
                     for ch in range(16):
                         if (ch!=0 and ch!=4 and ch!=ch_counter):
                             tot=0
                             for bx in range(3564):
                                 tot+=lumi_arr[ch][bx]
                             cal_tot = tot*cal_const[ch]
                             lumi_ch.append(cal_tot)
                     lumi_par_tot2=0
                     for element in lumi_ch:
                         lumi_par_tot2+=element
                     lumi_tot2 = lumi_par_tot2/13.
                     #print lumi_tot
                     #print lumi_tot2
                     storage.append(lumi_tot/lumi_tot2)
            channel_excl.append(storage)
            #print channel_excl

    print "Done with the file"


print "Now ready to plot"
#this will set different colors in the same plot
num_plots = 14
colormap = plt.cm.gist_ncar
plt.gca().set_prop_cycle(plt.cycler('color', plt.cm.gist_ncar(np.linspace(0, 0.9, num_plots))))


x=range(total_nb4)
labels=[["ch1", "ch2", "ch3", "ch5", "ch6"],[ "ch7", "ch8", "ch9", "ch10", "ch11"],[ "ch12", "ch13", "ch14", "ch15"]]

lgd=0
for alpha in range(14):
    y=[]
    for beta in range(total_nb4):
        y.append(channel_excl[beta][alpha])
    plt.plot(x,y)
    if alpha==4 or alpha==9 or alpha==13:
        plt.legend(labels[lgd], loc = "best", fancybox=True, shadow=True)
        plt.show()
        lgd+=1

#plt.show()

