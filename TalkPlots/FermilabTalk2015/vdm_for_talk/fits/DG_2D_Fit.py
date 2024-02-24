import FitManager
import ROOT as r
import sys
from vdmUtilities import *

class DG_2D_Fit(FitManager.FitProvider):

    fitDescription = """ 2D Double Gaussian with correlation between X and Y
    f2D = r.TF2("f2D","[9]* ( [8] * exp(-(x-[4])**2/(2*[0]**2)) * exp(-(y-[6])**2/(2*[2]**2)) + (1-[8])* exp(-(x-[5])**2/(2*([1]*[0])**2)) * exp(-(y-[7])**2/(2*([3]*[2])**2))) ")
    f2D.SetParNames("#Sigma_{x}","S_{x}", "#Sigma_{y}","S_{y}","Mean_{x1}","Mean_{x2}","Mean_{y1}","Mean_{y2}","Fraction", "Amp")
    """

    table = []

    table.append(["Scan", "Type", "BCID", "sigma_x","sigma_x_Err", "S_x", "S_x_Err", "sigma_y","sigma_y_Err", "S_y", "S_y_Err", 
                      "Mean_x1", "Mean_x1_Err", "Mean_x2", "Mean_x2_err", "Mean_y1", "Mean_y1_Err", "Mean_y2", "Mean_y2_Err",  
                      "Amp","AmpErr", "Frac","FracErr", "peak", "peakErr", 
                      "integral", "integralErr", "fitStatus", "chi2", "ndof"])


    def doFit2D(self,graph,graphX, graphY, config):

        ExpSigmaX = graphX.GetRMS()*0.5
        StartSigmaX = ExpSigmaX * config['StartSigmaX']

        StartSX = 2*StartSigmaX*config['StartSX']

        ExpSigmaY = graphY.GetRMS()*0.5
        StartSigmaY = ExpSigmaY * config['StartSigmaY']

        StartSY = 2*StartSigmaY*config['StartSY']

        StartMeanX1 = config['StartMeanX1']
        StartMeanX2 = config['StartMeanX2']
        StartMeanY1 = config['StartMeanY1']
        StartMeanY2 = config['StartMeanY2']

        StartFraction = config['StartFraction']

        ExpPeakX = graphX.GetHistogram().GetMaximum()
        ExpPeakY = graphY.GetHistogram().GetMaximum()
        StartAmp = 0.5*(ExpPeakX+ExpPeakY)*config['StartAmp']

        LimitSigmaX_lower = config['LimitsSigmaX'][0]
        LimitSigmaX_upper = config['LimitsSigmaX'][1]

        LimitSigmaY_lower = config['LimitsSigmaY'][0]
        LimitSigmaY_upper = config['LimitsSigmaY'][1]

        LimitSX_lower = config['LimitsSX'][0]
        LimitSX_upper = config['LimitsSX'][1]

        LimitSY_lower = config['LimitsSY'][0]
        LimitSY_upper = config['LimitsSY'][1]

        LimitMeanX1_lower = config['LimitsMeanX1'][0]
        LimitMeanX1_upper = config['LimitsMeanX1'][1]
        LimitMeanY1_lower = config['LimitsMeanY1'][0]
        LimitMeanY1_upper = config['LimitsMeanY1'][1]
        LimitMeanX2_lower = config['LimitsMeanX2'][0]
        LimitMeanX2_upper = config['LimitsMeanX2'][1]
        LimitMeanY2_lower = config['LimitsMeanY2'][0]
        LimitMeanY2_upper = config['LimitsMeanY2'][1]

        LimitFraction_lower = config['LimitsFraction'][0]
        LimitFraction_upper = config['LimitsFraction'][1]

        LimitAmp_lower = config['LimitsAmp'][0]
        LimitAmp_upper = config['LimitsAmp'][1]


        f2D = r.TF2("f2D","[9]* ( [8] * exp(-(x-[4])**2/(2*[0]**2)) * exp(-(y-[6])**2/(2*[2]**2)) + (1-[8])* exp(-(x-[5])**2/(2*([1]*[0])**2)) * exp(-(y-[7])**2/(2*([3]*[2])**2))) ")

        f2D.SetParNames("#Sigma_{x}","S_{x}", "#Sigma_{y}","S_{y}","Mean_{x1}","Mean_{x2}","Mean_{y1}","Mean_{y2}","Fraction", "Amp")

        f2D.SetParameters(StartSigmaX, StartSX, StartSigmaY, StartSY, StartMeanX1, StartMeanX2, StartMeanY1, StartMeanY2, StartFraction, StartAmp)


