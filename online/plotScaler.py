#!/cvmfs/cms-bril.cern.ch/brilconda3/bin/python3

import datetime
import functools
import gzip
import logging
import os
import pathlib
import sys
import timeit
import typing

import dateutil
import matplotlib.dates
import matplotlib.pyplot
import numpy
import pandas

logging.basicConfig(level=logging.INFO)

def timer(func:typing.Callable) -> typing.Callable:
    '''Timer decorator. Logs execution time for functions.''' # [https://realpython.com/primer-on-python-decorators/#timing-functions]
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        t0 = timeit.default_timer()
        value = func(*args, **kwargs)
        t1 = timeit.default_timer()
        logging.info(f'{func.__name__}(): {t1-t0:.6f} s')
        return value
    return wrapper

@timer
def readBuffer(scalerFile:pathlib.PosixPath) -> typing.List[numpy.ndarray]:
    '''Open {scalerFile} and split decoded buffer into a list of 30-byte numpy arrays.'''
    logging.info(f'Opening {scalerFile}...')
    ext = scalerFile.name.split('.')[-1]
    if ext == 'gz':
        with gzip.open(scalerFile, mode='rb') as scaler:
            data = numpy.frombuffer(scaler.read(), dtype=numpy.uint32)
    elif ext == 'dat':
        with open(scalerFile, mode = 'rb' ) as scaler:
            data = numpy.frombuffer(scaler.read(), dtype=numpy.uint32)
    return numpy.split(data, data.size/30)

@timer
def parseScaler(scalerBuffer:typing.List[numpy.ndarray]) -> pandas.DataFrame:
    '''Reindex each scaler, which consists of 30 bytes of interlaced even/odd channel data and one trailing zero-byte''' # [https://gitlab.cern.ch/bril/cmsplt/-/blob/master/interface/src/check_scalfile.cpp#L107]
    logging.info('DataFraming...')
    cols = ['ms<>','orb<>','ch<>','s0<','s1<','s2<','s0>','s1>','s2>','d0<','d1<','d2<','d0>','d1>','d2>','xs<','xs>','dc01<','dc02<','dc12<','dc01>','dc02>','dc12>','ldc01<','ldc02<','ldc12<','ldc01>','ldc02>','ldc12>','zero']
    df = pandas.DataFrame(data=scalerBuffer, columns=cols, dtype=int)
    even = df[df.columns[df.columns.str.contains('<')]].set_axis(['ms','orb','ch','s0','s1','s2','d0','d1','d2','xs','dc01','dc02','dc12','ldc01','ldc02','ldc12'], axis=1)
    odd = df[df.columns[df.columns.str.contains('>')]].set_axis(['ms','orb','ch','s0','s1','s2','d0','d1','d2','xs','dc01','dc02','dc12','ldc01','ldc02','ldc12'], axis=1)
    even['ch'] = 2 * even['ch']
    odd['ch'] = 2 * odd['ch'] + 1
    return pandas.concat([even,odd]).sort_index().reset_index(drop=True)

@timer
def appendDateTime(scaler:pandas.DataFrame, scalerFile:pathlib.PosixPath) -> pandas.DataFrame:
    '''Add date column. Slice and increment date if the number of ms since midnight rolls over.'''
    date = dateutil.parser.parse(scalerFile.name, fuzzy=True).date()
    date = datetime.datetime.combine(date, datetime.time(0, 0, 0), tzinfo=datetime.timezone.utc)
    rollover = scaler.ms[scaler.ms.diff() < 0].index
    if rollover.size:
        dt = pandas.Series(dtype ='datetime64[ns, UTC]')
        for day in range(rollover.size + 1):
            l = None if day == 0 else rollover[day-1]
            r = None if day == rollover.size == 1 else rollover[day]
            dt = dt.append(pandas.to_datetime(date + datetime.timedelta(days=day)) + pandas.to_timedelta(scaler.ms, unit='ms')[l:r]) # [https://stackoverflow.com/a/58263679/13019084]
    else:
        dt = pandas.to_datetime(date) + pandas.to_timedelta(scaler.ms, unit='ms')
    return pandas.concat([dt.rename('dt'), scaler], axis=1).sort_values(['dt','ch']).reset_index(drop=True)

