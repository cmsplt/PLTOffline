#!/cvmfs/cms-bril.cern.ch/brilconda3/bin/python3

import dataclasses
import datetime
import fileinput
import functools
import gzip
import logging
import os
import pathlib
import re
import timeit
import typing

import numpy
import pandas

# os.environ['NUMEXPR_MAX_THREADS'] = 24

'''https://cmsfence.cern.ch/alcm/note/show-note-details?noteID=CMS DN-2019/028 (see appendix A)'''

logging.basicConfig(format='%(levelname)s: %(message)s')
log = logging.getLogger(name=__name__)
log.setLevel(level=logging.DEBUG)

def timer(func: typing.Callable) -> typing.Callable:
    '''Timer decorator. Logs execution time for functions.''' # [https://realpython.com/primer-on-python-decorators/#timing-functions]
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        t0 = timeit.default_timer()
        value = func(*args, **kwargs)
        t1 = timeit.default_timer()
        logging.debug(f'{func.__name__}(): {t1-t0:.6f} s')
        return value
    return wrapper

@timer
def parseDateTime(milliseconds_since_midnight: pandas.DataFrame, filepath: pathlib.PosixPath) -> pandas.DataFrame:
    '''add datetime column. Slice and increment date if the number of ms since midnight rolls over.'''
    date = pandas.to_datetime(re.search('\d{8}.\d{6}', filepath.name).group(0), format='%Y%m%d.%H%M%S', utc=True).date()
    date = datetime.datetime.combine(date, datetime.time(0, 0, 0), tzinfo=datetime.timezone.utc)
    rollover = milliseconds_since_midnight[milliseconds_since_midnight.astype(numpy.int32).diff() < 0].index # dtype `numpy.uint32` will not return negative values from `diff()`
    if not rollover.size:
        return pandas.to_datetime(date) + pandas.to_timedelta(milliseconds_since_midnight, unit='ms')
    ms = pandas.Series(dtype ='datetime64[ns, UTC]')
    for day in range(rollover.size + 1):
        begin = None if (day == 0) else rollover[day-1]
        end = None if (day == rollover.size) else rollover[day]
        date = pandas.to_datetime(date + datetime.timedelta(days=day))
        time = pandas.to_timedelta(milliseconds_since_midnight, unit='ms')[begin:end] # [How to change the date in pandas datetime column?](https://stackoverflow.com/a/58263679/13019084)
        ms = pandas.concat([ms, pandas.Series(date + time)])
    return ms


class IO:

    @staticmethod
    def open(filepath: pathlib.PosixPath) -> typing.Tuple[typing.Callable, int]:
        # [How to tell if a file is gzip compressed?](https://stackoverflow.com/a/60634210/13019084)
        with gzip.open(filepath, 'rb') as f:
            try:
                f.read(1)
                return gzip.open
            except OSError:
                return open

    @classmethod
    @timer
    def readBuffer(cls, filepath: pathlib.PosixPath, dtype: numpy.dtype, offset: int = 0) -> numpy.ndarray:
        openMethod = cls.open(filepath)
        with openMethod(filepath, mode='rb') as file:
            data = numpy.frombuffer(buffer=file.read(), dtype=dtype, offset=offset)
        return data


class Scaler:

    @classmethod
    def dtype(cls) -> numpy.dtype:
        '''create structured dtype for scaler file'''
        # [Specifying and constructing data types](https://numpy.org/doc/stable/reference/arrays.dtypes.html#specifying-and-constructing-data-types)
        header = [f'ms⚁⚂', 'orb⚁⚂', 'ch⚁⚂']
        single_hits = [f's{roc}{parity}' for parity in '⚁⚂' for roc in '012']
        double_hits = [f'd{roc}{parity}' for parity in '⚁⚂' for roc in '012']
        exclusive_single_hits = ['xs⚁', 'xs⚂']
        double_coincidences = [f'dc{rocs}{parity}' for parity in '⚁⚂' for rocs in ('01', '12', '02')]
        loose_double_coincidences = [f'ldc{rocs}{parity}' for parity in '⚁⚂' for rocs in ('01', '12', '02')]
        names = header + single_hits + double_hits + exclusive_single_hits + double_coincidences + loose_double_coincidences + ['empty']
        return numpy.dtype({'names': names, 'formats': [numpy.uint32] * len(names)})

    @classmethod
    @timer
    def parse(cls, scaler_buffer: numpy.ndarray) -> pandas.DataFrame:
        '''reindex each scaler, which consists of 30 bytes of interlaced even/odd channel data and one trailing zero-byte''' # [https://gitlab.cern.ch/bril/cmsplt/-/blob/master/interface/src/check_scalfile.cpp#L107]
        scaler = pandas.DataFrame(data=scaler_buffer, columns=scaler_buffer.dtype.names)
        columns = {col: col.replace('⚁', '').replace('⚂', '') for col in scaler.columns}
        even = scaler[[col for col in scaler.columns if ('⚁' in col)]].rename(columns=columns)
        odd = scaler[[col for col in scaler.columns if ('⚂' in col)]].rename(columns=columns)
        even['ch'], even.index = 2*even.ch, 2*even.index
        odd['ch'], odd.index = 2*odd.ch+1, 2*odd.index+1
        return pandas.concat([even,odd]).sort_index()


@dataclasses.dataclass
class Hist(Scaler):

    @staticmethod
    def dtype() -> numpy.dtype:
        '''create structured dtype for histogram file'''
        # [Specifying and constructing data types](https://numpy.org/doc/stable/reference/arrays.dtypes.html#specifying-and-constructing-data-types)
        nBX = 3564
        names = ['ms', 'orb', 'nibble', 'ls', 'run', 'fill', 'ch', 'ratesPerBX']
        formats =[numpy.uint32] * len(names)
        shapes = [()] * (len(names)-1) + [(nBX,)]
        return numpy.dtype(list(zip(names, formats, shapes)))

    @staticmethod
    @timer
    def parse(histBuffer: numpy.ndarray) -> pandas.DataFrame:
        header = pandas.DataFrame(data=histBuffer[[*histBuffer.dtype.names[:-1]]])
        data = pandas.Series(data=list(histBuffer['ratesPerBX']&0x1fff)).rename('data') # [Put a 2d Array into a Pandas Series](https://stackoverflow.com/a/38840533/13019084)
        return pandas.concat([header, data], axis=1)

    @staticmethod
    @timer
    def ratePerBX(hist: pandas.DataFrame, ch: int=10) -> pandas.Series:
        bxRate = hist[hist.ch == ch].reset_index(drop=True).data
        nBX = bxRate.iloc[0].shape[0]
        return pandas.Series(bxRate.str[bx].sum() for bx in range(nBX)).rename(f'ch{ch} rate')


def scaler(filepath: pathlib.Path = pathlib.Path('/localdata/2018/WORKLOOP/Data_Scaler_20180515.000353.dat.gz')):
    scaler_buffer = IO.readBuffer(filepath=filepath, dtype=Scaler.dtype())
    scaler = Scaler.parse(scaler_buffer)
    scaler.insert(loc=0, column='dt', value=parseDateTime(milliseconds_since_midnight=scaler.ms, filepath=filepath))
    return scaler

def histogram(filepath: pathlib.Path = pathlib.Path('/localdata/2021/WORKLOOP/Data_Histograms_20211031.043548_V2.dat')):
    hist_buffer = IO.readBuffer(filepath=filepath, dtype=Hist.dtype())
    hist = Hist.parse(hist_buffer)
    hist.insert(loc=0, column='dt', value=parseDateTime(milliseconds_since_midnight=hist.ms, filepath=filepath))
    return hist
