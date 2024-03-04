import json

def extractValues(fileName):

    extractValue = False
    cond1 = False
    cond2 = False
    satelliteFraction = {}
    with open(fileName) as file:
        for i,line in enumerate(file):
            extractValue = cond1 and cond2
            if "###########################" in line:
                cond1 = False
                cond2 = False
                extractValue = False
            if extractValue:
                key = line.split(" ")[0]
                numerator = line.split(" ")[-1]
                numerator.rstrip("\r\n")
                denominator = line.split(" ")[6]
                ratio = float(numerator)/float(denominator)
                satelliteFraction[key] = ratio
            if "Filled slots' data" in line:
                cond1 = True
            if "Data format: j Nj1 Nj2 ... Nj9 Nj10 Bj N_tilde_j" in line:
                cond2 = True

    return satelliteFraction


rootDir = "/Users/grothe/lumi/VdM/fromLDM"

# fill 3503

valueFileB1 = "Results/3503/B1/2013_01_29_01:47/LDM_3503_B1_2013_01_29_01:47.dat"
valueFileB2 = "Results/3503/B2/2013_01_29_01:47/LDM_3503_B2_2013_01_29_01:47.dat"

satelliteFractionB1 = {}
satelliteFractionB2 = {}

fileName = rootDir + "/" + valueFileB1
satelliteFractionB1 = extractValues(fileName)

fileName = rootDir + "/" + valueFileB2
satelliteFractionB2 = extractValues(fileName)

# since according to documentation there is no raw data for slot 9 for B2
value = satelliteFractionB2["1"] 
satelliteFractionB2["9"] = value
result = {"SatellitesFraction_B1": satelliteFractionB1, "SatellitesFraction_B2": satelliteFractionB2}

outFile = "Satellites_3503.json"

with open(outFile, "w") as file:
    json.dump(result, file)

# fill 3537

valueFileB1 = "Results/3537/B1/2013_02_07_22:15/LDM_3537_B1_2013_02_07_22:15.dat"
valueFileB2 = "Results/3537/B2/2013_02_07_22:15/LDM_3537_B2_2013_02_07_22:15.dat"

satelliteFractionB1 = {}
satelliteFractionB2 = {}

fileName = rootDir + "/" + valueFileB1
satelliteFractionB1 = extractValues(fileName)

fileName = rootDir + "/" + valueFileB2
satelliteFractionB2 = extractValues(fileName)

# since according to documentation there is no raw data for slot 9 for B2
value = satelliteFractionB2["1"] 
satelliteFractionB2["9"] = value

result = {"SatellitesFraction_B1": satelliteFractionB1, "SatellitesFraction_B2": satelliteFractionB2}

outFile = "Satellites_3537.json"

with open(outFile, "w") as file:
    json.dump(result, file)


# fill 3563

valueFileB1 = "Results/3563/B1/2013_02_13_22:56/LDM_3563_B1_2013_02_13_22:56.dat"
valueFileB2 = "Results/3563/B2/2013_02_13_22:56/LDM_3563_B2_2013_02_13_22:56.dat" 

satelliteFractionB1 = {}
satelliteFractionB2 = {}

fileName = rootDir + "/" + valueFileB1
satelliteFractionB1 = extractValues(fileName)

fileName = rootDir + "/" + valueFileB2
satelliteFractionB2 = extractValues(fileName)

result = {"SatellitesFraction_B1": satelliteFractionB1, "SatellitesFraction_B2": satelliteFractionB2}

outFile = "Satellites_3563.json"

with open(outFile, "w") as file:
    json.dump(result, file)
