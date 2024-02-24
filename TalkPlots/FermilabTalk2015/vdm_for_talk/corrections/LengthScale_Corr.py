import CorrectionManager
import ROOT as r
import sys
import pickle
from vdmUtilities import *

class LengthScale_Corr(CorrectionManager.CorrectionProvider):

    LS_ScaleX =-999.    
    LS_ScaleY =-999.


    def GetCorr(self, fileName):

        table = {}
        with open(fileName, 'rb') as f:
            table = pickle.load(f)

        self.LS_ScaleX = float(table["LS_ScaleX"])
        self.LS_ScaleY = float(table["LS_ScaleY"])

        return


    def PrintCorr(self):

        print ""
        print "===="
        print "PrintLSCorr"
        print "LS_ScaleX ", self.LS_ScaleX
        print "LS_ScaleY ", self.LS_ScaleY
        print "===="
        print ""


    def doCorr(self,inData,configFile):

        print "Scaling coordinates with lengthscale factors"

        self.GetCorr(configFile)
        
        self.PrintCorr()

    # apply correction here to coordinate, then write back into entry, check if this really changes value in calling function

        for entry in inData:
            coord = entry.displacement
            if 'X' in entry.scanName:
                coord_corr = [a*self.LS_ScaleX for a in coord]
            if 'Y' in entry.scanName:
                coord_corr = [a*self.LS_ScaleY for a in coord]

            entry.displacement = coord_corr

            if 'X' in entry.scanName:
                for bx in entry.collidingBunches:
                    coordinates = entry.spPerBX[bx]
                    coordinates_corrected = [ a*self.LS_ScaleX for a in coordinates]
                    entry.spPerBX[bx] = coordinates_corrected
            if 'Y' in entry.scanName:
                for bx in entry.collidingBunches:
                    coordinates = entry.spPerBX[bx]
                    coordinates_corrected = [ a*self.LS_ScaleY for a in coordinates]
                    entry.spPerBX[bx] = coordinates_corrected
#            entry.spPerBx = coordPerBX_corr

