#!/bin/env python
from __future__ import print_function
import serial
import time
import datetime
import signal
import sys
import os
import pylab as py
#import matplotlib
#import matplotlib.pyplot as plt
#from analyze_dcdcs import read_position,find_DCDC

from argparse import ArgumentParser
parser = ArgumentParser()
parser.add_argument('-dtm',   '--dirToMove',     dest='dirToMove',     action='store', type=str, default='')
args = parser.parse_args()
dirToMove = args.dirToMove

def drange(start, stop, step):
    r = start
    while r < stop:
        yield r
        r += step

def signal_handler(sig, frame):
        print('You pressed Ctrl+C!')
        raise OSError("Couldn't open device!")

class myserial():
    
    def __init__(self,port="/dev/ttyUSB1",baudrate=57600, keithley=True):
        self.keithley=keithley
        if keithley:
            self.ser = serial.Serial(
                port=port,
                baudrate=baudrate,
                parity=serial.PARITY_ODD,
                bytesize=serial.EIGHTBITS,
                stopbits=serial.STOPBITS_ONE,
		xonxoff=True
            )
            self.ser.setRTS(False)
        else:
            self.ser = serial.Serial(
                port=port,
                baudrate=baudrate,
            )

        
    def read(self):
        out = []
        terminated = False
        # while ser.inWaiting() > 0 or not terminated:
        while self.ser.inWaiting() > 0:
            # print(self.ser.inWaiting())
            data = self.ser.read(1)
            if data == b'\r':
                out.append('\n')
                terminated = True
            else:
                out.append(data.decode('ascii',errors='ignore'))

        if out != '':
            return ''.join(out)
        else:
            return ''
    def write(self,command):
        if self.keithley:
            command+="\r"
        else:
            command+="\n"
        command=command.encode('ascii',errors='ignore')
        self.ser.write(command)
        time.sleep(0.05)
        #~ time.sleep(1)
    
    def get_mesurement(self):
        # time.sleep(2.)
        self.write(':INIT')
        time.sleep(.001)
        self.write(":FETCH?")
        return self.read()
    
    def enable_output(self):
        self.write(":OUTP ON")
        time.sleep(.25)
    
    def disable_output(self):
        self.write(":OUTP OFF")
        time.sleep(.25)
        
    def close(self):
        # time.sleep(.25)
        if self.keithley:
            self.write(":OUTP OFF")
        else:
            self.write("OP 0")
        self.ser.close()
    
    def set_voltate(self,voltage):
        self.write(":SOUR:VOLT:LEV {0}".format(voltage))
        
def main():
    
    #~ dcdc_position=read_position()
    ser=myserial(port="/dev/ttyUSB1",baudrate=57600)
    start_parameter=[
                "*RST",
                ":SOUR:FUNC VOLT",
                ":SOUR:VOLT:MODE FIXED",
                ":SOUR:VOLT:RANG -500",
                ":SENS:CURR:PROT 105E-6",
                ':SENS:FUNC "CURR"',
                # ":SENS:CURR:RANG 10E-2",
                ":SENS:CURR:RANG 105E-6",
                ":FORM:ELEM CURR",
            ]
    for i in start_parameter:
     ser.write(i)
  
    voltages = []
    muAmperes = [] 


    #ramping down 
    ini = 0
    fin = -500
    step = -10 
    for s in range(ini, fin-1, step): 
     ser.set_voltate(s)
     ser.enable_output()
     print(s)
     print(ser.get_mesurement())
     voltages.append(-s)
     muAmpere = -(float(str((ser.get_mesurement()).rstrip()))*1000000) #*10^6 is to convert in muAmpere
     muAmperes.append(muAmpere) 
     time.sleep(3)
  
    #ramping up
    for s in range(fin-step, ini, -step): 
     ser.set_voltate(s)
     ser.enable_output()
     print(s)
     print(ser.get_mesurement())
     time.sleep(3)

    ser.close()
    
    #Plot
    #py.style.use('seaborn-white')

	with open(dirToMove + '/iv.csv', 'w') as csv:
        for v, i in zip(voltages, muAmperes):
            csv.write(v + ',' + i + '\n')

    fig = py.figure(1)
    py.plot(voltages,muAmperes,label="", linestyle="", marker='o')
    py.xlabel('-V [V]',size=60)#, horizontalalignment='right', x=1.0)
    py.ylabel('-I [$\mu$A]',size=60)#, verticalalignment='top')
    py.tick_params(axis='x', labelsize=40)
    py.tick_params(axis='y', labelsize=40)
    mng = py.get_current_fig_manager()
    mng.resize(*mng.window.maxsize())
    name = "IVcurve.png"
    fig.set_size_inches((32, 18))#Number are W and H respectively, here adjusted to fit screen size
    fig.savefig(name, bbox_inches='tight')

    fig = py.figure(2)
    py.plot(voltages,muAmperes,label="", linestyle="", marker='o')
    py.xlabel('-V [V]',size=60)#, horizontalalignment='right', x=1.0)
    py.ylabel('-I [$\mu$A]',size=60)#, verticalalignment='top')
    py.tick_params(axis='x', labelsize=40)
    py.tick_params(axis='y', labelsize=40)
    mng = py.get_current_fig_manager()
    mng.resize(*mng.window.maxsize())
    nameLogY = "IVcurve_logY.png"
    py.yscale("log")    
    fig.set_size_inches((32, 18))#Number are W and H respectively, here adjusted to fit screen size
    fig.savefig(nameLogY, bbox_inches='tight')

    #Move png files to ROCID
    os.system('mv *png %s' % (dirToMove))

    '''    
    #ramping down 
    ini = 0
    fin = -40
    step = -20 #the steps corresponds to 2*times the actual steps you want to consider
    for s in range(ini, fin-1, step): #x,y,z
     ser.set_voltate(s)
     ser.enable_output()
     print(s)
     print(ser.get_mesurement())
     voltages.append(-s)
     muAmpere = -(float(str((ser.get_mesurement()).rstrip()))*1000000) #*10^6 is to convert in muAmpere
     muAmperes.append(muAmpere) 
     time.sleep(3)
  
    #ramping up
    fin = fin-step/2 
    for s in range(fin, ini, -step): #y-z/2 (y>0) or y+z/2 (y),x,-z
     ser.set_voltate(s)
     ser.enable_output()
     print(s)
     print(ser.get_mesurement())
     voltages.append(-s)
     muAmpere = -(float(str((ser.get_mesurement()).rstrip()))*1000000) #*10^6 is to convert in muAmpere
     muAmperes.append(muAmpere) 
     time.sleep(3)
    '''
   
if __name__ == '__main__':
    main()
