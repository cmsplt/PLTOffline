# 
# ====================================================================================================== *
# VERSION 1.0
# This Python script (BBscanTest.py) exercises the beam-beam deflection and the single-beam orbit shift  *
# that occur during a centered or off-axis beam-beam scan at an LHC IP. The deflection and orbit shift   *
# are calculated in the Python function BB.py, using the Basseti-Erskine formula applied to gaussian,    *
# untilted elliptical beams.                                                                             *
#                                                                                                        *
# Based on the input arguments detailed below, the function BB.py calculates the beam-beam deflection    *
# angle and the single-beam orbit shift for one step in a beam-separation scan in either the x-plane or  *
# the y-plane. This function is designed to be used in a stand-alone, plug-in function in any other      *   
# analysis program; it only requires, as an ancillary function, the pre-compiled Fortran subroutine      *
# errffor.so (that replaces the numerically unstable Python version of the complex error function).      *
#                                                                                                        *   
# For debugging purposes, the present script (BBscanTest.py) also calculates the beam-beam kick          *     
# and the associated orbit shift in the round-beam limit, using the simplified analytical formula        *
# applicable to that case.                                                                               *
#                                                                                                        *
# Note that in this first version:                                                                       *
# - the beam energy is hard-coded to 4 TeV per beam inside BB.py                                         * 
# - the formulas are only valid for pp collisions. p-Pb and Pb-Pb collisions are not yet supported.      *
#                                                                                                        *
# Usage:                                                                                                 *
# - specify the input bunch parameters in BBscanTest.py                                                  *
# - specify the input scan parameters in BBscanTest.py                                                   * 
# - type (e.g. on lxplus): python BBscanTest.py. 2 plots will appear:                                    * 
#   the deflection angle & the orbit shift as a function of the ebam separation                          * 
#                                                                                                        *
# T. Pieloni and W. Kozanecki, v. 1.0, 26 April 2013                                                     *   
# ====================================================================================================== *
#
# VERSION 1.1
# Beam Energy not hard coded in BB.py but input argument for calculations in the BB.py script
# T. Pieloni: 10 July 2013
#
#========================================================================================================*
import numpy as np
from numpy import sqrt, pi, exp,sign

import matplotlib.pyplot as plt
from BB import BB



