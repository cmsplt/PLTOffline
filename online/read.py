#!/cvmfs/cms-bril.cern.ch/brilconda310/bin/python3

import datetime
import functools
import gzip
import logging
import pathlib
import re
import timeit
import typing

import numpy
import pandas

'''https://cmsfence.cern.ch/alcm/note/show-note-details?noteID=CMS DN-2019/028 (see appendix A)'''

logging.basicConfig(format='%(levelname)s: %(message)s')
log = logging.getLogger(name=__name__)
log.setLevel(level=logging.INFO)

NBX = 3564

def timer(func: typing.Callable) -> typing.Callable:
    '''logs execution time for functions.''' # [https://realpython.com/primer-on-python-decorators/#timing-functions]
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        t0 = timeit.default_timer()
        value = func(*args, **kwargs)
        t1 = timeit.default_timer()
        log.debug(f'{func.__name__}(): {t1-t0:.6f} s')
        return value
    return wrapper

@timer
def parseDateTime(ms_since_midnight: pandas.DataFrame, filepath: pathlib.Path) -> pandas.DataFrame:
    '''add datetime column. Slice and increment date if the number of ms since midnight rolls over.'''
    match = re.search('(\d{8})\.(\d{6})', filepath.name)
    if not match:
        raise ValueError(f'no matching datetime pattern (YYYYMMDD.hhmmss) in {filepath.name}')
    date = datetime.datetime.strptime(match.group(1), '%Y%m%d').replace(tzinfo=datetime.timezone.utc)
    rollovers = (ms_since_midnight.astype(numpy.int32).diff() < 0).cumsum() # dtype `numpy.uint32` will not return negative values from `diff()`
    ms_since_midnight = ms_since_midnight + (rollovers * 24*60*60*1000)
    return date + pandas.to_timedelta(ms_since_midnight, unit="ms")


class IO:

    @staticmethod
    def open(filepath: pathlib.Path) -> typing.Tuple[typing.Callable, int]:
        # [How to tell if a file is gzip compressed?](https://stackoverflow.com/a/47080739/13019084)
        with open(filepath, 'rb') as f:
            magic = f.read(2)
        return gzip.open if magic == b'\x1f\x8b' else open

    @classmethod
    @timer
    def readBuffer(cls, filepath: pathlib.Path, dtype: numpy.dtype, offset: int = 0) -> numpy.ndarray:
        openMethod = cls.open(filepath)
        with openMethod(filepath, mode='rb') as file:
            return numpy.frombuffer(buffer=file.read(), dtype=dtype, offset=offset)


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
        scaler = pandas.DataFrame(data=scaler_buffer)
        columns = {col: col.replace('⚁', '').replace('⚂', '') for col in scaler.columns}
        even = scaler[[col for col in scaler.columns if ('⚁' in col)]].rename(columns=columns)
        odd = scaler[[col for col in scaler.columns if ('⚂' in col)]].rename(columns=columns)
        even['ch'], even.index = 2*even.ch, 2*even.index
        odd['ch'], odd.index = 2*odd.ch+1, 2*odd.index+1
        return pandas.concat([even,odd]).sort_index()


class Hist():

    @staticmethod
    def dtype() -> numpy.dtype:
        '''create structured dtype for histogram file'''
        # [Specifying and constructing data types](https://numpy.org/doc/stable/reference/arrays.dtypes.html#specifying-and-constructing-data-types)
        names = ['ms', 'orb', 'nibble', 'ls', 'run', 'fill', 'ch', 'zero_counts']
        formats = [numpy.uint32] * len(names)
        shapes = [()] * (len(names)-1) + [(NBX,)]
        return numpy.dtype(list(zip(names, formats, shapes)))
        # formats = [numpy.uint32] * (len(names)-1) + [numpy.dtype((numpy.uint32, NBX))]
        # return numpy.dtype(list(zip(names, formats)))

    @staticmethod
    @timer
    def parse(hist_buffer: numpy.ndarray) -> pandas.DataFrame:
        header = pandas.DataFrame(data=hist_buffer[list(hist_buffer.dtype.names[:-1])])
        per_bx_rates = hist_buffer['zero_counts'] & 0x1FFF
        data = pandas.Series(list(per_bx_rates), name='per_bx_rate')
        return pandas.concat([header, data], axis=1)

    @staticmethod
    @timer
    def ratePerBX(hist: pandas.DataFrame, ch: int=10) -> pandas.Series:
        per_bx_rate = hist.loc[(hist.ch == ch), 'per_bx_rate']
        return pandas.Series(data=per_bx_rate.sum(axis=0), name=f'ch{ch}')


def scaler(filepath: pathlib.Path = pathlib.Path('/localdata/2018/WORKLOOP/Data_Scaler_20180515.000353.dat.gz')):
    scaler_buffer = IO.readBuffer(filepath=filepath, dtype=Scaler.dtype())
    scaler = Scaler.parse(scaler_buffer)
    scaler.insert(loc=0, column='dt', value=parseDateTime(ms_since_midnight=scaler.ms, filepath=filepath))
    return scaler

def histogram(filepath: pathlib.Path = pathlib.Path('/localdata/2021/WORKLOOP/Data_Histograms_20211031.043548_V2.dat')):
    hist_buffer = IO.readBuffer(filepath=filepath, dtype=Hist.dtype())
    hist = Hist.parse(hist_buffer)
    hist.insert(loc=0, column='dt', value=parseDateTime(ms_since_midnight=hist.ms, filepath=filepath))
    return hist
