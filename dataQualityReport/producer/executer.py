#!/usr/bin/env python3
import os, sys
import ROOT

from functions import *
from variables import *
class variables(variables):
 def __init__(self,outputFile):
  super(variables,self).__init__(outputFile)

class executer(object):
 def __init__(self,**kwargs):
  #User inputs
  self.year = kwargs.get('year') 
  self.fill = kwargs.get('fill') 
  self.inputFile = kwargs.get('inputFile')
  self.outputFile = kwargs.get('outputFile')

  #General quantities
  if self.year==2018:
   print "The year is the last of Run2"
  else:
   raise ValueError('Year must be above 2016 (included).')

 def executeHelloWorld(self):
  helloWorld() #defined in functions.py
  out = variables(self.outputFile)
  out.helloWorld[0] = 1
  #save info into ROOT tree
  out.tree.Fill()
  out.outputfile.Write()
  out.outputfile.Close()  
