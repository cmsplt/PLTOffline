#!/usr/bin/env python3

import os, sys, io, numpy, pandas
import typing as t # [https://docs.python.org/3/library/typing.html]

# The Slink data files contain the decoded pixel data from the pixel FED, saved into a binary file
    # [PixelFED Version 5 Manual](https://cms-docdb.cern.ch/cgi-bin/DocDB/ShowDocument?docid=1871)
    # [https://github.com/cmsplt/PLTOffline/blob/master/bin/TestBinaryFileReader.cc]
    # [https://github.com/cmsplt/PLTOffline/blob/master/src/PLTBinaryFileReader.cc]
    # [https://github.com/cmsplt/PLTOffline/blob/master/bin/TestBXPattern.cc]

# Slink is a 64-bit format, but the file is formatted as a series of uint32t words
    # 64-bit header word
        # The first 32-bit word containing 0x50 in the top 8 bits and the event number in the next 24 bits
        # The second 32-bit word contains the BX number in the top 12 bits, the FED ID number in the next 12 bits, and 0x00 in the last 8 bits
    # 32-bit hit/error data word(s)
        # Hit data:
            # Bits 31–26: Channel number
            # Bits 25–21: ROC number (starting with 1)
            # Bits 20–16: Double column number
            # Bits 15–08: Pixel number within double column
            # Bits 07–00: Pulse height
        # Error data:
            # ROC ID 26 indicates a "gap" word, used to fill to an even number of 64-bit words
            # ROC ID 27 indicates a "dummy" word, used internally in the pixel FED to ensure that the data is of a minimum length. These are normally automatically removed by the FEDStreamReader
            # ROC ID 28 indicates a FIFO nearly full condition
            # ROC ID 29 indicates a timeout error (i.e. no data was received from a channel when expected)
            # ROC ID 30 indicates an error in the TBM trailer (either a bad trailer, or an error flag sent from the TBM in the trailer)
            # ROC ID 31 indicates an event number error (a mismatch in the event number from the TBM with the number in the FED)
    # 64-bit trailer word
        # The first 32-bit word contains 0xa0 in the top 8 bits, signalling the trailer. The rest of the bits indicate the event length (as a number of 64-bit entries)
        # The second 32-bit word is rewritten by the FEDStreamReader to contain the timestamp, in the standard PLT format (number of miliseconds since midnight of the current day)

def timerDecorator( func ):
    # [https://realpython.com/primer-on-python-decorators/]
    import functools, timeit
    @functools.wraps( func )
    def wrapper( *args, **kwargs ):
        t0      = timeit.default_timer()
        value   = func( *args, **kwargs )
        t1      = timeit.default_timer()
        print( f'\t{func.__name__}(): { t1 - t0 }s' )
        return value
    return wrapper

def testFiles():
    # fill 4984
    if not os.path.isfile('Slink_20160603.153652.dat'):
        os.system('wget https://adelannoy.com/CMS/PLT/delannoy/slink/Slink_20160603.153652.dat') # ~ 200 MB
    if not os.path.isfile('20160603.153652.TestBinaryFileReader'):
        os.system('wget https://adelannoy.com/CMS/PLT/delannoy/slink/20160603.153652.TestBinaryFileReader') # ~1.6 GB

def openSlinkFile(slinkFileName:str, offset:int=0) -> numpy.ndarray:
    # open the entire slink file (compressed or not) into memory as an ndarray with an optional byte offset
    if slinkFileName.endswith('.dat'):
        with open(slinkFileName, mode='rb') as slinkFile:
            if offset != 0: slinkFile.seek(offset, os.SEEK_SET) # [https://stackoverflow.com/a/30126751/13019084]
            return numpy.fromfile(slinkFile, dtype=numpy.uint32)
    elif slinkFileName.endswith('.gz'):
        import gzip
        with gzip.open(slinkFileName, mode='rb') as slinkFile:
            if offset != 0: slinkFile.seek(offset, os.SEEK_SET)
            return numpy.frombuffer(slinkFile.read(), dtype=numpy.uint32)
            # numpy.fromfile(gzip.open(slinkFileName)) yields garbage data
            # "[gzip module] returns a file handle to the compressed stream" [https://github.com/numpy/numpy/issues/10866] [https://stackoverflow.com/a/58549964/13019084]

