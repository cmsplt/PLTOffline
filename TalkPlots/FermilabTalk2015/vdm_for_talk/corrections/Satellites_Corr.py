import CorrectionManager
import ROOT as r
import sys
import pickle
from vdmUtilities import *

class Satellites_Corr(CorrectionManager.CorrectionProvider):


    SatellitesFraction_B1 =[]    
    SatellitesFraction_B2 =[]


    def GetCorr(self, fileName):

        table = {}
        with open(fileName, 'rb') as f:
            table = pickle.load(f)

        self.SatellitesFraction_B1 = table["SatellitesFraction_B1"]
        self.SatellitesFraction_B2 = table["SatellitesFraction_B2"]

        return


    def PrintCorr(self):

        print ""
        print "===="
        print "PrintSatellitesCorr"
        print "SatellitesFraction_B1 ", self.SatellitesFraction_B1
        print "SatellitesFraction_B2 ", self.SatellitesFraction_B2
        print "===="
        print ""


    def doCorr(self,inData,configFile):

        print "Scaling beam currents with SatellitesFraction factors, this is per colliding BCID"

        self.GetCorr(configFile)
        
        self.PrintCorr()

    # apply correction here to beam currents, then write back into entry

        for entry in inData:

            for i, bx in enumerate(entry.collidingBunches):
                currB1 = entry.avrgFbctB1[i]
                currB2 = entry.avrgFbctB2[i]
                corrB1 = self.SatellitesFraction_B1[str(bx)]
                corrB2 = self.SatellitesFraction_B2[str(bx)]

                currB1_corr = [a * (1-corrB1) for a in currB1]
                currB2_corr = [a * (1-corrB2) for a in currB2]

                entry.avrgFbctB1[i] = currB1_corr
                entry.avrgFbctB2[i] = currB2_corr


