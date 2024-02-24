import ROOT as r
import pickle

class vdmInputData:

# class meant to hold relevant data
# to be instantiated per scan


    def __init__(self, scanNumber):

        self.scanNumber = scanNumber

# -->> from Scan file

        self.fill = 0
        self.date = ""
        self.run = 0
        self.inputDIPFile = ""
        self.scanTimeWindows = []
        self.betaStar = 0
        self.angle = 0
        self.offset = 0.0
        self.particleTypeB1 = ""
        self.particleTypeB2 = ""
        self.energyB1 = 0.0
        self.energyB2 = 0.0

        self.filledBunchesB1 = []
        self.filledBunchesB2 = []
        self.collidingBunches = []

# not all colliding bunches are used for all luminometers

        self.usedCollidingBunches = []

        self.scanName = ""
        self.scanNamesAll = []

# scan points info table
        self.sp = {}
        self.nSP = 0

# per scan point
        self.tStart = []
        self.tStop = []
        self.displacement = []

# to allow for SP coordinates that vary with bcid
        self.spPerBX = {}

# -->> from Beam Current file

# currents info table
        self.curr = {}
    
# per scan point
        self.avrgDcctB1 = []
        self.avrgDcctB2 = []
        self.sumAvrgFbctB1 = []
        self.sumAvrgFbctB2 = []
        self.avrgFbctB1PerSP = []
        self.avrgFbctB2PerSP = []

# per BX
        self.avrgFbctB1 = []
        self.avrgFbctB2 = []
        self.avrgFbctB1PerBX = {}
        self.avrgFbctB2PerBX = {}


# -->> from Luminometer Data file

        self.luminometerDataSource = ""
        self.lum = {}
# per BX
        self.lumi = []
        self.lumiErr = []
        self.lumiPerBX = {}
        self.lumiErrPerBX = {}
# per scan point
        self.lumiPerSP = []
        self.lumiErrPerSP = []



    def GetScanInfo(self, fileName):

        table = {}
        with open(fileName, 'rb') as f:
            table = pickle.load(f)

        self.fill = table["Fill"]
        self.date = table["Date"]
        self.run = table["Run"]
        self.inputDIPFile = table["InputDIPFile"]
        self.scanTimeWindows = table["ScanTimeWindows"]
        self.betaStar = table["BetaStar"]
        self.angle = table["Angle"]
        self.particleTypeB1 = table["ParticleTypeB1"]
        self.particleTypeB2 = table["ParticleTypeB2"]
        self.energyB1 = table["EnergyB1"]
        self.energyB2 = table["EnergyB2"]
        self.filledBunchesB1 = table["FilledBunchesB1"]
        self.filledBunchesB2 = table["FilledBunchesB2"]
        self.collidingBunches = table["CollidingBunches"]

        self.scanNamesAll = table["ScanNames"]

        key = "Scan_" + str(self.scanNumber)
        self.scanName = table["ScanNames"][int(self.scanNumber) -1]
        self.offset = table["Offset"][int(self.scanNumber) -1]
        self.sp = table[key]

        self.tStart = [entry[3] for entry in self.sp] 
        self.tStop = [entry[4] for entry in self.sp] 
        self.displacement = [entry[5] for entry in self.sp] 
        self.nSP = len(self.displacement)

        for entry in self.collidingBunches:
            self.spPerBX[entry]= self.displacement

        return
    
    
    def GetBeamCurrentsInfo(self, fileName):

        table = {}
        with open(fileName, 'rb') as f:
            table = pickle.load(f)
        
        key = "Scan_" + str(self.scanNumber)
        self.curr = table[key]
 
# curr values for filled bunches per beam, first index SP, second index BCID
        self.avrgFbctB1PerSP = [{} for entry in self.curr]
        self.avrgFbctB2PerSP = [{} for entry in self.curr]
        for entry in self.curr:
            self.avrgFbctB1PerSP[int(entry[2])-1] = entry[7]
            self.avrgFbctB2PerSP[int(entry[2])-1] = entry[8]

        for index, value in enumerate(self.curr):
            self.avrgDcctB1.append(self.curr[index][3])
            self.avrgDcctB2.append(self.curr[index][4])
            self.sumAvrgFbctB1.append(self.curr[index][5])
            self.sumAvrgFbctB2.append(self.curr[index][6])


