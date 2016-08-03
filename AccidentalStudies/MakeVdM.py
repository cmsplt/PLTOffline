import csv
import pickle
import math

# Function to covert a number to a 4-digit string
def four(int):
    if int < 10:
        return "000" + str(int)
    elif int < 100:
        return "00" + str(int)
    elif int < 1000:
        return "0" + str(int)
    else:
        return str(int)

# Determine the steps
scanStart = {29085431: 'X1', 30322339: 'Y1', 31569528: 'Y2', 32748126: 'X2',
             34047807: '2imagingX', 35477032: '2imagingY', 37229694: '1imagingX',
             38452142: '1imagingY', 43669489: 'X3', 44942813: 'Y3'}
scanEnd = {30039676: 'X1', 31279577: 'Y1', 32528151: 'Y2', 33702425: 'X2',
           34991850: '2imagingX', 36421139: '2imagingY', 38170894: '1imagingX',
           39404964: '1imagingY', 44622282: 'X3', 45902961: 'Y3'}

# The bunches with stuff in them
trigger = [1, 41, 81, 110, 121, 161, 201, 241, 281, 591, 872, 912, 952, 992,
           1032, 1072, 1112, 1151, 1152, 1682, 1783, 1823, 1863, 1903, 1943,
           1983, 2023, 2063, 2654, 2655, 2694, 2734, 2774, 2814, 2854, 2894, 2934]

scanNum = "4954"

first = trigger[0]

# Prepare the output file                     
output_csv = open("VdMAnalysis.csv", 'wb')
write = csv.writer(output_csv, delimiter = ',', lineterminator="\n")

#Write the first csv row
write.writerow(["ScanNumber, ScanNames, ScanPointNumber, Rates per bx, RateErr per bx"])

#Arrays and dictionaries to hold stuff
csvTable = []
pickleTable = {}
files = []
readers = {}

# Load the readers
for i in trigger:
    filename = "CombinedRates_" + scanNum + "_" + four(i) + ".txt"
    input_csv = open(filename, "rb")
    files.append(input_csv)
    readers[i] = (csv.reader(input_csv, delimiter = ' '))
    readers[i].next() #Skip the first line of each file

# Variables that will be changed in the loop
rowToAdd = []
inScan = False
toBeInScan = True
scanNum = 0
scanStep = 1
scanName = ""
# Loop through the channels
for row in readers[first]:
    if (int(row[0]) in scanStart): #If this is the start of a new step
        scanStep = 1
        scanNum += 1
        scanName = scanStart[int(row[0])]
        inScan = True
        extendedName = "Scan_" + str(scanNum)

        csvTable.append([extendedName])
        pickleTable[extendedName] = []

    if (int(row[0]) in scanEnd): #If this is the end of a step
        toBeInScan = False #Tell it to exit this scan after this last data point

    if inScan:
        addRow = []
        addRow.append(str(scanNum))
        addRow.append(scanName)
        addRow.append(str(scanStep))
        
        bunches = {}
        bunchesErr = {}
        for b in readers:
            if b == first: #Because we're already reading through the first file
                readRow = row
            else:
                readRow = readers[b].next()

            #Read in the variables
            tracksAll = float(readRow[3])
            nTrig = float(readRow[2])
            nFull = float(readRow[7])
            nFilledTrig = float(readRow[5])

            f0 = 1.0-nFull/nFilledTrig
            f0err = math.sqrt(f0*(1.0-f0)/nFilledTrig)
            goodTrackZC = -math.log(f0)
            rate = goodTrackZC
            goodTrackZCPlus = -math.log(f0-f0err)
            rateErr = abs(goodTrackZCPlus-goodTrackZC)
            if int(readRow[0]) >= 39844939:
                rateErr /= math.sqrt(9.)
            else:
                rateErr /= math.sqrt(13.)
            # Add values to out dictionaries
            bunches[str(b)] = rate
            bunchesErr[str(b)] = rateErr
        
        csvTable.append(addRow)
        csvTable.append([bunches])
        csvTable.append([bunchesErr])
        pickleTable[extendedName].append(addRow)
        pickleTable[extendedName].append([bunches])
        pickleTable[extendedName].append([bunchesErr])
        scanStep += 1
        if (not toBeInScan): #If that was the last data point
            inScan = False
            toBeInScan = True
    else: #if not in scan, advance the lines in all of the other files
        for b in readers:
            if b != first: #Because we're already reading through the first file
                readers[b].next()

                                                    


#Write to csv and pkl        
write.writerows(csvTable)
with open('VdMAnalysis.pkl', 'wb') as f:
    pickle.dump(pickleTable, f)

# Close the files
for f in files:
    f.close()
output_csv.close()
