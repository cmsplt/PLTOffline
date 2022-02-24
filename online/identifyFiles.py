#!/cvmfs/cms-bril.cern.ch/brilconda3/bin/python3

import os
import pandas
import pathlib
import sys
import typing

def fillTS(fill:int) -> typing.Tuple[pandas.Timestamp]:
    fillTS = pandas.read_csv('/localdata/fillTimestamps.csv', parse_dates=[1,2,3,4])
    fillInfo = fillTS[fillTS['fill'] == fill]
    if not fillInfo.empty:
        startTime = fillTS[fillTS['fill'] == fill].start_stable_beam.item() # .strftime('%Y%m%d.%H%M%S')
        endTime = fillTS[fillTS['fill'] == fill].end_stable_beam.item() # .strftime('%Y%m%d.%H%M%S')
        return (startTime, endTime)

def findFiles(timestamps:typing.Tuple[pandas.Timestamp], logType:str='scaler') -> typing.List[pathlib.PosixPath]:
    (start, end) = timestamps
    wlPath = pathlib.Path(f'/localdata/{start.year}/WORKLOOP/')
    wlFiles = pandas.Series(sorted(wlPath.glob(f'Data_{logType.capitalize()}*.dat*'))).rename('files')
    ts = wlFiles.astype(str).str.extract('(\d{8}.\d{6})').set_axis(['timestamps'], axis=1)
    wlFiles.index = pandas.to_datetime(ts.timestamps, format='%Y%m%d.%H%M%S', utc=True)
    first = wlFiles.index.get_loc(start, method='pad')
    try:
        last = wlFiles.index.get_loc(end, method='backfill')
    except KeyError:
        last = None # catch exception for when there are no files older than the end of the fill (e.g. not transferred yet)
    return wlFiles.iloc[first:last].to_list()

def main(fill:int, *args):
    ts = fillTS(fill)
    return findFiles(ts, *args) if ts else None

if __name__ == '__main__':
    fill = int(sys.argv[1])
    print(main(fill))
