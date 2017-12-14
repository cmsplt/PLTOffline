#Project8/ter: Starting from one or more  txt file(s), this script will collect the data for baseline correction values in PLT channels, gathering data for Pixel FED, FastOR FED-xy for every channel 0 to 13 -- this version is for 2016 fills and for the workloop files that divide FED0 and FED1
#Beg. date: 24/10/2017


import matplotlib.pyplot as plt
import matplotlib.dates
import gzip
from datetime import datetime

from os import listdir
from os.path import isfile, join

import sys


#This floowing part makes it possible to just write the fill number on the command line
if (len(sys.argv)<2):
    print "Usage: "+sys.argv[0]+" fillNumber"
    sys.exit(1)
nfill= int(sys.argv[1])


#This following part will loop over all the files in a given directory - the one corresponding to the fill
dir = "./%d" %nfill
files = [join(dir, f) for f in listdir(dir) if isfile(join(dir, f))]


#initializing the lists that will be used to plot stuff
time_list = []

pixel_ch_list = [1,2,4,5,7,8,10,11,13,14,16,17,19,20,22,23]
pixel_blc_list = []

fed_ch_list = [1,2,3,4,5,6,10,11,12,13,14,15,19,20,21,22,23,24,28,29,30,31,32,33]
fed0_blc_list = []
fed1_blc_list = []

#Conventional naming of files: if the files are named "data_n.dat.gz" where n is an integer
#from 0 to N, then by writing as input the number N+1 (which is, the total number of files)
#the script will be able to loop over this files automatically.
                                                                       

k=0    #just for debugging
for f in files:
    infile = gzip.open(f, "rt")
    for line in infile:
        index = 0

        if line[0]=="+": #the lines with the timestamps always start with "+++"
            time = ""
            for i in range(19,38):  #this is the exact point of the line where the timestamp (y-m-d H:M:S) is
                time+=line[i]
            time_list.append(time)
            #print time_list

        elif line[0]=="B" and line[24] == "P": #this is a line with Pixel data
            temp_list= []

            while index<len(line):
                if line[index] == ",": #which basically is a way to get the baseline cal since the data are (ch,blc)
                    element=""
                    counter=index+2
                    while line[counter]!=")":
                        element += line[counter]
                        counter+=1
                    if element != "Correction": #there is a legend at any line that needs to be discarded
                        element = int(element)
                        temp_list.append(element)
                
                index+=1

            if len(temp_list)==14: #14 channels in 2016
                pixel_blc_list.append(temp_list)
            else:
                print "An unforeseen instance has occoured (Pixel) at line %i" %k

        
        elif line[0]=="B" and line[35]=="#" and line[36]=="0": #this is a line with FED0 data
            temp_list= []

            while index<len(line):
                if line[index] == ",": #which basically is a way to get the baseline cal since the data are (ch,blc)
                    element=""
                    counter=index+2
                    while line[counter]!=")":
                        element += line[counter]
                        counter+=1
                    if element != "Correction": #there is a legend at any line that needs to be discarded
                        element = int(element)
                        temp_list.append(element)
                
                index+=1

            if len(temp_list)==24: #FED0 has 24 elements also in 2016
                fed0_blc_list.append(temp_list)
            else:
                print "An unforeseen instance has occoured (FED0) at line %i" %k


        elif line[0]=="B" and line[35]=="#" and line[36]=="1": #this is the line with FED1 data
            temp_list= []

            while index<len(line):
                #index = line.find("(", index)
                if line[index] == ",": #which basically is a way to get the baseline cal since the data are (ch,blc)
                    element=""
                    counter=index+2
                    while line[counter]!=")":
                        element += line[counter]
                        counter+=1
                    if element != "Correction": #there is a legend at any line that needs to be discarded
                        element = int( element)
                        temp_list.append(element)                    

                index+=1

            if len(temp_list)==18: #FED1 has less elements in 2016 (2 channels less)
                fed1_blc_list.append(temp_list)
            else:
                print "An unforeseen instance has occoured (FED1) at line %i" %k

        k+=1 #this is just to check that the code is running
        if k%10000==0:
            print "Done with line number %i" %k
    
    infile.close()


#if everything has gone alright, then the last four values should be equal
print "The number of lines in total is " + str(k)
print "The length of time_list is " + str(len(time_list))
print "The length of pixel_blc_list " + str(len(pixel_blc_list))
print "The length of fed0_blc_list is " + str(len(fed0_blc_list))
print "The length of fed1_blc_list is " + str(len(fed1_blc_list))


#Fixing the dates so that they can be plotted
x = [datetime.strptime(d, "%Y-%m-%d %H:%M:%S") for d in time_list]
xtime = matplotlib.dates.date2num(x)

#Now on to plotting
cnt=0    #this counter is for the FED#0 list
cnt2=0   #this counter is for the FED#1 list
for channel in range(14):  #in 2016 there were 14 channels
   
    hfmt = matplotlib.dates.DateFormatter("%Y/%m/%d\n%H:%M:%S") #formatting the date and the plot features
    plt.gca().xaxis.set_major_formatter(hfmt) 
    plt.title(r"$\mathrm{Baseline\ correction\ for\ channel\ %i\ in\ fill\ %i}$" %(channel, nfill))
    plt.xlabel("Date Time")
    plt.ylabel("Baseline correction [?]")


    if channel <= 7:
        y = [item[channel] for item in pixel_blc_list]  #data for pixel
        z = [item[cnt] for item in fed0_blc_list]       #data from the three ROCs
        v = [item[cnt+1] for item in fed0_blc_list]
        w = [item[cnt+2] for item in fed0_blc_list]

        plt.plot(xtime, y, "m", label="Pixel FED %i"     %pixel_ch_list[channel])          
        plt.plot(xtime, z, "r", label="FastOr FED 0-%i"  %fed_ch_list[cnt])
        plt.plot(xtime, v, "g", label="FastOr FED 0-%i"  %fed_ch_list[cnt+1])
        plt.plot(xtime, w, "b", label="FastOr FED 0-%i"  %fed_ch_list[cnt+2])

        cnt+=3

    else:
        y = [item[channel] for item in pixel_blc_list]  #data for pixel
        z = [item[cnt2] for item in fed1_blc_list]      #data for the three ROCs
        v = [item[cnt2+1] for item in fed1_blc_list]
        w = [item[cnt2+2] for item in fed1_blc_list]
        
        plt.plot(xtime, y, "m", label="Pixel FED %i"     %pixel_ch_list[channel])
        plt.plot(xtime, z, "r", label="FastOr FED 1-%i"  %fed_ch_list[cnt2])
        plt.plot(xtime, v, "g", label="FastOr FED 1-%i"  %fed_ch_list[cnt2+1])
        plt.plot(xtime, w, "b", label="FastOr FED 1-%i"  %fed_ch_list[cnt2+2])

        cnt2+=3

   
    plt.legend(loc="best", fancybox=True, shadow=True)
    plt.grid(True)
    plt.savefig("blc_%i_ch%i.png" %(nfill, channel))    #the plots will be saved to the directory: WATCH OUT for the fill(s) number!  
    plt.close()

    print "Done with the graph, on to the next one"
