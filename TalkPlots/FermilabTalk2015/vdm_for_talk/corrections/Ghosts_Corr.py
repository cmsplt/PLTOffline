import CorrectionManager
import ROOT as r
import sys
import pickle
from vdmUtilities import *

class Ghosts_Corr(CorrectionManager.CorrectionProvider):


    GhostsFraction_B1 =-999.    
    GhostsFraction_B2 =-999.


    def GetCorr(self, fileName):

        table = {}
        with open(fileName, 'rb') as f:
            table = pickle.load(f)

        self.GhostsFraction_B1 = float(table["GhostsFraction_B1"])
        self.GhostsFraction_B2 = float(table["GhostsFraction_B2"])

        return


    def PrintCorr(self):

        print ""
        print "===="
        print "PrintGhostsCorr"
        print "GhostsFraction_B1 ", self.GhostsFraction_B1
        print "GhostsFraction_B2 ", self.GhostsFraction_B2
        print "===="
        print ""


    def doCorr(self,inData,configFile):

        print "Scaling beam currents with GhostsFraction factors"

        self.GetCorr(configFile)
        
        self.PrintCorr()

    # apply correction here to beam currents, then write back into entry

        for entry in inData:

            for i, bx in enumerate(entry.collidingBunches):
                currB1 = entry.avrgFbctB1[i]
                currB2 = entry.avrgFbctB2[i]
                currB1_corr = [a * (1-self.GhostsFraction_B1) for a in currB1]
                currB2_corr = [a * (1-self.GhostsFraction_B2) for a in currB2]

                entry.avrgFbctB1[i] = currB1_corr
                entry.avrgFbctB2[i] = currB2_corr


