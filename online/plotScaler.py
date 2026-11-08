#!/cvmfs/cms-bril.cern.ch/brilconda310/bin/python3

import functools
import logging
import math
import pathlib
import sys
import timeit
import typing

import matplotlib.dates
import matplotlib.pyplot
import pandas

logging.basicConfig(format='%(levelname)s: %(message)s')
log = logging.getLogger(name=__name__)
log.setLevel(level=logging.INFO)

BRIL_TECH = pathlib.Path('/eos/cms/store/group/briltech/')
OUT_PATH = BRIL_TECH/'plt/plt-scaler/plots'

DPI = 400

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

def applyExponentToYLabel(axis: matplotlib.axes.Axes, label: str) -> None:
    max_abs_val = max([abs(val) for val in axis.get_ylim()])
    if (max_abs_val <= 0):
        return
    exponent = math.floor(math.log10(max_abs_val))
    axis.yaxis.set_major_formatter(matplotlib.ticker.FuncFormatter(lambda x, pos: f'{x / (10 ** exponent):g}'))
    axis.set_ylabel(f'{label} ($\\times 10^{{{exponent}}}$)')

def plotRates(axis: matplotlib.pyplot.Axes, x: pandas.Series, y: pandas.Series, ylabel: str, color: str, label: str):
    axis.set(ylabel=ylabel)
    applyExponentToYLabel(axis=axis, label=ylabel)
    axis.scatter(x, y, s=0.5, color=color, alpha=0.5, edgecolors='none', label=label)
    axis.legend(loc='upper right', borderpad=0.1, labelspacing=0.1, fancybox=True, framealpha=0.4)

@timer
def plot(scaler: pandas.DataFrame, ch: int, title: str, **kwargs):
    '''plot single- and double-plane rates for channel `ch`'''
    start = kwargs.get('start', scaler.dt.iloc[0])
    end = kwargs.get('end', scaler.dt.iloc[-1])
    scaler = scaler[(scaler.ch == ch) & (scaler.dt >= start) & (scaler.dt <= end)]
    log.info(f"plotting {title} ch {ch:02d} from '{start}' to '{end}'")
    matplotlib.pyplot.rcParams.update({'font.size': 6})
    matplotlib.pyplot.rc('figure', titlesize=8)
    matplotlib.pyplot.rc('legend', fontsize=5)
    f, (ax_singles, ax_doubles) = matplotlib.pyplot.subplots(2, sharex=True)
    plotRates(axis=ax_singles, x=scaler.dt, y=scaler.s0, ylabel='Single-Plane Rates', color='b', label='ROC0' )
    plotRates(axis=ax_singles, x=scaler.dt, y=scaler.s1, ylabel='Single-Plane Rates', color='g', label='ROC1' )
    plotRates(axis=ax_singles, x=scaler.dt, y=scaler.s2, ylabel='Single-Plane Rates', color='r', label='ROC2' )
    plotRates(axis=ax_doubles, x=scaler.dt, y=scaler.dc01, ylabel='Double-Coincidence Rates', color='c', label='ROC0-ROC1' )
    plotRates(axis=ax_doubles, x=scaler.dt, y=scaler.dc02, ylabel='Double-Coincidence Rates', color='m', label='ROC0-ROC2' )
    plotRates(axis=ax_doubles, x=scaler.dt, y=scaler.dc12, ylabel='Double-Coincidence Rates', color='y', label='ROC1-ROC2' )
    ax_doubles.set(xlabel='Time (UTC)')
    ax_doubles.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%Y.%m.%d %Hh%M'))
    matplotlib.pyplot.xticks(rotation=10)
    f.suptitle(f'{title}: ch{ch:02d}')
    matplotlib.pyplot.tight_layout()
    (OUT_PATH/str(title)).mkdir(exist_ok=True)
    matplotlib.pyplot.savefig(OUT_PATH/f'{title}/ch{ch:02d}.png', dpi=DPI)
    matplotlib.pyplot.close('all')

def processAll():
    for scaler_file in sorted((BRIL_TECH/'plt/plt-scaler').glob('*.hdf5'), key=lambda filepath: int(filepath.stem)):
        if len(list((OUT_PATH/scaler_file.stem).glob('*.png'))) == 16:
            continue
        scaler = pandas.read_hdf(scaler_file)
        for ch in range(16):
            plot(scaler=scaler, ch=ch, title=scaler_file.stem)

def main():
    OUT_PATH.mkdir(exist_ok=True)
    scaler_file = pathlib.Path(str(sys.argv[1]))
    scaler = pandas.read_hdf(scaler_file)
    for ch in range(16):
        plot(scaler=scaler, ch=ch, title=scaler_file.stem)

if __name__ == '__main__':
    main()
