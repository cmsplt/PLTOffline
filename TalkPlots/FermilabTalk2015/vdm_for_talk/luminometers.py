# available classes correspond to entries in tuple XsecCalculationOptions.LuminometerOptions.AvailableLuminometers

class LuminometerDefaults:

    class StandardHF:
        LuminometerName = "HF"
        LuminometerDescription = "HFlumi, zero counting in 2 HF rings,  Hadron Forward Calorimeter based"
        WhatIsMeasured = "Counts"
        NormalizationGraphs = "CurrentProduct"
        OldNormAvailable = True
        
    class StandardPCC:
        LuminometerName = "PCC"
        LuminometerDescription = "Pixel Cluster Counting, Pixel detector based"
        WhatIsMeasured = "Counts"
        NormalizationGraphs = "CurrentProduct"
        OldNormAvailable = False

    class StandardVtx:
        LuminometerName = "Vtx"
        LuminometerDescription = "Vertex Counting, Tracker based"
        WhatIsMeasured = "Counts"
        NormalizationGraphs = "CurrentProduct"
        OldNormAvailable = False

# these need to be filled in
            
    class StandardBCM1F:
        LuminometerName = "BCM1F"
        LuminometerDescription = "BCM1F based"
        WhatIsMeasured = ""
        NormalizationGraphs = ""
        OldNormAvailable = False

    class StandardPLT:
        LuminometerName = ""
        LuminometerDescription = "Pixel Luminosity Telescope based"
        WhatIsMeasured = ""
        NormalizationGraphs = ""
        OldNormAvailable = False

    def __init__(self, name):
        
        self.LuminometerName = name
        defaultSettings = self.returnDefaults(name)
        self.WhatIsMeasured = defaultSettings.WhatIsMeasured
        self.NormalizationGraphs = defaultSettings.NormalizationGraphs
        self.OldNormAvailable = defaultSettings.OldNormAvailable

    def returnDefaults(self, name):
        if name == "HF":
            return LuminometerDefaults.StandardHF
        if name == "PCC":
            return LuminometerDefaults.StandardPCC
        if name == "Vtx":
            return LuminometerDefaults.StandardVtx
        if name == "BCM1F":
            return LuminometerDefaults.StandardBCM1F
        if name == "PLT":
            return LuminometerDefaults.PLT


