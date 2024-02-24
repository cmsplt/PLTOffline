import ROOT

def get_dip_data(dipfile, timeWindow):

    import os
    vdmdir = os.getenv("VDMPATH") 
    ROOT.gROOT.LoadMacro(vdmdir+'/dict/DIP_dict.C+')

    masks = set()  # to collect collision patterns and verify that there is exactly one
    rawdata = {}   # to collect per-scanpoint times and bunch intensities

    # get TTree with recorded HF data
    f = ROOT.TFile(dipfile)
    tree = f.Get('VdMDIPCombined')

    # loop over tree entries and read all lumi sections
    for i in range(tree.GetEntriesFast()):
        if tree.GetEntry(i) <= 0:
            raise Exception('TTree::GetEntry() failed')

        # a shorthand notation
        dip = tree.VdMDIPCombined

        timestamp = dip.timestamp
        if timestamp>= timeWindow[0] and timestamp <= timeWindow[1]:

            parameters = {}
            parameters["CMEnergy"] = dip.Energy
            parameters["Fill"] = dip.FillNumber
            parameters["Run"] = dip.runNumber

        # take stable beams
            beamMode = dip.beamMode.strip('\x00') # remove zero characters at the end
            if beamMode != 'STABLE BEAMS':
                print('WARN: dip.beamMode = "{0}", entry skipped'.format(beamMode))
                continue

        # take lumi sections with recorded data
            if dip.VdMScan.RecordDataFlag == 0:
                print('WARN: dip.VdMScan.RecordDataFlag = 0, entry skipped')
                continue

            beam_info1 = ROOT.BEAM_INFO()
            beam_info2 = ROOT.BEAM_INFO()
            ROOT.getBeam(dip, beam_info1, 0)
            ROOT.getBeam(dip, beam_info2, 1)

            beam1 = list(beam_info1.averageBunchIntensities)
            beam2 = list(beam_info2.averageBunchIntensities)

        # verify that FBCT data was written indeed
            if sum(beam1) < 1e9 or sum(beam2) < 1e9:
                print('WARN: sum of averageBunchIntensities < 1e9, entry skipped')
                continue

        # determine which bunches are colliding and which are non-colliding
            bconf1 = list(beam_info1.beamConfig)
            bconf2 = list(beam_info2.beamConfig)
            masks.add(bxmask_from_config(bconf1, bconf2))

            key = (dip.VdMScan.isXaxis, dip.VdMScan.BeamSeparation)
            point = rawdata.setdefault(key, {})

        # collect raw info
            point.setdefault('times', []).append(timestamp)

    # verify that there is exactly one collision pattern
    if len(masks) != 1:
        raise Exception('More than one collision pattern in single DIP file')

    # get collision pattern
    (colliding, noncoll1, noncoll2) = masks.pop()

    # repack scanpoints into more coding-friendly form
    scanpoints = []
    for (key, point) in rawdata.items():
        tstart = min(point['times'])
        tstop = max(point['times'])
        scanpoints.append((key[0], key[1], tstart, tstop))

    return (parameters, colliding, noncoll1, noncoll2, scanpoints)