# if one does not want to set limits, set lower bound larger than upper bound in config file

        if LimitSigmaX_upper > LimitSigmaX_lower:
            f2D.SetParLimits(0, LimitSigmaX_lower,LimitSigmaX_upper)
        if LimitSX_upper > LimitSX_lower:
            f2D.SetParLimits(1, LimitSX_lower,LimitSX_upper)
        if LimitSigmaY_upper > LimitSigmaY_lower:
            f2D.SetParLimits(2, LimitSigmaY_lower,LimitSigmaY_upper)
        if LimitSY_upper > LimitSY_lower:
            f2D.SetParLimits(3, LimitSY_lower,LimitSY_upper)
        if LimitMeanX1_upper > LimitMeanX1_lower:
            f2D.SetParLimits(4, LimitMeanX1_lower,LimitMeanX1_upper)
        if LimitMeanX2_upper > LimitMeanX2_lower:
            f2D.SetParLimits(5, LimitMeanX2_lower,LimitMeanX2_upper)
        if LimitMeanY1_upper > LimitMeanY1_lower:
            f2D.SetParLimits(6, LimitMeanY1_lower,LimitMeanY1_upper)
        if LimitMeanY2_upper > LimitMeanY2_lower:
            f2D.SetParLimits(7, LimitMeanY2_lower,LimitMeanY2_upper)
        if LimitFraction_upper > LimitFraction_lower:
            f2D.SetParLimits(8, LimitFraction_lower,LimitFraction_upper)
        if LimitAmp_upper > LimitAmp_lower:
            f2D.SetParLimits(9, LimitAmp_lower,LimitAmp_upper)


# Some black ROOT magic to get Minuit output into a log file
# see http://root.cern.ch/phpBB3/viewtopic.php?f=14&t=14473, http://root.cern.ch/phpBB3/viewtopic.php?f=13&t=16844, https://agenda.infn.it/getFile.py/access?resId=1&materialId=slides&confId=4933 slide 23

        r.gROOT.ProcessLine("gSystem->RedirectOutput(\".\/minuitlogtmp\/Minuit.log\", \"a\");")
        r.gROOT.ProcessLine("gSystem->Info(0,\"Next BCID\");")


        for j in range(5):
            fit = graph.Fit("f2D","S")
            if fit.CovMatrixStatus()==3 and fit.Chi2()/fit.Ndf() < 2: break

        r.gROOT.ProcessLine("gSystem->RedirectOutput(0);")


        fitStatus = -999
        fitStatus = fit.Status()

        sigma_x = f2D.GetParameter("#Sigma_{x}")
        m = f2D.GetParNumber("#Sigma_{x}")
        sigma_x_Err = f2D.GetParError(m)
        S_x = f2D.GetParameter("S_{x}")
        m = f2D.GetParNumber("S_{x}")
        S_x_Err = f2D.GetParError(m)

        sigma_y = f2D.GetParameter("#Sigma_{y}")
        m = f2D.GetParNumber("#Sigma_{y}")
        sigma_y_Err = f2D.GetParError(m)
        S_y = f2D.GetParameter("S_{y}")
        m = f2D.GetParNumber("S_{y}")
        S_y_Err = f2D.GetParError(m)

        mean_x1 = f2D.GetParameter("Mean_{x1}")
        m = f2D.GetParNumber("Mean_{x1}")
        mean_x1_Err = f2D.GetParError(m)

        mean_x2 = f2D.GetParameter("Mean_{x2}")
        m = f2D.GetParNumber("Mean_{x2}")
        mean_x2_Err = f2D.GetParError(m)

        mean_y1 = f2D.GetParameter("Mean_{y1}")
        m = f2D.GetParNumber("Mean_{y1}")
        mean_y1_Err = f2D.GetParError(m)

        mean_y2 = f2D.GetParameter("Mean_{y2}")
        m = f2D.GetParNumber("Mean_{y2}")
        mean_y2_Err = f2D.GetParError(m)

        amp = f2D.GetParameter("Amp")
        m = f2D.GetParNumber("Amp")
        ampErr = f2D.GetParError(m)
        frac = f2D.GetParameter("Fraction")
        m = f2D.GetParNumber("Fraction")
        fracErr = f2D.GetParError(m)

        title = graph.GetTitle()
        title_comps = title.split('_')
        scan = title_comps[1] + "_" + title_comps[2]
        type = "2D"
        bcid = title_comps[3]
        chi2 = f2D.GetChisquare()
        ndof = f2D.GetNDF()
        
        xmax = r.TMath.MaxElement(graph.GetN(),graph.GetX())

        import math
        peak = amp
        peakErr = ampErr

        integral = 2 * math.pi * amp * (frac*sigma_x*sigma_y+(1-frac)*S_x*sigma_x*S_y*sigma_y) 
        integralErr = (sigma_x_Err/sigma_x)*(sigma_x_Err/sigma_x) + (sigma_y_Err/sigma_y)*(sigma_y_Err/sigma_y) + (ampErr/amp)*(ampErr/amp)
        integralErr = math.sqrt(integralErr)

        self.table.append([scan, type, bcid, sigma_x, sigma_x_Err, S_x, S_x_Err, sigma_y, sigma_y_Err, S_y, S_y_Err, 
                           mean_x1, mean_x1_Err, mean_x2, mean_x2_Err, mean_y1, mean_y1_Err, mean_y2, mean_y2_Err,  
                           amp, ampErr, frac, fracErr, peak, peakErr, integral, integralErr, fitStatus, chi2, ndof])




