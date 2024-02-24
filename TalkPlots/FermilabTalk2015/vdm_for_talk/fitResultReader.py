import ROOT as r
import pickle, sys

class fitResultReader:

    def __init__(self, fitResultFile):

        self.fitResultTable = {}

        with open(fitResultFile, 'rb') as f:
            self.fitResultTable = pickle.load(f)

        self.fitParamNames = self.fitResultTable[0]


    def getFitParam(self, paramName):

        if paramName not in self.fitParamNames:
            print "First row of FitResultTable should be a list of strings that shows what is in the columns"
            print paramName + " should be somewhere in that list"
            print "This is not the case, hence probem with fitResultTable, exiting program"
            sys.exit(1)

        paramIndex = -99
        for index, value in enumerate(self.fitParamNames):
            if value.strip() == paramName:
                paramIndex = index

# return value is a defaultdict(dict)

        from collections import defaultdict
        fitParam = defaultdict(dict)

        for row in self.fitResultTable[1:]:
            scanNumber = row[0]
            scanName  = "Scan_"+str(scanNumber)
            bx = row[2]
            value = row[paramIndex]
            fitParam[scanName][bx] = value

        return fitParam