def bxmask_from_config(beamconf1, beamconf2):

    def get_bx(bconf):
        # remove trailing zeroes
        try:
            i = bconf.index(0)
        except ValueError:
            pass
        else:
            # verify that only zeroes present after the index i
            if len(set(bconf[i:])) != 1:
                raise Exception('unexpected zero in beamConfig')
            bconf = bconf[:i]

        # verify that each element gives 1 modulo 10
        for val in bconf:
            if val % 10 != 1:
                raise Exception('invalid beamConfig value modulo 10')

        # translate into bx numbers
        return [(x - 1)//10 + 1 for x in bconf]

    bx1 = get_bx(beamconf1)
    bx2 = get_bx(beamconf2)

    colliding = tuple(sorted(set(bx1) & set(bx2)))      # bx common to both
    noncoll1 = tuple(sorted(set(bx1) - set(colliding))) # bx only in the first
    noncoll2 = tuple(sorted(set(bx2) - set(colliding))) # bx only in the second

    return (colliding, noncoll1, noncoll2)


def doMakeScanFile(ConfigInfo):

    Fill = str(ConfigInfo['Fill'])
    Date = str(ConfigInfo['Date'])
    Run = str(ConfigInfo['Run'])
    InputDIPFile = str(ConfigInfo['InputDIPFile'])
    ScanNames= ConfigInfo['ScanNames']
    ScanTimeWindows= ConfigInfo['ScanTimeWindows']
    BetaStar = str(ConfigInfo['BetaStar'])
    Angle = str(ConfigInfo['Angle'])
    Offset = str(ConfigInfo['Offset'])
    ParticleTypeB1 = str(ConfigInfo['ParticleTypeB1'])
    ParticleTypeB2 = str(ConfigInfo['ParticleTypeB2'])
    EnergyB1 = str(ConfigInfo['EnergyB1'])
    EnergyB2 = str(ConfigInfo['EnergyB2'])

    scan = [ [] for entry in ScanNames]
    for i in range(len(ScanNames)):
        timeWindow = [ScanTimeWindows[i][0], ScanTimeWindows[i][1]]
        (parameters, colliding, noncoll1, noncoll2, scanpoints) = get_dip_data(InputDIPFile, timeWindow)

        sp_timeSorted = sorted(scanpoints, key=lambda k: k[2])

        for point in sp_timeSorted:
            if point[2] > ScanTimeWindows[i][0] and point[3] < ScanTimeWindows[i][1]:
                if "X" in ScanNames[i]:
                    if point[0] == 1:
                        if point[1] != 0.0: # avoid zero points before/after VdM scan
                            scan[i].append(point)
                if "Y" in ScanNames[i]:
                    if point[0] == 0:
                        if point[1] != 0.0: # avoid zero points before/after VdM scan
                            scan[i].append(point)




        if parameters["Run"] != int(Run):
            print('Mismatch between dip info and config info: dip.runNumber:', parameters["Run"], ' and config.runNumber', Run)
            import sys
            sys.exit(1)

        if parameters["Fill"] != int(Fill):
            print('Mismatch between dip info and config info: dip.runNumber:', parameters["Fill"], ' and config.runNumber', Fill)
            import sys
            sys.exit(1)


# Will eventually also come from dip

    parameters["ParticleTypeB1"] = ParticleTypeB1
    parameters["ParticleTypeB2"] = ParticleTypeB2
    parameters["BetaStar"] = BetaStar
    parameters["CrossingAngle"] = Angle
    parameters["EnergyB1"] = EnergyB1
    parameters["EnergyB2"] = EnergyB2
    parameters["Offset"] = Offset


    table = {}

    table["Fill"] = Fill
    table["Date"] = Date
    table["Run"] = Run
    table["InputDIPFile"] = InputDIPFile
    table["ScanNames"] = ScanNames
    table["ScanTimeWindows"] = ScanTimeWindows 
    table["BetaStar"] = BetaStar
    table["Angle"] = Angle
    table["Offset"] = Offset
    table["ParticleTypeB1"] = ParticleTypeB1
    table["ParticleTypeB2"] = ParticleTypeB2
    table["EnergyB1"] = EnergyB1
    table["EnergyB2"] = EnergyB2
    table["FilledBunchesB1"] = sorted(colliding + noncoll1)
    table["FilledBunchesB2"] = sorted(colliding + noncoll2)
    table["CollidingBunches"] = sorted(colliding)


    csvtable = []

    csvtable.append(["Fill", Fill])
    csvtable.append(["Date", Date])
    csvtable.append(["Run", Run])
    csvtable.append(["InputDIPFile", InputDIPFile])
    csvtable.append(["ScanNames", ScanNames])
    csvtable.append(["ScanTimeWindows",ScanTimeWindows ])
    csvtable.append(["BetaStar",BetaStar ])
    csvtable.append(["Angle",Angle ])
    csvtable.append(["Offset",Offset ])
    csvtable.append(["ParticleTypeB1",ParticleTypeB1 ])
    csvtable.append(["ParticleTypeB2", ParticleTypeB2])
    csvtable.append(["EnergyB1",EnergyB1 ])
    csvtable.append(["EnergyB2", EnergyB2])
    csvtable.append(["FilledBunchesB1", sorted(colliding + noncoll1)])
    csvtable.append(["FilledBunchesB2", sorted(colliding + noncoll2)])
    csvtable.append(["CollidingBunches", sorted(colliding) ])
    csvtable.append(["scan number", "scan type", "scan points: number, tStart, tStop, relative displacement"])

    for i in range(len(ScanNames)):
        table["Scan_" + str(i+1)]=[]
        csvtable.append(["Scan_" + str(i+1)] )
        for j in range(len(scan[i])):
            row = [i+1, str(ScanNames[i])]
            row.append(j+1)
            row.append(scan[i][j][2])
            row.append(scan[i][j][3])
            row.append(scan[i][j][1])
            csvtable.append(row)
            lst = list(scan[i][j])
            lst[0] = str(ScanNames[i])
            scan[i][j] = tuple(lst)
            table["Scan_" + str(i+1)].append(row)


    return table, csvtable



if __name__ == '__main__':

    import pickle, csv, sys, json

    ConfigFile = sys.argv[1]

    Config=open(ConfigFile)
    ConfigInfo = json.load(Config)
    Config.close()

    Fill = str(ConfigInfo['Fill'])
    AnalysisDir = str(ConfigInfo['AnalysisDir'])    
    OutputSubDir = str(ConfigInfo['OutputSubDir'])    

    outpath = './' + AnalysisDir + '/'+ OutputSubDir 

    import os
    dirlist = ['./'+AnalysisDir, outpath]
    for entry in dirlist:
        if not os.path.isdir(entry):
            print "Make directory ", entry
            os.mkdir(entry, 0755 )
            
    table = {}
    csvtable = []

    table, csvtable = doMakeScanFile(ConfigInfo)

    csvfile = open(outpath+'/Scan_'+str(Fill)+'.csv', 'wb')
    writer = csv.writer(csvfile)
    writer.writerows(csvtable)
    csvfile.close()


    with open(outpath+'/Scan_'+str(Fill)+'.pkl', 'wb') as f:
        pickle.dump(table, f)

