# VERION 1.0:
# Function BB tales arguments and calculates the beam-beam deflection angle and orbit effect
# for a step in a in-plane Van der Meer scan which can be either in x-plane or y-plane.
# T. Pieloni and W. Kozanecki 24 April 2013
#
# VERSION 1.1
# Beam Energy not hard coded in BB.py but input argument for calculations in the BB.py script
# T. Pieloni: 10 July 2013

from numpy import sqrt, pi, exp,sign, tan
from errffor import errf


# Define the complex error function from errfff.f fortran program written at CERN by K. Koelbig
# ATTENTION: the python erf function has problems with singularity approaching the round beams case. Do not use it!

def wfun(z):
    x=z.real
    y=z.imag
    wx,wy=errf(x,y)
    return wx+1j*wy


#Solve the Bassetti Eskine formula from original paper CERN-ISR-TH/80-06 for generalize case with Capsig12 and Capsig21 equal zero

def BassErsk(Csigx,Csigy,sepx,sepy):

# The BassErsk routine computes the x and y electric field vectors produced from one bunch by the opposite one with Np particles located at a distance (x,y)             


# Separations are moved from mm to meters

    x=abs(sepx/1e3);
    y=abs(sepy/1e3);

# Calculating back in meter the sigmax and sigmay of paper CERN-ISR-TH/80-06

    sigmax = sqrt((Csigx*(1e-6))**2)
    sigmay = sqrt((Csigy*(1e-6))**2)


#I change the constant factor in front of the fields to have it in units of rp and gamma (K=2.*rp/gamma) and not eps0 which I have to set to 1 instead of eps0=8.854187817620e-12;

    eps0= 1.  

    
    if sigmax>sigmay:
    
        S=sqrt(2*(sigmax*sigmax-sigmay*sigmay));
        factBE=sqrt(pi)*2/(2*eps0*S);
        etaBE=(sigmay/sigmax)*x+1j*(sigmax/sigmay)*y;
        zetaBE=x+1j*y;
        
        val=factBE*(wfun(zetaBE/S)-exp(-x*x/(2*sigmax*sigmax)-y*y/(2*sigmay*sigmay))*wfun(etaBE/S));
           
        Ex=abs(val.imag)*sign(sepx);
        Ey=abs(val.real)*sign(sepy);
    
    else:
    
        S=sqrt(2*(sigmay*sigmay-sigmax*sigmax));
        factBE=sqrt(pi)*2/(2*eps0*S);
        etaBE=(sigmax/sigmay)*y+1j*(sigmay/sigmax)*x;
        yetaBE=y+1j*x;
        
        val=factBE*(wfun(yetaBE/S)-exp(-y*y/(2*sigmay*sigmay)-x*x/(2*sigmax*sigmax))*wfun(etaBE/S));
           
        Ey=abs(val.imag)*sign(sepy);
        Ex=abs(val.real)*sign(sepx);
         
    return Ex, Ey

#The function BB receives as input 


def BB(Csigx,Csigy,sepx,sepy,betax,betay,tunex,tuney,Np,Ep):

# Capsigx == sqrt(sigxb1**2+sigxb2**2) in micrometer
# Capsigy == sqrt(sigyb1**2+sigyb2**2) in micrometer
# Np = intensity of opposite beam in number of protons per bunch
# sepx is the separation in the x plane in mm
# sepy is the separation in the y plane in mm
# betax is the beta function in the x plane in meter
# betay is the beta function in the y plane in meter
# Qx is the betatron tune in the x plane
# Qy is the betatron tune in the y plane
# Retuns the deflection angles due to BB in x and y planes in microrad and the orbit deflections in x and y planes in micrometer
# Ep Beam Energy during VdM in eV

    #Parameters needed
    #Mass in atomic units 
    A=1
    # Proton rest energy
    Epr=A*938.272*1e6
    # relativistic gamma factor 
    gamma=Ep/Epr
    # Proton classical radius in meter
    rp=1.53e-18

    # For the case of round beams the Bassetti Erskine evaluation doens't work. To avoid numerical problems by adding in the Capsigy a contribution of 10**-12. This approximation can be changed allowing with if statement the round beam formula. I implemented as in MADX. 


    if Csigx == Csigy:
       Csigx = Csigy + 0.1e-12


    K = 2*rp/gamma

    Ex,Ey = BassErsk(Csigx,Csigy,sepx,sepy)

    Dfleix = K*Np*Ex
    Dfleiy = K*Np*Ey

    Orbx = Dfleix*betax*(1./(2.*tan(pi*tunex)))
    Orby = Dfleiy*betay*(1./(2.*tan(pi*tuney)))

    return Dfleix*1e6,Dfleiy*1e6,Orbx*1e6,Orby*1e6


