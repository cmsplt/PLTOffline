#!/usr/bin/env python

from __future__ import annotations
import pathlib

PIXEL_FED_CH = [1,2,4,5,7,8,10,11,13,14,16,17,19,20,22,23]
CH_MAP = [(mfec_slot, mfec_ch, hub) for mfec_slot in [5,7] for mfec_ch in [1,2] for hub in [5,13,21,29]]

def header() -> list[str]:
    return [f'{mfec_hub[0]} {mfec_hub[1]} {mfec_hub[2]} {pix_ch}' for mfec_hub, pix_ch in zip(CH_MAP, PIXEL_FED_CH)]

def fits(pix_ch: int, roc: int) -> list[str]:
    # [TF1 FitFunc("FitFunc", "[0]*x*x + [1]*x + [2] + TMath::Exp( (x-[3]) / [4]  )", 150, 400);](https://github.com/cmsplt/PLTOffline/blob/master/bin/GainCalFastFits.cc#L142)
    fit = ('8.727799E-02', '-2.689344E+01', '2.104635E+03', '2.276370E+02', '5.369915E-01') # https://github.com/cmsplt/PLTOffline/blob/master/GainCal/GainCalFits_Test.dat
    return [f'{pix_ch} {roc} {col} {row} {fit[0]} {fit[1]} {fit[2]} {fit[3]} {fit[4]}' for col in range(52) for row in range(80)]

def main():
    with pathlib.Path('dummy_gaincalfits.dat').open(mode='w') as f:
        f.writelines('\n'.join(header()) + '\n')
        f.write('\n')
        for pix_ch in PIXEL_FED_CH:
            for roc in range(3):
                f.writelines('\n'.join(fits(pix_ch=pix_ch, roc=roc)))