def readWords(slinkFile:io.BufferedReader) -> t.Tuple[int,int]:
    # read two 32-bit words for a given offset
    w1 = numpy.frombuffer(slinkFile.read(4), dtype=numpy.uint32)[0]
    w2 = numpy.frombuffer(slinkFile.read(4), dtype=numpy.uint32)[0]
    return (w1,w2)

def parseHeader(w1:int, w2:int) -> t.Dict[str,int]:
    # The first 32-bit word contains 0x50 in the top 8 bits and the event number in the next 24 bits
    # The second 32-bit word contains the BX number in the top 12 bits, the FED ID number in the next 12 bits, and 0x00 in the last 8 bits
    if (w1 & 0xff000000) == 0x50000000 and (w2 & 0xff) == 0:
        event   = (w1 & 0xffffff)
        bx      = (w2 & 0xfff00000) >> 20
        fedID   = (w2 & 0xfff00)    >> 8
        return {'event':event, 'bx':bx, 'fedID':fedID}

def parseTrailer(w1:int, w2:int) -> int:
    # The first 32-bit word contains 0xa0 in the top 8 bits, signalling the trailer. The rest of the bits indicate the event length (as a number of 64-bit entries)
    # The second 32-bit word is rewritten by the FEDStreamReader to contain the timestamp, in the standard PLT format (number of miliseconds since midnight of the current day)
    if   (w1 & 0xff000000) == 0xa0000000: return w2
    elif (w2 & 0xff000000) == 0xa0000000: return w1
    # to do: add date from filename and add new day if midnight rolls over during the fill

class mask:
    pls:int  = 0xff
    pix:int  = 0xff00
    dcol:int = 0x1f0000
    roc:int  = 0x3e00000
    ch:int   = 0xfc000000

def decodeDataWord(word:int) -> t.Dict[str, t.Union[str,int]]:
    if (word & 0xfffffff):
        ch  = (word & mask.ch)  >> 26
        roc = (word & mask.roc) >> 21
        if roc > 25:
            return {'id':'err', 'ch':ch, 'roc':roc, **decodeSpecialWord(word,roc)}
        elif 0 < ch < 37:
            col = decodeHit(word)
            roc -= 1 # The fed gives 123, and we use the convention 012
            if roc <= 2:
                # to do: verify pixel is active
                return {'id':'hit', 'ch':ch, 'roc':roc, 'col':col, 'row':convertPixel((word & mask.pix) >> 8), 'adc':(word & mask.pls)}
            else:
                pass

def decodeHit(word:int) -> int:
    if ((word & mask.pix) >> 8) % 2:
        col = ((word & mask.dcol) >> 16) * 2 + 1 # odd
    else:
        col = ((word & mask.dcol) >> 16) * 2 # even
    return col

def convertPixel(pix:int) -> int:
    if (pix == 160) or (pix == 161): pix = 0
    elif pix % 2 == 1: pix = 80-(pix-1)/2
    else: pix = pix/2-80
    return int(abs(pix))