@timer
def plotScaler(scaler:pandas.DataFrame, ch:int, title:str, **kwargs):
    '''Plot single- and double-plane rates'''
    def plotRates(axis:matplotlib.pyplot.Axes, rate:pandas.Series, ylabel:str, color:str, label:str):
        axis.set(ylabel=ylabel)
        axis.ticklabel_format(style='sci', axis='y', scilimits=(-2,2)) # [https://matplotlib.org/api/_as_gen/matplotlib.axes.Axes.ticklabel_format.html]
        axis.plot(scaler.dt, rate, color=color, label=label)
        axis.legend(loc='upper right', borderpad=0.1, labelspacing=0.1, fancybox=True, framealpha=0.4)
    start = kwargs.get('start', scaler.dt.iloc[0])
    end = kwargs.get('end', scaler.dt.iloc[-1])
    scaler = scaler[(scaler.ch == ch) & (scaler.dt >= start) & (scaler.dt <= end)]
    pandas.plotting.register_matplotlib_converters()
    matplotlib.pyplot.rcParams.update({'font.size': 6})
    matplotlib.pyplot.rc('figure', titlesize=8)
    matplotlib.pyplot.rc('legend', fontsize=5)
    f, (axSingles, axDoubles) = matplotlib.pyplot.subplots(2, sharex=True)
    plotRates(axis=axSingles, rate=scaler.s0, ylabel='Single-Plane Rates', color='b', label='ROC0' )
    plotRates(axis=axSingles, rate=scaler.s1, ylabel='Single-Plane Rates', color='g', label='ROC1' )
    plotRates(axis=axSingles, rate=scaler.s2, ylabel='Single-Plane Rates', color='r', label='ROC2' )
    plotRates(axis=axDoubles, rate=scaler.dc01, ylabel='Double-Coincidence Rates', color='c', label='ROC0-ROC1' )
    plotRates(axis=axDoubles, rate=scaler.dc02, ylabel='Double-Coincidence Rates', color='m', label='ROC0-ROC2' )
    plotRates(axis=axDoubles, rate=scaler.dc12, ylabel='Double-Coincidence Rates', color='y', label='ROC1-ROC2' )
    axDoubles.set(xlabel="Time (UTC)")
    axDoubles.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%Y.%m.%d %Hh%M'))
    matplotlib.pyplot.xticks(rotation=10)
    f.suptitle(f'ReadoutCh{str(ch).zfill(2)} [{title}]')
    matplotlib.pyplot.tight_layout()
    matplotlib.pyplot.savefig(f'plots/{title}-ch{str(ch).zfill(2)}.png', dpi=150)
    # matplotlib.pyplot.show()
    matplotlib.pyplot.close('all')

def main():
    try:
        if pathlib.Path(str(sys.argv[1])).is_file():
            scalerFile = pathlib.Path(str(sys.argv[1]))
    except IndexError:
        return(logging.error(f'Usage: ./{sys.argv[0]} [filePath]'))
    scaler = parseScaler(readBuffer(scalerFile))
    scaler = appendDateTime(scaler=scaler, scalerFile=scalerFile)
    # scaler.to_feather(f'{scalerFiles[0].name[:-4]}.feather')
    for ch in range(16):
        plotScaler(scaler=scaler, ch=ch, title=scalerFile.name)
    os.popen(f'convert plots/{scalerFile.name[:-4]}*.png plots/{scalerFile.name[:-4]}.pdf && rm plots/{scalerFile.name[:-4]}*.png').read()

if __name__ == "__main__":
    main()    
 