# natural order per BX for analysis: curr values only for colliding bunches
# first index BCID (for colliding bx only), second index SP
        self.avrgFbctB1 = [[] for a in range(len(self.collidingBunches))]
        self.avrgFbctB2 = [[] for a in range(len(self.collidingBunches))]
        for i, bx in enumerate(self.collidingBunches):
            for j in range(len(self.displacement)):
                value = self.avrgFbctB1PerSP[j][str(bx)]
                self.avrgFbctB1[i].append(value)
                value = self.avrgFbctB2PerSP[j][str(bx)]
                self.avrgFbctB2[i].append(value)
            self.avrgFbctB1PerBX[bx] = self.avrgFbctB1[i]
            self.avrgFbctB2PerBX[bx] = self.avrgFbctB2[i]


        return


    def GetLuminometerData(self, fileName):

        self.luminometerDataSource = fileName

        table = {}
        with open(fileName, 'rb') as f:
            table = pickle.load(f)

        key = "Scan_" + str(self.scanNumber)
        self.lum = table[key]
 
        self.lumiPerSP = [{} for entry in self.lum]
        self.lumiErrPerSP = [{} for entry in self.lum]
        for entry in self.lum:
            self.lumiPerSP[int(entry[2])-1] = entry[3][0]
            self.lumiErrPerSP[int(entry[2])-1] = entry[3][1]

#        print ">", self.nSP, len(self.lum)
#        for i, val in enumerate(self.lumiPerSP):
#            print ">>", i, val

# determine which of the colliding bunches are in fact used
# for HF should be identical to all colliding ones
# for PCC should only be subset, typically 5

        usedCollidingBunches = []
        for i, bx in enumerate(self.collidingBunches):
            for j in range(self.nSP):
                if str(bx) in self.lumiPerSP[j]:
                    if bx not in usedCollidingBunches:
                        usedCollidingBunches.append(bx)
        self.usedCollidingBunches = usedCollidingBunches

# this is the natural order for analysis
        self.lumi = [[] for a in range(len(self.usedCollidingBunches))]
        self.lumiErr = [[] for a in range(len(self.usedCollidingBunches))]
        for i, bx in enumerate(self.usedCollidingBunches):
            for j in range(self.nSP):
#                print self.lumiPerSP[j]
                value = self.lumiPerSP[j][str(bx)]
                self.lumi[i].append(value)
                valueErr = self.lumiErrPerSP[j][str(bx)]
                self.lumiErr[i].append(valueErr)
            self.lumiPerBX[bx] = self.lumi[i]
            self.lumiErrPerBX[bx] = self.lumiErr[i]

        return
        
    
    def PrintScanInfo(self):

        print ""
        print "===="
        print "PrintScanInfo"
        print "fill", self.fill
        print "date", self.date
        print "run", self.run
        print "inputDIPFile", self.inputDIPFile
        print "scanName", self.scanName
        print "scanNamesAll", self.scanNamesAll
        print "scanTimeWindows", self.scanTimeWindows
        print "betaStar", self.betaStar
        print "angle", self.angle
        print "particleTypeB1", self.particleTypeB1
        print "particleTypeB2", self.particleTypeB2
        print "filledBunchesB1", self.filledBunchesB1
        print "filledBunchesB2", self.filledBunchesB2
        print "collidingBunches", self.collidingBunches
        print "scanNumber", self.scanNumber
        print "complete ScanPoint info table", self.sp
        print "tStart", self.tStart
        print "tStop", self.tStop
        print "displacement", self.displacement
        print "SP coordinates, which may vary with BX", self.spPerBX
        

    def PrintBeamCurrentsInfo(self):
        
        print ""
        print" ===="
        print "PrintBeamCurrentsInfo" 
        print "complete current info table", self.curr
        print "avrgDcctB1 per SP", self.avrgDcctB1
        print "avrgDcctB2 per SP", self.avrgDcctB2
        print "sumAvrgFbctB1 per SP", self.sumAvrgFbctB1
        print "sumAvrgFbctB2 per SP", self.sumAvrgFbctB2
        print "avrgFbctB1 per SP", self.avrgFbctB1PerSP
        print "avrgFbctB2 per SP", self.avrgFbctB2PerSP
        print "avrgFbctB1 per BX", self.avrgFbctB1
        print "avrgFbctB2 per BX", self.avrgFbctB2
        print "avrgFbctB1 per BX", self.avrgFbctB1PerBX
        print "avrgFbctB2 per BX", self.avrgFbctB2PerBX


    def PrintLuminometerData(self):

        print self.lum

        print ""
        print" ===="
        print "PrintLuminometerData"
        print "LuminometerDataSource", self.luminometerDataSource
        print "usedCollidingBunches", self.usedCollidingBunches
        print "complete Luminometer Rate info", self.lum
        for idx, val in enumerate(self.lumiPerSP, start=1):
            print "For SP ", idx, " Luminometer Rates for each BX ", val
        for idx, val in enumerate(self.lumiErrPerSP, start=1):
            print "For SP ", idx, " Luminometer Rates Errors for each BX ", val
        for idx, val in enumerate(self.lumi):
            print "For BX index ", idx, " Luminometer Rates for each SP ", val
        for idx, val in enumerate(self.lumiErr):
            print "For BX index ", idx, " Luminometer Rates Errors for each SP ", val