def decodeSpecialWord(word:int, roc:int) -> t.Dict[str,int]:
    # Check for embeded special words: roc > 25 is special, not a hit
    errKeys = ['nan','gap','dummy','fullFIFO','timeout','trailer','eventNumber']
    err = dict(zip(errKeys, len(errKeys)*[0]))
    if (word & 0xffffffff) == 0xffffffff: err['nan'] += 1 # unknown error
    elif (roc == 26): err['gap']         += 1 # ROC ID 26 indicates a "gap" word, used to fill to an even number of 64-bit words
    elif (roc == 27): err['dummy']       += 1 # ROC ID 27 indicates a "dummy" word, used internally in the pixel FED to ensure that the data is of a minimum length. These are normally automatically removed by the FEDStreamReader
    elif (roc == 28): err['fullFIFO']    += 1 # ROC ID 28 indicates a FIFO nearly full condition
    elif (roc == 29): err['timeout']     += 1 # ROC ID 29 indicates a timeout error (i.e. no data was received from a channel when expected)
        # err.update(decodeTimeOutError(word, roc, err))
    elif (roc == 30): err['trailer']  += 1 # ROC ID 30 indicates an error in the TBM trailer (either a bad trailer, or an error flag sent from the TBM in the trailer)
        # err.update(decodeTrailerError(word, roc, err))
    elif (roc == 31): err['eventNumber'] += 1 # ROC ID 31 indicates an event number error (a mismatch in the event number from the TBM with the number in the FED)
    return err

def decodeTrailerError(word:int, roc:int, err:t.Dict[str,int]):
    # trailer error. for ease of decoding split this into FED trailer error and TBM error.
    # it's possible we could actually have BOTH errors, so do this check independently.
    if (word & (0xf00)): err['trailerFED'] += ((word >> 8) & 0xf)
    if (word & (0xff)):  err['trailerTBM'] += (word & 0xff)
    return err

def decodeTimeOutError(word:int, roc:int, err:t.Dict[str,int]):
    # time out error. these are a little complicated because there are two words.
    # Fortunately, the first word is just the pedestal value and can be ignored.
    # Unfortunately, the second word is a little complicated -- rather than storing the channel number normally,
    # here is instead a bit mask showing which channels have the time out error in a group,
    # so we go through this bit mask and create one error per 1 in the mask.
    # print(f'((word & 0xff00000)==0x3b00000):{((word & 0xff00000) == 0x3b00000)} ((word & 0xff00000) == 0x3a00000):{((word & 0xff00000) == 0x3a00000)}')
    if ((word & 0xff00000) == 0x3b00000): pass # first word
    elif ((word & 0xff00000) == 0x3a00000):
        # second word -- contains the channels which have timed out
        group = (word >> 8) & 0x7 # group of input channels
        chMask = word & 0x1f # bitmask for the five channels in the group
        timeoutCounter = (word >> 11) & 0xff
        offset = dict(zip(*[range(8)], [0,4,9,13,18,22,27,31]))
        print(f'group:{group} chMask:{chMask} timeoutCounter:{timeoutCounter}')
        for i in range(5):
            print(f'{(chMask & (1 << i))}')
            if (chMask & (1 << i)):
                ch = offset[group] + i + 1
                print(f'err[timeout{ch}] += {timeoutCounter}')
    else: pass # what the heck, there's an error in our error? for the time being, let's just ignore this
    return err

def decodeEvent(word:int, header:t.Dict[str,int], hits:t.List[t.Dict[str,int]], errs:t.List[t.Dict[str,int]]) -> (t.List[t.Dict[str,int]], t.List[t.Dict[str,int]]):
    # if data := decodeDataWord(word):
    data = decodeDataWord(word)
    if data:
        if data['id'] == 'hit':
            del data['id']
            hits.append({'ts':None, **header, **data})
        elif data['id'] == 'err':
            del data['id']
            errs.append({'ts':None, 'event':header['event'], **data})
    return hits,errs

