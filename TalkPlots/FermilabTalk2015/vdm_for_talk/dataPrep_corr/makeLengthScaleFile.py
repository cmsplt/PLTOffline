import sys, json, pickle, csv

def doMakeLengthScaleFile(ConfigInfo):

    table = {}
    csvtable = []

    FillLS = ConfigInfo["FillLS"]
    InputDataDir = str(ConfigInfo['InputDataDir'])

# get correction factors

    LSValuesFile = InputDataDir + '/LengthScale_' + FillLS + '.json'
    LSConfigInfo = open(LSValuesFile)
    LSValues =json.load(LSConfigInfo)
    LSConfigInfo.close()

    LS_ScaleX = LSValues['LS_ScaleX']
    LS_ScaleY = LSValues['LS_ScaleY']

    table['LS_ScaleX'] = LS_ScaleX
    table['LS_ScaleY'] = LS_ScaleY

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
    table, csvtable = doMakeLengthScaleFile(ConfigInfo)

    csvfile = open(OutputDir+'/LengthScale_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(csvtable)
    csvfile.close()


    with open(OutputDir+'/LengthScale_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)


 
