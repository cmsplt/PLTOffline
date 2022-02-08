#!/usr/bin/env python3
import os, sys
import numpy as np
import ROOT

class variables(object):
 def __init__(self,outputFile):
  #Create file 
  compression = "LZMA:9"
  ROOT.gInterpreter.ProcessLine("#include <Compression.h>")
  (algo, level) = compression.split(":")
  compressionLevel = int(level)
  if   algo == "LZMA": compressionAlgo  = ROOT.ROOT.kLZMA
  elif algo == "ZLIB": compressionAlgo  = ROOT.ROOT.kZLIB
  else: raise RuntimeError("Unsupported compression %s" % algo)
  self.outputfile = ROOT.TFile(outputFile+'.root', 'RECREATE',"",compressionLevel)
  self.outputfile.SetCompressionAlgorithm(compressionAlgo)
  self.tree = ROOT.TTree('tree','tree')

  ##variables to be stored in the tree
  #float
  self.add_float('helloWorld')
  #string
  #vector float
  #vector string

 def add_float(self,name,dtype=np.dtype(float)):
  if hasattr(self,name):
   print('ERROR! SetBranchAddress of name "%s" already exists!' % (name))
   exit(1)
  setattr(self,name,np.full((1),-99999999999999999999999999999999999999999999999999,dtype=dtype)) #1 elem w/ inizialization '-99999999999999999999999999999999999999999999999999'
  self.tree.Branch(name,getattr(self,name),'{0}/D'.format(name)) #the types in root (/D in this example) are defined here https://root.cern/root/html528/TTree.html

 def add_string(self,name,dtype=np.dtype('S100')): #it assumes a string of max 100 characters
  if hasattr(self,name):
   print('ERROR! SetBranchAddress of name "%s" already exists!' % (name))
   exit(1)
  setattr(self,name,np.full((1),'noVal',dtype=dtype)) #1 elem w/ inizialization 'noval'
  self.tree.Branch(name,getattr(self,name),'{0}/C'.format(name))

 def add_vectorFloat(self,name):
  if hasattr(self,name):
   print('ERROR! SetBranchAddress of name "%s" already exists!' % (name))
   exit(1)
  setattr(self,name,ROOT.std.vector('float')()) #no inizialization
  self.tree.Branch(name,getattr(self,name))

 def add_vectorString(self,name):
  if hasattr(self,name):
   print('ERROR! SetBranchAddress of name "%s" already exists!' % (name))
   exit(1)
  setattr(self,name,ROOT.std.vector('string')()) #no inizialization
  self.tree.Branch(name,getattr(self,name))