@timerDecorator
def processEvents():
    slinkFileName = '/localdata/2016/SLINK/Slink_20160603.153652.dat' # fill 4984
    hits,errs = [],[]
    event, eventNum, maxEvents = False, 0, 1000
    with open(slinkFileName, mode='rb') as slinkFile:
        print(f'processing {maxEvents} events...')
        while eventNum <= maxEvents:
            w1, w2 = readWords(slinkFile)
            if event:
                decodeEvent(w1, header, hits, errs)
                decodeEvent(w2, header, hits, errs)
            # if headerWord := [h for h in (parseHeader(w1, w2), parseHeader(w2, w1)) if h]:
            headerWord = [h for h in (parseHeader(w1, w2), parseHeader(w2, w1)) if h]
            if headerWord :
                header = headerWord[0]
                event = True
                eventNum += 1
                continue
            # if trailerWord := [t for t in (parseTrailer(w1, w2), parseTrailer(w2, w1)) if t]:
            trailerWord = [t for t in (parseTrailer(w1, w2), parseTrailer(w2, w1)) if t]
            if trailerWord :
                hits[-1]['ts'], errs[-1]['ts'] = trailerWord[0], trailerWord[0]
                event = False
                continue
    def toDF(inDict):
        _ = pandas.DataFrame(inDict)
        _.ts = _.ts.fillna(method='bfill').astype(int)
        return _
    return (toDF(hits), toDF(errs))

@timerDecorator
def sumErrs(errs:pandas.DataFrame) -> pandas.DataFrame:
    def errEv(errs, ev):
        return pandas.concat([errs.loc[errs.event==ev,['ts','event']].iloc[-1], errs.loc[errs.event==ev,errs.columns[4:]].cumsum().iloc[-1]])
    return pandas.DataFrame(errEv(errs, ev) for ev in errs.event.unique()).reset_index(drop=True)

def validateThisScript(eventNum:int, hits:pandas.DataFrame, sErr:pandas.DataFrame):
    print('\nslink.py')
    h = hits[hits.event==eventNum].reset_index(drop=True)
    e = sErr[sErr.event==eventNum].reset_index(drop=True)
    if not h.empty:
        print(f'Found header for event {eventNum} BX {h.bx.unique()[0]} ({h.bx.unique()[0] + 1})')
        for ch,roc,col,row in zip(h.ch,h.roc,h.col,h.row):
            print(f'IgnoreWord  Hit: {ch}-{roc}@{col},{row}')
        # subtract one from Gap/Dummy for absolutely zero reason except to match TestBinaryFileReader ¯\(°_o)/¯
        print(f'Found trailer for event {eventNum} time {h.ts.unique()[0]} data words {len(h[h.ch!=0]) + e.iloc[:,3:].values.sum() - 1}')
        print(f'Hits: {len(h[h.ch!=0])} Gap/Dummy: {e.gap.loc[0] + e.dummy.loc[0] - 1} Ev#Err: {e.eventNumber.loc[0]} Timeout: {e.timeout.loc[0] + e.trailer.loc[0]} OtherErr: {e.nan.loc[0] + e.fullFIFO.loc[0]}')
        # print(f'Found trailer for event {eventNum} time {h.ts.unique()[0]} data words {len(h[h.ch!=0]) + e.iloc[:,2:].values.sum()}')
        # print(f'Hits: {len(h[h.ch!=0])} Gap/Dummy: {e.gap.loc[0] + e.dummy.loc[0]} Ev#Err: {e.eventNumber.loc[0]} Timeout: {e.timeout.loc[0] + e.trailer.loc[0]} OtherErr: {e.nan.loc[0] + e.fullFIFO.loc[0]}')

def validateTestBinaryFileReader(eventNum:int):
    print('\nTestBinaryFileReader')
    with open('/home/delannoy/20160603.153652.TestBinaryFileReader', mode='r') as valFile:
        # [https://stackoverflow.com/a/11156538/13019084]
        match = False
        for line in valFile:
            if f'Found header for event {eventNum}' in line:
                match = True
            elif f'Found trailer for event {eventNum}' in line:
                print(line.strip('\n'))
                print(next(valFile).strip('\n')) # [https://stackoverflow.com/a/46410276/13019084]
                match = False
                break
            if match:
                print(line.strip('\n'))

if __name__ == "__main__":
    hits, errs = processEvents()
    sErr = sumErrs(errs)
    validateThisScript(4933473, hits, sErr)
    validateTestBinaryFileReader(4933473)
