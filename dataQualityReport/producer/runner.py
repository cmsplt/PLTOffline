#!/usr/bin/env python3
import os, sys
from argparse import ArgumentParser

#User inputs
parser = ArgumentParser()
parser.add_argument('-y', '--year', dest='year', action='store', choices=[2016,2017,2018], type=int, default=2018)
parser.add_argument('-f', '--fill', dest='fill', action='store', type=int, default=4468)
parser.add_argument('-if', '--inputFile', dest='inputFile', action='store', type=str, default='None')
parser.add_argument('-of', '--outputFile', dest='outputFile', action='store', type=str, default='None')
args = parser.parse_args()
year = args.year
fill = args.fill
inputFile = args.inputFile
outputFile = args.outputFile
kwargs = {
 'year': year,
 'fill': fill, 
 'inputFile': inputFile,
 'outputFile': outputFile
}

#Modules
from executer import *
class executer(executer):
 def __init__(self,**kwargs):
  super(executer,self).__init__(**kwargs)

#Here specify what you want to run
theExecuter = executer(**kwargs) #Call the class the execute the commands
theExecuter.executeHelloWorld() #Call the methods of the class
