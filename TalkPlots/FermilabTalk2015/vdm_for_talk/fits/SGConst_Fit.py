import FitManager
import ROOT as r
import sys
from vdmUtilities import *

class SGConst_Fit(FitManager.FitProvider):

    fitDescription = """ Single Gaussian with const background.
    ff = r.TF1("ff","[3] + [2]*exp(-(x-[1])**2/(2*[0]**2))")
    ff.SetParNames("#Sigma","Mean","Amp","Const") """

    table = []


    table.append(["Scan", "Type", "BCID", "sigma","sigmaErr","Amp","AmpErr", \
                      "Mean","MeanErr", "Const", "ConstErr", "CapSigma", "CapSigmaErr", "peak", "peakErr", \
                      "area", "areaErr","fitStatus", "chi2", "ndof"])



    def doFit( self,graph,config):

        ExpSigma = graph.GetRMS()*0.5
        ExpPeak = graph.GetHistogram().GetMaximum()

        StartSigma = ExpSigma * config['StartSigma']
        LimitSigma_lower = config['LimitsSigma'][0]
        LimitSigma_upper = config['LimitsSigma'][1]

        StartPeak = ExpPeak * config['StartPeak']
        LimitPeak_lower = config['LimitsPeak'][0]
        LimitPeak_upper = config['LimitsPeak'][1]

        StartConst = config['StartConst']
        LimitConst_lower = config['LimitsConst'][0]
        LimitConst_upper = config['LimitsConst'][1]


        ff = r.TF1("ff","[3] + [2]*exp(-(x-[1])**2/(2*[0]**2))")
        ff.SetParNames("#Sigma","Mean","Amp","Const")

        ff.SetParameters(StartSigma,0.,StartPeak, StartConst)

        if LimitSigma_upper > LimitSigma_lower:
            ff.SetParLimits(0, LimitSigma_lower,LimitSigma_upper)
        if LimitPeak_upper > LimitPeak_lower:
            ff.SetParLimits(2, LimitPeak_lower,LimitPeak_upper)
        if LimitConst_upper > LimitConst_lower:
            ff.SetParLimits(3, LimitConst_lower,LimitConst_upper)

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
        mean = ff.GetParameter("Mean")
        m = ff.GetParNumber("Mean")
        meanErr = ff.GetParError(m)
        amp = ff.GetParameter("Amp")
        m = ff.GetParNumber("Amp")
        ampErr = ff.GetParError(m)
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
        sqrttwopi = math.sqrt(2*math.pi)
        CapSigma = (const*2*xmax/sqrttwopi + sigma*amp)/(const+amp)
        term1 = (2*xmax*(const+amp)/sqrttwopi - const*2*xmax/sqrttwopi - sigma*amp)/(const+amp)/(const+amp)
        term2 = amp/(const+amp)
        term3 = sigma/(const+amp)-(2*xmax*const/sqrttwopi - sigma*amp)/(const+amp)/(const+amp)
        CapSigmaErr = term1*term1*constErr*constErr + term2*term2*sigmaErr*sigmaErr + term3*term3*ampErr*ampErr
        CapSigmaErr = math.sqrt(CapSigmaErr)
        peak = const + amp
        peakErr = math.sqrt(constErr*constErr+ampErr*ampErr)
        area  = sqrttwopi*peak*CapSigma
        areaErr = (sqrttwopi*CapSigma*peakErr)*(sqrttwopi*CapSigma*peakErr) + (sqrttwopi*peak*CapSigmaErr)*(sqrttwopi*peak*CapSigmaErr)
        areaErr = math.sqrt(areaErr)

        self.table.append([scan, type, bcid, sigma, sigmaErr, amp, ampErr, mean, meanErr, const, constErr, CapSigma, CapSigmaErr, peak, peakErr, area, areaErr, fitStatus, chi2, ndof])


# Define signal and background pieces of full function separately, for plotting

        fSignal = r.TF1("fSignal","[2]*exp(-(x-[1])**2/(2*[0]**2))")
        fSignal.SetParNames("#Sigma","Mean","Amp")
        fSignal.SetParameters(sigma, mean, amp)

        import array
        errors = array.array('d',[sigmaErr, meanErr, ampErr])
        fSignal.SetParErrors(errors)

        fBckgrd =r.TF1("fBckgrd","[0]")
        fBckgrd.SetParNames("Const")
        const = ff.GetParameter("Const")
        m = ff.GetParNumber("Const")
        constErr = ff.GetParError(m)
        fBckgrd.SetParameter(0,const)
        fBckgrd.SetParError(0,constErr)

        functions = [ff, fSignal, fBckgrd]

        return [functions, fit]


    def doPlot(self, graph, functions, fill):
        
        canvas =  r.TCanvas()
        canvas = doPlot1D(graph, functions, fill)
        return canvas
