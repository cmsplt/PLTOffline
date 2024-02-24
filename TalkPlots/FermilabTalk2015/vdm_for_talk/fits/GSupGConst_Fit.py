import FitManager
import ROOT as r
import sys
from vdmUtilities import *

class GSupGConst_Fit(FitManager.FitProvider):

    fitDescription = """ Gaussian plus SuperGaussian with const background.
    modifier = 1.1627366340382372
    ff = r.TF1("ff","[5] + [2]*([3]*exp(-(x-[4])**2/(2*([0]*[1]/([3]*[1]+1-[3]))**2)) + (1-[3])*"+str(modifier)+"*exp(-(x-[4])**4/(2*([0]/([3]*[1]+1-[3]))**4)) )")
    ff.SetParNames("#Sigma","#sigma_{1}/#sigma_{2}","Amp","Frac","Mean","Const")

    modelled after the normal DG fit function, i.e.
    double gaussian formula with substition to effective width and widths ration
    CapSigma = h*sigma1 + (1-h)*sigma2
    sigRatio = sigma1/sigma2
    [0] -> [0]*[1]/([3]*[1]+1-[3])
    [1] -> [0]/([3]*[1]+1-[3])
    replace second Gaussian by Supergaussian"""

    table = []

    table.append(["Scan", "Type", "BCID", "sigma","sigmaErr", "sigmaRatio","sigmaRatio_Err","Amp","AmpErr", \
                      "Frac","FracErr","Mean","MeanErr", "Const", "ConstErr", "CapSigma", "CapSigmaErr", "peak", "peakErr", \
                      "area", "areaErr", "fitStatus", "chi2", "ndof"])


    def doFit( self,graph,config):

        ExpSigma = graph.GetRMS()*0.5
        ExpPeak = graph.GetHistogram().GetMaximum()

        StartSigma = ExpSigma * config['StartSigma']
        LimitSigma_lower = config['LimitsSigma'][0]
        LimitSigma_upper = config['LimitsSigma'][1]

        StartRatio = config['StartRatio']
        LimitRatio_lower = config['LimitsRatio'][0]
        LimitRatio_upper = config['LimitsRatio'][1]

        StartFrac = config['StartFrac']
        LimitFrac_lower = config['LimitsFrac'][0]
        LimitFrac_upper = config['LimitsFrac'][1]

        StartPeak = ExpPeak*config['StartPeak']
        LimitPeak_lower = ExpPeak*config['LimitsPeak'][0]
        LimitPeak_upper = ExpPeak*config['LimitsPeak'][1]

        StartConst = config['StartConst']
        LimitConst_lower = config['LimitsConst'][0]
        LimitConst_upper = config['LimitsConst'][1]

	import math
	Gamma025 = 3.6256099082219083 
	modifier = 1.1627366340382372
	#modifier = math.sqrt(2* math.pi)*math.sqrt(math.sqrt(8))/Gamma025

	ff = r.TF1("ff","[5] + [2]*([3]*exp(-(x-[4])**2/(2*([0]*[1]/([3]*[1]+1-[3]))**2)) + (1-[3])*"+str(modifier)+"*exp(-(x-[4])**4/(2*([0]/([3]*[1]+1-[3]))**4)) )")
	ff.SetParNames("#Sigma","#sigma_{1}/#sigma_{2}","Amp","Frac","Mean","Const")

        ff.SetParameters(StartSigma,1.,StartPeak,StartFrac,0.,StartConst)

        if LimitSigma_upper > LimitSigma_lower:
            ff.SetParLimits(0, LimitSigma_lower,LimitSigma_upper)
        if LimitPeak_upper > LimitPeak_lower:
            ff.SetParLimits(2, LimitPeak_lower,LimitPeak_upper)
        if LimitFrac_upper > LimitFrac_lower:
            ff.SetParLimits(3, LimitFrac_lower,LimitFrac_upper)
        if LimitRatio_upper > LimitRatio_lower:
            ff.SetParLimits(3, LimitRatio_lower,LimitRatio_upper)
        if LimitConst_upper > LimitConst_lower:
            ff.SetParLimits(5, LimitConst_lower,LimitConst_upper)

