#!/cvmfs/cms-bril.cern.ch/brilconda3/bin/python3

import getpass
import logging
import os
import pandas
import pathlib
import socket
import sys
import typing

import read

if not 'scx5-c2f06-36' in socket.gethostname():
    sys.exit(logging.error('please run this script from pltoffline'))

user: str = getpass.getuser()
path: pathlib.Path = pathlib.Path(f'/eos/home-{user[0]}/{user}/www/plt-scaler/')
path.mkdir(exist_ok=True)

def dateParser(dt:str) -> pandas.Timestamp:
    return pandas.to_datetime(dt, format='%Y-%m-%d %H:%M:%S', utc=True)

def fillTimestamps(stable_beams: bool = False) -> pandas.DataFrame:
    url = 'https://delannoy.web.cern.ch/fills.csv'
    cols = ['oms_fill_number', 'oms_stable_beams', 'oms_start_time', 'oms_end_time',]
    fills = pandas.read_csv(url, usecols=cols, parse_dates=['oms_start_time', 'oms_end_time'], date_parser=dateParser)
    if stable_beams:
        fills = fills[fills['oms_stable_beams'] == True] # .drop(columns={'oms_stable_beams'})
    fills = fills.rename(columns={'oms_fill_number':'fill', 'oms_start_time':'start', 'oms_end_time':'end'})
    return fills # .set_index('fill')

def fillStartEnd(fills: pandas.DataFrame, fill: int) -> typing.Tuple[pandas.Timestamp]:
    timestamps = fills[fills['fill'] == fill]
    if not timestamps.empty:
        return (timestamps['start'].item(), timestamps['end'].item())

def workloopFiles(start: pandas.Timestamp, end: pandas.Timestamp, logType: str = 'scaler') -> typing.List[pathlib.PosixPath]:
    wlPath = pathlib.Path(f'/localdata/{start.year}/WORKLOOP/')
    wlFiles = pandas.Series(sorted(wlPath.glob(f'Data_{logType.capitalize()}*.dat*'))).rename('files')
    ts = wlFiles.astype(str).str.extract('(\d{8}.\d{6})').set_axis(['timestamps'], axis=1)
    wlFiles.index = pandas.to_datetime(ts.timestamps, format='%Y%m%d.%H%M%S', utc=True)
    try:
        first = wlFiles.index.get_loc(start, method='pad')
    except KeyError:
        return # don't return anything if the first file timestamp is after the start of the fill
    try:
        last = wlFiles.index.get_loc(end, method='backfill')
    except KeyError:
        last = None # catch exception for when there are no files older than the end of the fill (e.g. not transferred yet)
    return wlFiles.iloc[first:last].to_list()

def scaler(fill:int, start: pandas.Timestamp, end: pandas.Timestamp) -> pandas.DataFrame:
    scalerFiles = workloopFiles(start, end)
    if scalerFiles:
        logging.info(f'[{fill}] [{start} : {end}] [{scalerFiles}]')
        try:
            scalerData = pandas.concat([read.scaler(f) for f in scalerFiles])
            scalerData = scalerData[(scalerData['dt'] >= start) & (scalerData['dt'] <= end)]
            if not scalerData.empty:
                # scalerData.to_hdf(f'{path}/{fill}.hd5', key='scaler', mode='w', complevel=5)
                scalerData.to_pickle(f'{path}/{fill}.pkl')
                return scalerData
        except EOFError:
            return

def doAllTheThings():
    fills = fillTimestamps()
    lastFill = int(max(pathlib.Path(f'/eos/home-{user[0]}/{user}/www/plt-scaler/').glob('*.pkl')).stem)
    fills = fills[fills.fill >= lastFill] # fills[fills['start'] > '2022-11-01'].reset_index(drop=True)
    for fill in fills['fill']:
        try:
            start, end = fillStartEnd(fills, fill)
            _ = scaler(fill, start, end)
        except KeyError:
            continue

def fill(fill: int) -> pandas.DataFrame:
    fills = fillTimestamps()
    start, end = fillStartEnd(fills, fill)
    return scaler(fill, start, end)

def main():
    if len(sys.argv) == 1:
        doAllTheThings()
    else:
        try:
            fill(int(sys.argv[1]))
        except IndexError:
            return(logging.error(f'Usage: {sys.argv[0]} [fill]'))


if __name__ == '__main__':
    main()
