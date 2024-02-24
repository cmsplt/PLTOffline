import CorrectionManager
import ROOT as r
import sys
import pickle
from vdmUtilities import *

class BeamBeam_Corr(CorrectionManager.CorrectionProvider):

    BBcorr = {}

    def GetCorr(self, fileName):

        table = {}
        with open(fileName, 'rb') as f:
            table = pickle.load(f)

        self.BBcorr = table 

        return


    def PrintCorr(self):

        print ""
        print "===="
        print "PrintBeamBeamCorr"
        print "Correction factors derived from fits to uncorrected distributions"
        print "Correction factors depend on scan number, scan point number and bcid"
        print "===="
        print ""


    def doCorr(self,inData,configFile):

        print "Correcting coordinates with beambeam correction factors"

        self.GetCorr(configFile)
        
        self.PrintCorr()

    # apply correction here to coordinate, then write back into entry, check if this really changes value in calling function

        for entry in inData:
            scanNumber = entry.scanNumber
            key = "Scan_"+str(scanNumber)
            
            corrPerSP  = self.BBcorr[key]
        

            corrXPerSP = [{} for value in corrPerSP]
            corrYPerSP = [{} for value in corrPerSP]
            for value in corrPerSP:
                corrXPerSP[value[2]-1] = value[3]
                corrYPerSP[value[2]-1] = value[4]
                
            print ">>"
            print corrXPerSP
            print corrYPerSP
            corrXPerBX = {bx:[] for bx in entry.collidingBunches}
            corrYPerBX = {bx:[] for bx in entry.collidingBunches}
            for bx in entry.collidingBunches:
                for j in range(entry.nSP):
                    valueX = corrXPerSP[j][str(bx)]
                    corrXPerBX[bx].append(valueX)
                    valueY = corrYPerSP[j][str(bx)]
                    corrYPerBX[bx].append(valueY)

            print "<<<"
            print corrXPerBX
            print corrYPerBX

            for index in entry.spPerBX:
                if 'X' in entry.scanName:
                    entry.spPerBX[index] = entry.spPerBX[index] +  corrXPerBX[index]
                if 'Y' in entry.scanName:
                    entry.spPerBX[index] = entry.spPerBX[index] +  corrYPerBX[index]

            
