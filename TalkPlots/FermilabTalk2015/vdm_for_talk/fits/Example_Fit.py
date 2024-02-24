import FitManager
import ROOT as r
from vdmUtilities import *

class Example_Fit(FitManager.FitProvider):

    def doFit(self,graph,config):

        # read config json
        StartSigma = ExpSigma * config['StartSigma']
        etcetc

        # define fit function
        ff = r.TF1("ff","...")                                      
        ff.SetParameters(...)
        ff.SetParNames(...)

        # if one does not want to set limits, set lower bound larger than upper bound in config file
        if LimitSigma_upper > LimitSigma_lower:
            ff.SetParLimits(....)
        etcetc

        # Some black ROOT magic to get Minuit output into a log file
        r.gROOT.ProcessLine("gSystem->RedirectOutput(\".\/minuitlogtmp\/Minuit.log\", \"a\");")
        r.gROOT.ProcessLine("gSystem->Info(0,\"Next BCID\");")

        # do the fitting
        for j in range(5):
            fit = graph.Fit("ff","S")
            if fit.CovMatrixStatus()==3 and fit.Chi2()/fit.Ndf() < 2: break

        r.gROOT.ProcessLine("gSystem->RedirectOutput(0);")

        # in case the fit function consists of several signal and background components, list them here
        functions = [ff, fSignal1, fSignal2, ..., fBckgrd]

        return [functions, fit, graph]

    def doPlot(self, graph, functions, fill):
        # use doPlot1D function from vdmUtilities.py
        canvas =  r.TCanvas()
        canvas = doPlot1D(graph, functions, fill)
        return canvas

    def writeParameters(self):
        # provides input for FitResults_XXX.pkl        
