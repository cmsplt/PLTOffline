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

os.environ['NUMEXPR_MAX_THREADS'] = '24'

logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

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


@dataclasses.dataclass
class IO:

    @classmethod
    @timer
    def readBuffer(cls, logfile:pathlib.PosixPath, dtype:numpy.dtype, offset:int=0) -> numpy.ndarray:
        openMethod = cls.open(logfile)
        with openMethod(logfile, mode='rb') as logfile:
            data = numpy.frombuffer(buffer=logfile.read(), dtype=dtype, offset=offset)
        return data

    @staticmethod
    def open(logfile:pathlib.PosixPath) -> typing.Tuple[typing.Callable, int]:
        # [How to tell if a file is gzip compressed?](https://stackoverflow.com/a/60634210/13019084)
        with gzip.open(logfile, 'rb') as f:
            try:
                f.read(1)
                return gzip.open
            except OSError:
                return open


@dataclasses.dataclass
class Scaler:

    rocs:typing.Tuple[int] = (0,1,2)
    pairs:typing.Tuple[str] = ('01','12','02')
    even:str = '<' # '|'
    odd:str = '>' # '∤'

    @staticmethod
    def triplet(string:str, planes:typing.List[typing.Union[int,str]], parity:str='') -> typing.List[str]:
        return [f'{string}{plane}{parity}' for plane in planes]

    @classmethod
    def interleave(cls, string:str, planes:typing.List[typing.Union[int,str]]) -> typing.List[str]:
        return [*cls.triplet(string, planes, cls.even), *cls.triplet(string, planes, cls.odd)]

    @classmethod
    def interleavedFields(cls) -> typing.List[str]:
        ms = f'ms{cls.even}{cls.odd}'
        orb = f'orb{cls.even}{cls.odd}'
        ch = f'ch{cls.even}{cls.odd}'
        s = cls.interleave('s', cls.rocs)
        d = cls.interleave('d', cls.rocs)
        dc = cls.interleave('dc', cls.pairs)
        ldc = cls.interleave('ldc', cls.pairs)
        return [ms, orb, ch, *s, *d, f'xs{cls.even}', f'xs{cls.odd}', *dc, *ldc, 'zero'] # == ['ms<>','orb<>','ch<>','s0<','s1<','s2<','s0>','s1>','s2>','d0<','d1<','d2<','d0>','d1>','d2>','xs<','xs>','dc01<','dc12<','dc02<','dc01>','dc12>','dc02>','ldc01<','ldc12<','ldc02<','ldc01>','ldc12>','ldc02>','zero']

    @classmethod
    def dtype(cls) -> numpy.dtype:
        '''create structured dtype for scaler file'''
        # [Specifying and constructing data types](https://numpy.org/doc/stable/reference/arrays.dtypes.html#specifying-and-constructing-data-types)
        names = cls.interleavedFields()
        formats = [numpy.uint32] * len(names)
        return numpy.dtype({'names':names, 'formats':formats})

    @classmethod
    def splitParity(cls, scaler:pandas.DataFrame, parity:str):
        scaler = scaler.loc[:, scaler.columns[scaler.columns.str.contains(getattr(cls, parity))]]
        scaler.columns = scaler.columns.str.rstrip('<>')
        if parity == 'even':
            scaler.ch = 2*scaler.ch
            scaler.index = 2*scaler.index
        elif parity == 'odd':
            scaler.ch = 2*scaler.ch+1
            scaler.index = 2*scaler.index+1
        return scaler

    @classmethod
    @timer
    def parse(cls, scalerBuffer:numpy.ndarray) -> pandas.DataFrame:
        '''Reindex each scaler, which consists of 30 bytes of interlaced even/odd channel data and one trailing zero-byte''' # [https://gitlab.cern.ch/bril/cmsplt/-/blob/master/interface/src/check_scalfile.cpp#L107]
        scaler = pandas.DataFrame(data=scalerBuffer, columns=scalerBuffer.dtype.names)
        even = cls.splitParity(scaler, 'even')
        odd = cls.splitParity(scaler, 'odd')
        return pandas.concat([even,odd]).sort_index()

    @staticmethod
    @timer
    def appendDateTime(df:pandas.DataFrame, logfile:pathlib.PosixPath) -> pandas.DataFrame:
        '''Add datetime column. Slice and increment date if the number of ms since midnight rolls over.'''
        date = pandas.to_datetime(re.search('\d{8}.\d{6}', logfile.name).group(0), format='%Y%m%d.%H%M%S', utc=True).date()
        date = datetime.datetime.combine(date, datetime.time(0, 0, 0), tzinfo=datetime.timezone.utc)
        rollover = df.ms[df.ms.astype(numpy.int32).diff() < 0].index # dtype `numpy.uint32` will not return negative values from `diff()`
        if rollover.size:
            DT = pandas.Series(dtype ='datetime64[ns, UTC]')
            for day in range(rollover.size + 1):
                i = None if day == 0 else rollover[day-1]
                j = None if day == rollover.size else rollover[day]
                date = pandas.to_datetime(date + datetime.timedelta(days=day))
                time = pandas.to_timedelta(df.ms, unit='ms')[i:j] # [How to change the date in pandas datetime column?](https://stackoverflow.com/a/58263679/13019084)
                DT = DT.append(date + time) 
        else:
            DT = pandas.to_datetime(date) + pandas.to_timedelta(df.ms, unit='ms')
        return pandas.concat([DT.rename('dt'), df], axis=1).sort_values(['dt','ch']).reset_index(drop=True)


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
    def parse(histBuffer:numpy.ndarray) -> pandas.DataFrame:
        header = pandas.DataFrame(data=histBuffer[[*histBuffer.dtype.names[:-1]]])
        data = pandas.Series(data=list(histBuffer['ratesPerBX']&0x1fff)).rename('data') # [Put a 2d Array into a Pandas Series](https://stackoverflow.com/a/38840533/13019084)
        return pandas.concat([header, data], axis=1)

    @staticmethod
    @timer
    def ratePerBX(hist:pandas.DataFrame, ch:int=10) -> pandas.Series:
        bxRate = hist[hist.ch == ch].reset_index(drop=True).data
        nBX = bxRate.iloc[0].shape[0]
        return pandas.Series(bxRate.str[bx].sum() for bx in range(nBX)).rename(f'ch{ch} rate')


def scaler(scalerFile:pathlib.Path = pathlib.Path('/localdata/2018/WORKLOOP/Data_Scaler_20180515.000353.dat.gz')):
    scalerBuffer = IO.readBuffer(logfile=scalerFile, dtype=Scaler.dtype())
    scaler = Scaler.parse(scalerBuffer)
    return Scaler.appendDateTime(df=scaler, logfile=scalerFile)

def histogram(histFile:pathlib.Path = pathlib.Path('/localdata/2021/WORKLOOP/Data_Histograms_20211031.043548_V2.dat')):
    histBuffer = IO.readBuffer(logfile=histFile, dtype=Hist.dtype())
    hist = Hist.parse(histBuffer)
    hist = Hist.appendDateTime(df=hist, logfile=histFile)
    return Hist.ratePerBX(hist=hist, ch=10)
