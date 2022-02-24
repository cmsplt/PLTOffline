#!/cvmfs/cms-bril.cern.ch/brilconda3/bin/python3

import pandas

import identifyFiles
import plotScaler
import read

def strToDT(dt:str) -> pandas.Timestamp:
    return pandas.to_datetime(dt, format='%Y-%m-%d %H:%M:%S', utc=True)

def plotScalers(fill:int, **kwargs): # , start:pandas.Timestamp=None, end:pandas.Timestamp=None):
    start, end  = identifyFiles.fillTS(fill)
    files = identifyFiles.findFiles((start, end), 'scaler')
    scaler = pandas.concat([read.scaler(f) for f in files])
    for ch in scaler.ch.unique().tolist():
        plotScaler.plotScaler(scaler=scaler, ch=ch, title=f'fill{fill}', start=kwargs.get('start', start), end=kwargs.get('end', end))

def parseHist(fill:int, start:pandas.Timestamp, end:pandas.Timestamp):
    files = identifyFiles.findFiles((start, end), 'histogram')
    hist = pandas.concat([read.histogram(f) for f in files])
    for ch in scaler.ch.unique().tolist():
        plotScaler.plotScaler(scaler=scaler, ch=ch, title=f'fill{fill}')

def main():
    plotScalers(fill=7515)
    plotScalers(fill=7516)
    plotScalers(fill=7517)
    plotScalers(fill=7518)
    plotScalers(fill=7520)
    plotScalers(fill=7521)
    plotScalers(fill=7524)
    plotScalers(fill=7525)
    plotScalers(fill=7526)
    plotScalers(fill=7527)
    plotScalers(fill=7531)
