import sys, json, pickle, csv


def doMakeGhostsFile(ConfigInfo):
    
    table = {}
    csvtable = []

    Fill = ConfigInfo["Fill"]
    InputDataDir = str(ConfigInfo['InputDataDir'])

# get correction factors

    GhostsValuesFile = InputDataDir + '/Ghosts_' + Fill+ '.json'
    GhostsConfigInfo = open(GhostsValuesFile)
    GhostsValues =json.load(GhostsConfigInfo)
    GhostsConfigInfo.close()

    GhostsFraction_B1 = GhostsValues['GhostsFraction_B1']
    GhostsFraction_B2 = GhostsValues['GhostsFraction_B2']

    table['GhostsFraction_B1'] = GhostsFraction_B1
    table['GhostsFraction_B2'] = GhostsFraction_B2

    for entry in table:
        csvtable.append([entry])
        csvtable.append([table[entry]])

    return table, csvtable


if __name__ == '__main__':

# read config file
    ConfigFile = sys.argv[1]

    Config=open(ConfigFile)
    ConfigInfo = json.load(Config)
    Config.close()

    Fill = ConfigInfo["Fill"]
    AnalysisDir = ConfigInfo["AnalysisDir"]
    OutputDir = AnalysisDir +'/'+ConfigInfo["OutputSubDir"]

    table = {}
    csvtable = []
    table, csvtable = doMakeGhostsFile(ConfigInfo)

    csvfile = open(OutputDir+'/Ghosts_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(csvtable)
    csvfile.close()

    with open(OutputDir+'/Ghosts_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)


 
