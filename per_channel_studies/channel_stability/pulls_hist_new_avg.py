#Project 7/bis: The script will print histograms of pulls for all the channels, this time using an average calculated individually for every channel and over the previous 25 NB4s.
#Beg. date: 06/10/2017


import struct
import tables
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
import numbers
from scipy.stats import norm

from os import listdir
from os.path import isfile, join

import sys


zero_test = 0

total_rows = 0
#This floowing part makes it possible to just write the fill number on the command line
if (len(sys.argv)<2):
    print "Usage: "+sys.argv[0]+" fillNumber"
    sys.exit(1)
nfill= int(sys.argv[1])

#This following part will loop over all the files in a given directory - the one corresponding to the fill
dir = "./%d/" %nfill
files = [join(dir, f) for f in listdir(dir) if isfile(join(dir, f))]
nfiles = len(files)


for f in files:
    h5file = tables.open_file(f, "r")
    agg_zero = h5file.root.pltaggzero
    n_rows = agg_zero.nrows
    total_rows += n_rows
total_nb4 = (total_rows/16)

print "There are %i files with %i rows overall, which means there are %i nibbles in total." %(nfiles, total_rows, total_nb4)

#this is just a way to make sure that there are no partially incomplete nibbles
if total_nb4 == total_rows/16.:
    print "There are no incomplete nibbles, go on with file analysis"
else:
    print "There is some sort of problem related to some incomplete nb4"

avg_list = []
hist_sample = []


k=0
for f in files:
    h5file = tables.open_file(f, "r")
    agg_zero = h5file.root.pltaggzero
    total_bx = []
    for row in agg_zero.iterrows():
        #This is just a quick way to check that the script is running smoothly
        channel=row["channelid"] 
        k+=1
        if k%10000==0:
            print "This is data_row " + str(k)

        if channel!=0 and channel!=4:
            if nfill <= 6156:
                #next two lines to fix the data
                binaryData = [struct.pack('i', i) for i in row["data"]]
                fixedData = [struct.unpack('f', b)[0] for b in binaryData]
                
                total = 0
                for bunch in range(3564):
                    fixedData[bunch]=(4*4096.)-(fixedData[bunch]) #the no of zeroes has to be converted into the no of hits
                    total += fixedData[bunch]
                avg_list.append(total)

            else:
                #no need to fix data for fills after fill 6156
                data = row["data"]

                total = 0
                for bunch in range(3564):
                    data[bunch]=(4*4096.)-(data[bunch]) #the no of zeroes has to be converted into the no of hits
                    total += data[bunch]
                avg_list.append(total)


        if channel == 15:
            if len(avg_list)==(25*14):   #it has to store 14 channel values for 25 nb4s, so total 25*14=350
                channel_list = []
                for ch in range(14):
                    sublist = [s for s in avg_list[ch:350:14]] #list with values  for each channel
                    #print "The length of sublist for channel %i is " %ch + str(len(sublist))
                    sum=0
                    for item in sublist:
                        sum+=item
                    N=sum/25.
                    if N==0: #The nibbles where <N>=0 cannot be properly normalized
                        zero_test+=1
                    else:
                        x=sublist[len(sublist)-1]
                        if x>0:
                            bx_norm = (x-N)/(x**.5) #the last element of sublist is the one of the nb4 to be normalized
                            channel_list.append(bx_norm)
                if len(channel_list)==14:
                    hist_sample.append(channel_list)
                else:
                    zero_test+=1
                del avg_list[0:14]

                #print N
                #print "Channel list is  " + str(channel_list)
            
    print "Done with the file, onto the following one"

print "There are %i problematic nibbles that were discarded." %zero_test


ch=1
for channel in range(14):    
    ch_sample = []
    for item in hist_sample:
        ch_sample.append(item[channel])
    #histogram of the rates of the channel
    n, bins, patches = plt.hist(ch_sample, bins = 50000)

    #best Gaussian fit
    (mu, sigma) = norm.fit(ch_sample)
    #y = mlab.normpdf(bins, mu, sigma)
    #plt.plot(bins, y, "r--")

    #plot features
    plt.xlabel("Pulls")
    plt.ylabel("Occourences")
    if ch>=4:
        ch_bis=ch+1
        plt.title(r"$\mathrm{Pull\ values\ for\ channel\ %i \ in\ fill\ %i}\ (\mu=%.3f,\ \sigma=%.3f)$" %(ch_bis, nfill, mu, sigma))
    else:
        plt.title(r"$\mathrm{Pull\ values\ for\ channel\ %i \ in\ fill\ %i}\ (\mu=%.3f,\ \sigma=%.3f)$" %(ch, nfill, mu, sigma))
    plt.grid(True)
    plt.show()
        
    ch+=1
