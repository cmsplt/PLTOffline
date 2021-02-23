#!/usr/bin/env python3

# pip install uproot # [https://github.com/scikit-hep/uproot4]
import os, sys, re, uproot, pandas, matplotlib.pyplot

def testFile(filePath='histo_occupancy.root'):
    if not os.path.isfile(filePath):
        os.system(f'wget https://adelannoy.com/CMS/PLT/tmp/Fill4984-Slink20160603.153652/histo_occupancy.root -O {filePath}') # ~750 kB

def plotOccupancyROC(rootFile, key=f'Occupancy Ch02 ROC0;1'):
    # [https://uproot.readthedocs.io/en/latest/basic.html]
    # rootFile.keys()
    # (rootFile[key] == rootFile[f'{key[:-1]}2']) and (rootFile[key] == rootFile[f'{key[:-1]}3'])
    # rootFile[key].to_numpy()[1].astype(int).tolist() == [*range(53)]
    # rootFile[key].to_numpy()[2].astype(int).tolist() == [*range(81)]
    df = pandas.DataFrame(rootFile[key].to_numpy()[0]).T
    matplotlib.pyplot.pcolormesh(df, cmap='inferno')
    matplotlib.pyplot.title(key[:-2])
    matplotlib.pyplot.ylabel('row')
    matplotlib.pyplot.xlabel('column')
    matplotlib.pyplot.colorbar()
    matplotlib.pyplot.tight_layout()
    matplotlib.pyplot.savefig(f'{key[:-2].replace(" ","")}.png', dpi=300)
    matplotlib.pyplot.close('all')

def main():
    # testFile()
    def usage(): sys.exit('usage:\n./plotOccupancy.py /path/to/histo_occupancy/root/file')
    try: filePath = str(sys.argv[1])
    except: usage()
    if not os.path.isfile(filePath): usage()
    with uproot.open(filePath) as rootFile: # [https://uproot.readthedocs.io/en/latest/uproot.reading.open.html]
        errors = pandas.Series(rootFile['Errors;1'].values()).rename(dict(zip([*range(6)],['timeOut','eventNum','nearFull','trailer','tbm','unknown'])))
        for key in [ k for k in rootFile.keys() if re.match('Occupancy Ch[\d]{2} ROC[\d];1', k) ]:
            print(f'plotting {key[:-2]}...')
            plotOccupancyROC(rootFile, key)
    return errors

if __name__ == "__main__":
    main()