#        fit.Print()

# Define X and Y part of full function separately, for plotting

        p = [f2D.GetParameter(i) for i in range(0,10)]
        pErr = [f2D.GetParError(i) for i in range(0,10)]

        xmax = r.TMath.MaxElement(graphX.GetN(),graphX.GetX())
        ymax = r.TMath.MaxElement(graphY.GetN(),graphY.GetX())

        f_y0 = r.TF1("f_y0","[9]* ( [8] * exp(-(x-[4])**2/(2*[0]**2)) * exp(-([6])**2/(2*[2]**2)) + (1-[8])* exp(-(x-[5])**2/(2*([1]*[0])**2)) * exp(-([7])**2/(2*([3]*[2])**2))) ", -1.5 * xmax, 1.5* xmax)

        for i in range(0,10):
                f_y0.SetParameter(i, p[i])
                f_y0.SetParError(i, pErr[i])
        f_y0.SetChisquare(f2D.GetChisquare())
        f_y0.SetNDF(f2D.GetNDF())
        f_y0.SetParNames("#Sigma_{x}","S_{x}", "#Sigma_{y}","S_{y}","Mean_{x1}","Mean_{x2}","Mean_{y1}","Mean_{y2}","Fraction", "Amp")

        f_x0 = r.TF1("f_x0","[9]* ( [8] * exp(-([4])**2/(2*[0]**2)) * exp(-(x-[6])**2/(2*[2]**2)) + (1-[8])* exp(-([5])**2/(2*([1]*[0])**2)) * exp(-(x-[7])**2/(2*([3]*[2])**2))) ", -1.5 * ymax, 1.5* ymax)

        for i in range(0,10):
                f_x0.SetParameter(i, p[i])
                f_x0.SetParError(i, pErr[i])
        f_x0.SetChisquare(f2D.GetChisquare())
        f_x0.SetNDF(f2D.GetNDF())
        f_x0.SetParNames("#Sigma_{x}","S_{x}", "#Sigma_{y}","S_{y}","Mean_{x1}","Mean_{x2}","Mean_{y1}","Mean_{y2}","Fraction", "Amp")


        functions = [f2D, f_y0, f_x0]

        print "#chi^{2}/ndof " + str(round(f2D.GetChisquare(),1))+"/"+str(f2D.GetNDF())

        return [functions, fit]


    def doPlot2D(self, graphX, graphY, functions, fill):

        canvasX =  r.TCanvas()
        funcX = [functions[1]]
        canvasX = doPlot1D(graphX, funcX, fill)
        canvasY =  r.TCanvas()
        funcY = [functions[2]]
        canvasY = doPlot1D(graphY, funcY, fill)
        return [canvasX, canvasY]
    

