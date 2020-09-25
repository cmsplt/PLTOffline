#!/usr/bin/env python3

import os, sys

if len(sys.argv) < 3:
    print("Usage:",sys.argv[0],"fill_number slink_file")
    sys.exit(1)

fill_number = sys.argv[1]
slink_file = sys.argv[2]
normtags = {"hfoc": "hfoc16PaperV3", "pltzero": "pltzero16PaperV2"}

print("Invoking brilcalc for fill",fill_number)
# Invoke brilcalc for hfoc and plt.
for l in ["hfoc", "pltzero"]:
    os.system('brilcalc lumi -f '+fill_number+' --byls -b "STABLE BEAMS" --normtag '+
              normtags[l]+' -u hz/ub --tssec -o '+l+'_'+fill_number+'.csv')

# Run the accidental code.
os.system('../TrackLumiZeroCounting '+slink_file+' ../GainCal/GainCalFits_20160501.155303.dat ../ALIGNMENT/Trans_Alignment_4892.dat ../TrackDistributions/TrackDistributions_MagnetOn2016_4892.txt')

os.rename("TrackLumiZC.txt", "TrackLumiZC_"+fill_number+".txt")