if __name__ == '__main__':

   # ***************** Specify input bunch parameters **********************
    # 
    # Intensity of opposing bunch (units: # protons)
    Np=1.15e11

    # Measured CapSigma values (units: microns)
    Csigx=16.
    Csigy=16.


    # IP beta functions beta* of deflected bunch (units: m)
    betax = 0.60
    betay = 0.60
    
    # LHC tunes (units: 2*pi)
    tunex = 64.31
    tuney = 59.32

    # The beam energy is given as an imput to BB.py

    # 
    # ***************** End of input bunch parameters **********************


    # ***************** Specify input scan parameters **********************       
    #    
    # Define the plane of scan (H or V)
    plane='H'

    # Define magnitude of out-of-plane offset (in units of CapSigma) for offset scan
    offset = 0.
    #
    # ***************** End of input scan parameters **********************

    # ============================ END OF USER INTERFACE ====================================


    # ********* The following lines are NOT used for the elliptical beam calculation;   *****
    #         they are only needed if one wants to run also the round-beam formula as a check
    # ***************************************************************************************
    # ******


    #Mass in atomic units 
    A=1

    # Conversion of CapSigma to the units used by the round-beam formula
    sigxeff=Csigx*1e-6
    sigyeff=Csigy*1e-6
    
    #Collision Energy in eV
    Ep=4e12
    Epr=A*938.272*1e6
    gamma=Ep/Epr

    # Proton classical radius in meter    
    rp=1.53e-18


    K = 2.*rp/gamma

    # ****
    # **********   End of round-beam comparison special *****************************************


    # ************ Define variables for plotting    

    x=[];
    x1 = [];
    x2 = []; 
    y1 = [];
    y2 = [];
    y3 = [];
    y4 = [];
    y5 = [];
    y6 = [];
    y7 = [];
    y8 = [];
    y9 = [];
    y10 = [];
    # ****************************************************************************************



    # ***************** Setup internal scan parameters ******************************
    #        CapSigma's (defined above) are in microns
    #        Separations (as input to BB.py) must be in mm
    #        Internal loop variables (Scan, Off, d) are in m for historical reasons

    
    if plane == 'H':
        Scan = Csigx*1e-6
        Off = offset*Scan
    elif plane == 'V':
        Scan = Csigy*1e-6
        Off = offset*Scan
    else:
        print "Wrong definition of scan plane:",plane,"must be H or V"
        sys.exit(0)     

    # ***************** End of setup internal scan parameters ******************************



    # ********************* Carry out H scan or V scan ******************************

    for d in np.arange(-6.*Scan,+6.*Scan,1e-3*Scan):

        if plane == 'H':
            sepx = d*1e3
            sepy = Off*1e3
            sepx1 = d
            sepy1 = Off

        elif plane == 'V':
            sepy = d*1e3
            sepx = Off*1e3
            sepy1 = d
            sepx1 = Off

        # ************** Compute deflection & orbit shift in round-beam model as a check ************
        # ***


        Droundx=K*Np*(sepx1/(sepx1**2+sepy1**2))*(1.-exp(-(sepx1**2+sepy1**2)/(2.*sigxeff**2)))
        Droundy=K*Np*(sepy1/(sepy1**2+sepx1**2))*(1.-exp(-(sepx1**2+sepy1**2)/(2.*sigyeff**2)))
        Oroundx=Droundx*betax*(1./(2.*np.tan(pi*tunex)))
        Oroundy=Droundy*betay*(1./(2.*np.tan(pi*tuney)))

        # ********************************* End of round-beam calculations ***********************


        

        # **** Compute & plot deflection & single-beam orbit shift using elliptical-beam model ********
        # ***

        Dfleix,Dfleiy,Orbx,Orby = BB(Csigx,Csigy,sepx,sepy,betax,betay,tunex,tuney,Np,Ep)
        

        # ******************************** End of elliptical-beam calculations ********************





        x.append(d*1e6)
        x1.append(sepx);
        x2.append(sepy);
        y1.append(Droundx*1e6)
        y2.append(Droundy*1e6)
        y3.append(Dfleix)
        y4.append(Dfleiy)
        y5.append(Oroundx*1e6)
        y6.append(Oroundy*1e6);
        y7.append(Orbx)
        y8.append(Orby)


    # *********  Plot scan curves  ***********
    
    plt.figure(0);
    plt.plot(x,y1,'k-');
    plt.plot(x,y2,'r-');
    plt.plot(x,y3,'b-.');   
    plt.plot(x,y4,'g-.');   
    plt.xlabel('sep micron');
    plt.ylabel('BB Deflection angle [micro rad]');
    plt.legend(["Roundx","Roundy","BEx","BEy"])

    ax=plt.gca()
#    ax.set_ylim(-0.8, 0.8)

    plt.figure(1);
    plt.plot(x,y5,'k-');
    plt.plot(x,y6,'r-');
    plt.plot(x,y7,'b-.');   
    plt.plot(x,y8,'g-.'); 
    plt.xlabel('sep micron');
    plt.ylabel('BB Orbit deflection [micro m]');
    plt.legend(["Roundx","Roundy","BEx","BEy"])
    ax=plt.gca()
    ax.set_ylim(-0.8, 0.8)
    ax.set_xlim(-100, 100)    

    fout = open('data.out','w')
#    fout.write(str(x[0]));
    for i in range(len(x)):
        fout.write(str(x[i])+',');
        fout.write(str(x1[i])+','); 
        fout.write(str(x2[i])+',');
        fout.write(str(y1[i])+',');
        fout.write(str(y2[i])+',');
        fout.write(str(y3[i])+',');
        fout.write(str(y4[i])+',');
        fout.write(str(y5[i])+',');
        fout.write(str(y6[i])+',');
        fout.write(str(y7[i])+',');
        fout.write(str(y8[i]));
        fout.write('\n'); 
    fout.close()
    plt.show();
