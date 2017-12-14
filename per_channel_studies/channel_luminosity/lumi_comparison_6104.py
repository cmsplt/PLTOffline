#Project3/bis: I want to see if by just combining the two previous scripts I will get a programme which runs in reasonable time
#Beg date 24/08/2017



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


print total_rows  
print total_nb4


cal_const_overall=11246./298.4734 #overall calibration constant for luminosity (used for lumi1)

sigma_vis = [338.3976, 325.7006, 296.6242, 256.5519, 260.3086, 280.232, 301.5987, 322.5363, 275.4835, 307.3605, 337.1033, 347.7954, 324.2083, 286.8872, 261.4798, 254.3162]
cal_const = [] #here the single channel cal consts are stored instead (lumi2)
for item in sigma_vis:
    item = 11246/item
    cal_const.append(item)


lumi_arr = np.empty([16, 3564]) #here lumi[ch][bx] for all the channels will be stored
lumi1 = []   #lumis per nibble ch -> bx
lumi2 = []   #lumis per nibble bx -> ch
ratio = []


k=0
for f in files:
    h5file = tables.open_file(f, "r")
    agg_zero = h5file.root.pltaggzero
    for row in agg_zero.iterrows(16,):  #the first nibble is problematic, so I will just discard it
        channel=row["channelid"] 
        #just a way to display the script is running
        k+=1
        if k%10000==0:
            print "This is data_row " + str(k)

    #the following two lines are to fix the data
        binaryData = [struct.pack('i', i) for i in row["data"]]
        fixedData = [struct.unpack('f', b)[0] for b in binaryData]

        for bunch in range(3564):
            ln_arg=(fixedData[bunch])/(4.*4096)
            lumi_arr[channel][bunch]= -np.log(ln_arg)

        if channel==15:

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

        #(2): getting lumi2
            lumi_ch = [] #here we will store the total lumi per channel 
            for ch in range(16):
                if (ch!=0 and ch!=4):
                    tot=0
                    for bx in range(3564):
                        tot+=lumi_arr[ch][bx]
                    cal_tot = tot*cal_const[ch]
                    lumi_ch.append(cal_tot)
            lumi_par_tot2=0
            for element in lumi_ch:
                lumi_par_tot2+=element
            lumi_tot2 = lumi_par_tot2/14.
            lumi2.append(lumi_tot2)
            #print "The lumi2 for this nibble is " + str(lumi_tot2)
            
            #Now let's get the ratio r=lumi2/lumi1
            r=lumi_tot2/lumi_cal
            ratio.append(r)
            #print r

    print "Done with the file"
        
        
print "Now ready to plot"
#Now on to plot index vs luminosity => how the luminosity changes over nibbles
label = ["lumi1", "lumi2"]
x=range(total_nb4)
plt.plot(x,lumi1, "r.")
plt.plot(x,lumi2, "b.")
plt.legend(label, loc = "best", fancybox=True, shadow=True)
plt.show()

#the ratio
z=[1 for res in range(total_nb4)]
label2 = ["lumi2/lumi1", "y=1"]
plt.plot(x,ratio, "g")
plt.plot(x,z, "m")
plt.legend(label2, loc = "best", fancybox=True, shadow=True)
plt.show()

### SECONDARY PARTS ###
#Let's try to plot an histogram to see how spread out around 1 the results are.
plt.hist(ratio, "auto", normed=True)
plt.show()

#plt.hist(ratio, bins=100, normed=True)
print "Program done"
