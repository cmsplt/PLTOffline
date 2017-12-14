#Project4: Now we want to see how much every single working channel influences the final results for lumi1 
#Beg date 31/08/2017


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
cal_const_overall=11246./298.4734 #overall calibration constant for luminosity (used for lumi1)


print total_rows  
print total_nb4

lumi1 = []   #lumis per nibble ch -> bx
channel_excl=[]


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

        if channel==15: #that is, at the end of one nb4

        #(1): getting lumi1
            lumi_bx =[] #here we will store the total lumi per bunch
            for bx in range(3564):
                par_total=0
                for ch in range(16):
                    if (ch!=0 and ch!=4):
                        par_total+=lumi_arr[ch][bx]
                total = par_total/14.
                lumi_bx.append(total)
            lumi_tot1=0
            for item in lumi_bx:
                lumi_tot1+=item
            lumi_cal=lumi_tot1*cal_const_overall
            lumi1.append(lumi_cal)
            #print "The lumi1 for this nibble is " + str(lumi_cal)

        #Now I need to calculate the lumi1 excluding the channels one by one
            storage = []
            for ch_counter in range(16):
                if ch_counter!=0 and ch_counter!=4:
                    lumi_bx =[] #here we will store the total lumi per bunch
                    for bx in range(3564):
                        par_total=0
                        for ch in range(16):
                            if (ch!=0 and ch!=4 and ch!=ch_counter):
                                par_total+=lumi_arr[ch][bx]
                        total = par_total/13.
                        lumi_bx.append(total)
                    lumi_tot1=0
                    for item in lumi_bx:
                        lumi_tot1+=item
                    #print "Hey this is "+ str(lumi_cal)
                    lumi_cal2=lumi_tot1*cal_const_overall
                    storage.append(lumi_cal/lumi_cal2)
                    #print lumi_cal2
            channel_excl.append(storage)
            #print channel_excl
        
    print "Done with the file"
        

print "Now ready to plot"
#this will set different colors in the same plot
#num_plots = 14
#colormap = plt.cm.gist_ncar
#plt.gca().set_prop_cycle(plt.cycler('color', plt.cm.gist_ncar(np.linspace(0, 0.9, num_plots))))


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

