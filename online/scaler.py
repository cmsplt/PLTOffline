#!/cvmfs/cms-bril.cern.ch/brilconda310/bin/python3

import logging
import pathlib
import re
import sys
import typing

import pandas

import read

logging.basicConfig(format='%(levelname)s: %(message)s')
log = logging.getLogger(name=__name__)
log.setLevel(level=logging.INFO)

BRIL_TECH = pathlib.Path('/eos/cms/store/group/briltech')
OUT_PATH = BRIL_TECH/'plt/plt-scaler'

def oms(stable_beams: bool = False) -> pandas.DataFrame:
    url = 'https://brildpg.web.cern.ch/fills/oms.csv'
    dtype = dict(fill_number=int, stable_beams=bool, start_time=str, end_time=str)
    fills = pandas.read_csv(url, usecols=dtype.keys(), dtype=dtype, parse_dates=['start_time', 'end_time'])
    if stable_beams:
        fills = fills[fills.stable_beams == True].reset_index(drop=True)
    return fills

def parseDate(filepath: pathlib.Path) -> pandas.Timestamp:
    timestamp = re.search(pattern='(\d{8}.\d{6})', string=filepath.stem).group()
    return pandas.to_datetime(timestamp, format='%Y%m%d.%H%M%S', utc=True)

def run(fill: int) -> pandas.DataFrame:
    fills = oms()
    if fill not in fills.fill_number.to_list():
        raise ValueError(f'fill {fill} information not available!')
    start, end = fills[fills['fill_number'] == fill][['start_time', 'end_time']].squeeze()
    path = BRIL_TECH/f'plt/{start.year}/WORKLOOP/'
    files = pandas.Series(sorted(path.glob('Data_Scaler*.dat*')))
    timestamps = pandas.Index(parseDate(filepath) for filepath in files)
    scaler_files = files[(timestamps > start) & (timestamps < end)]
    if scaler_files.empty:
        raise ValueError(f'no scaler files corresponding to fill {fill} identified [{start} : {end}]')
    log.info(f'[{fill}] [{start} : {end}] {[file.name for file in scaler_files]}')
    scaler_data = pandas.concat([read.scaler(f) for f in scaler_files])
    scaler_data = scaler_data[(scaler_data['dt'] >= start) & (scaler_data['dt'] <= end)]
    if scaler_data.empty:
        raise ValueError(f'no scaler data in {scaler_files}')
    scaler_data.to_hdf(OUT_PATH/f'{fill}.hdf5', key='_')
    return scaler_data

def process(year: int):
    fills = oms(stable_beams=True)
    fills = fills[(fills.start_time > f'{year}-01-01') & (fills.end_time < f'{year}-12-31')]
    path = BRIL_TECH/f'plt/{year}/WORKLOOP/'
    files = pandas.Series(sorted(path.glob('Data_Scaler*.dat*')))
    timestamps = pandas.Index(parseDate(filepath) for filepath in files)
    for idx, fill in fills.iterrows():
        scaler_files = files[(timestamps > fill.start_time) & (timestamps < fill.end_time)]
        if scaler_files.empty:
            continue
        log.info(f'[{fill.fill_number}] [{fill.start_time} : {fill.end_time}] {[file.name for file in scaler_files]}')
        scaler_data = pandas.concat([read.scaler(f) for f in scaler_files])
        scaler_data = scaler_data[(scaler_data['dt'] >= fill.start_time) & (scaler_data['dt'] <= fill.end_time)]
        if scaler_data.empty:
            continue
        scaler_data.to_hdf(OUT_PATH/f'{fill.fill_number}.hdf5', key='_')

def main():
    OUT_PATH.mkdir(exist_ok=True)
    run(fill=int(sys.argv[1]))

if __name__ == '__main__':
    main()
