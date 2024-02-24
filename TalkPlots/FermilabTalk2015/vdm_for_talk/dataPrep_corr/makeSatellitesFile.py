import sys, json, pickle, csv

def doMakeSatellitesFile(ConfigInfo):

    InputDataDir = str(ConfigInfo['InputDataDir'])
    Fill = ConfigInfo["Fill"]

# get correction factors

    SatellitesValuesFile = InputDataDir + '/Satellites_' + Fill+ '.json'
    SatellitesConfigInfo = open(SatellitesValuesFile)
    SatellitesValues =json.load(SatellitesConfigInfo)
    SatellitesConfigInfo.close()

    SatellitesFraction_B1 = SatellitesValues['SatellitesFraction_B1']
    SatellitesFraction_B2 = SatellitesValues['SatellitesFraction_B2']
    
    table = {}
    table['SatellitesFraction_B1'] = SatellitesFraction_B1
    table['SatellitesFraction_B2'] = SatellitesFraction_B2

    csvtable = []
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
    AnalysisDir = ConfigInfo['AnalysisDir']
    OutputDir = AnalysisDir +'/'+ConfigInfo["OutputSubDir"]

    table = {}
    csvtable = []
    table, csvtable = doMakeSatellitesFile(ConfigInfo)
    
    csvfile = open(OutputDir + '/Satellites_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(csvtable)
    csvfile.close()
    
    with open(OutputDir+'/Satellites_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)


 