# Some black ROOT magic to get Minuit output into a log file
# see http://root.cern.ch/phpBB3/viewtopic.php?f=14&t=14473, http://root.cern.ch/phpBB3/viewtopic.php?f=13&t=16844, https://agenda.infn.it/getFile.py/access?resId=1&materialId=slides&confId=4933 slide 23

        r.gROOT.ProcessLine("gSystem->RedirectOutput(\".\/minuitlogtmp\/Minuit.log\", \"a\");")
        r.gROOT.ProcessLine("gSystem->Info(0,\"Next BCID\");")

        for j in range(5):
            fit = graph.Fit("ff","S")
            if fit.CovMatrixStatus()==3 and fit.Chi2()/fit.Ndf() < 2: break

        r.gROOT.ProcessLine("gSystem->RedirectOutput(0);")

        fitStatus = -999
        fitStatus = fit.Status()

        sigma = ff.GetParameter("#Sigma")
        m = ff.GetParNumber("Sigma")
        sigmaErr = ff.GetParError(m)
        sigRatio = ff.GetParameter("#sigma_{1}/#sigma_{2}")
        m = ff.GetParNumber("#sigma_{1}/#sigma_{2}")
        sigRatioErr = ff.GetParError(m)
        amp = ff.GetParameter("Amp")
        m = ff.GetParNumber("Amp")
        ampErr = ff.GetParError(m)
        frac = ff.GetParameter("Frac")
        m = ff.GetParNumber("Frac")
        fracErr = ff.GetParError(m)
        mean = ff.GetParameter("Mean")
        m = ff.GetParNumber("Mean")
        meanErr = ff.GetParError(m)
        const = ff.GetParameter("Const")
        m = ff.GetParNumber("Const")
        constErr = ff.GetParError(m)

        title = graph.GetTitle()
        title_comps = title.split('_')
        scan = title_comps[0]
        type = title_comps[1]
        bcid = title_comps[2]
        chi2 = ff.GetChisquare()
        ndof = ff.GetNDF()
        
        xmax = r.TMath.MaxElement(graph.GetN(),graph.GetX())

        import math
        factor = (frac + (1-frac)*modifier)
        peak = amp * factor
        peakErr = ampErr*ampErr * factor*factor + amp*amp*(1-modifier)*(1-modifier)*fracErr*fracErr
        peakErr = math.sqrt(peakErr)
        CapSigma = sigma/factor
        CapSigmaErr = 1/factor/factor*(sigmaErr*sigmaErr + (1-modifier)*(1-modifier)*fracErr*fracErr/factor/factor)
        CapSigmaErr = math.sqrt(CapSigmaErr)  

        sqrttwopi = math.sqrt(2*math.pi)
        CapSigma = (const*2*xmax/sqrttwopi + CapSigma*peak)/(const+peak)
        term1 = (2*xmax*(const+peak)/sqrttwopi - const*2*xmax/sqrttwopi - CapSigma*peak)/(const+peak)/(const+peak)
        term2 = peak/(const+peak)
        term3 = CapSigma/(const+peak)-(2*xmax*const/sqrttwopi - CapSigma*peak)/(const+peak)/(const+peak)
        CapSigmaErr = term1*term1*constErr*constErr + term2*term2*CapSigmaErr*CapSigmaErr + term3*term3*peakErr*peakErr
        CapSigmaErr = math.sqrt(CapSigmaErr)
        peak = const + peak
        peakErr = math.sqrt(constErr*constErr+peakErr*peakErr)
        area  = sqrttwopi*peak*CapSigma
        areaErr = (sqrttwopi*CapSigma*peakErr)*(sqrttwopi*CapSigma*peakErr) + (sqrttwopi*peak*CapSigmaErr)*(sqrttwopi*peak*CapSigmaErr)
        areaErr = math.sqrt(areaErr)

        self.table.append([scan, type, bcid, sigma, sigmaErr, sigRatio, sigRatioErr, amp, ampErr, frac, fracErr, mean, meanErr, const, constErr, CapSigma, CapSigmaErr, peak, peakErr, area, areaErr, fitStatus, chi2, ndof])


# Define signal and background pieces of full function separately, for plotting

        h = frac
        s2 = CapSigma/(h*sigRatio+1-h)
        a1 = amp*h
        a2 = amp*(1-h)*modifier
        s1 = CapSigma*sigRatio/(h*sigRatio+1-h)

        fSignal1 = r.TF1("fSignal1","[2]*exp(-(x-[1])**2/(2*[0]**2))")
        fSignal1.SetParNames("#Sigma","Mean","Amp")
        fSignal1.SetParameters(s1, mean, a1)

        fSignal2 = r.TF1("fSignal2","[2]*exp(-(x-[1])**4/(2*[0]**4))")
        fSignal2.SetParNames("#Sigma","Mean","Amp")
        fSignal2.SetParameters(s2, mean, a2)

        fBckgrd =r.TF1("fBckgrd","[0]")
        fBckgrd.SetParNames("Const")
        fBckgrd.SetParameter(0,const)

        functions = [ff, fSignal1, fSignal2, fBckgrd]

        return [functions, fit]

    def doPlot(self, graph, functions, fill):
        
        canvas =  r.TCanvas()
        canvas = doPlot1D(graph, functions, fill)
        return canvas


    def writeParameters(self):
        
# provides input for FitResults_XXX.pkl        
        
        print "writeParameters still to do"
