#!/cvmfs/cms-bril.cern.ch/brilconda310/bin/python3

from __future__ import annotations
import pathlib

import matplotlib.pyplot
import pandas

CHANNEL_MAP = dict(zip(range(16), [f'"CMS_PLT/PLT_H{q[0]}/PLT_H{q}/PLTHV_H{q}T{ch}"' for q in ['mN','mF','pN','pF'] for ch in [0,2,1,3]])) # https://twiki.cern.ch/twiki/bin/viewauth/CMS/PLT#PLT_Channel_Map

def parseChannel(log_file: str, ch: int) -> pandas.DataFrame:
    log = pathlib.Path(log_file).read_text(encoding='utf-8-sig').splitlines() # utf-8-sig encoding removes byte order mark ('\ufeff') [https://stackoverflow.com/a/49150749/13019084]
    head = log.index(CHANNEL_MAP.get(ch))
    tail = log.index('', head) # head + log[head:].index('')
    ch_log = pandas.Series(log[head+2 : tail]).rename(f'ch{ch}')
    ch_log = ch_log.str.replace('"', '')
    ch_log = ch_log.str.split(',', expand=True)
    ch_log.columns = log[head+1].split(',')
    ch_log = ch_log.astype(float) # ch_log.apply(pandas.to_numeric)
    ch_log['Date'] = pandas.to_datetime(ch_log.Date, unit='ms', utc=True)
    return ch_log

def mergeIV(year: int, ch: int) -> pandas.DataFrame:
    vmon = parseChannel(log_file=f'{year}-vmon.csv', ch=ch)
    imon = parseChannel(log_file=f'{year}-imon.csv', ch=ch)
    data = pandas.merge(vmon, imon, how='outer')
    data = data.fillna(method='ffill') # CAEN only appends to the log when there is a change to the value; thus, fill all nan by propagating from the last valid value
    return data

def plotIVperMonth(year: int, ch: int):
    data = mergeIV(year=year, ch=ch)
    for month in range(1, 13):
        print(f'{year}-{month:02}')
        month_data = data[data.Date.dt.month == month]
        axis = month_data.plot.scatter(x='ACTUAL_VMON', y='ACTUAL_IMON', xlabel='HV (V)', ylabel='I (uA)', title=f'Ch{ch:02} current draw vs HV during {year}-{month:02}')
        matplotlib.pyplot.savefig(fname=f'{year}-{month:02}-ch{ch:02}-IV.png')
