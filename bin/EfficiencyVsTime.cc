//Phil Hebda
//10 August 2012
//This script requires as input hit_data.root, which is created by TrackingEfficiencyCastor.

#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TF2.h"
#include "TFile.h"
#include "TTree.h"
#include "TGraphErrors.h"
#include "TMath.h"
using namespace std;

bool reject;
double x_min, x_max, y_min, y_max;
int fit_dim;

double fgaus(double*, double*);
double fgaus2D(double*, double*);
double fexp(double*, double*);
void EfficiencyVsTime(int,int,int);

double fgaus(double *x, double *par)
{
  if (reject && ((fit_dim==1 && x[0] > x_min && x[0] < x_max) || (fit_dim==2 && x[0] > y_min && x[0] < y_max))) {
    TF1::RejectPoint();
    return 0;
  }
  return par[0] * exp(-1.*(TMath::Power(x[0]-par[1],2)/2./TMath::Power(par[2],2)));
}

double fgaus2D(double *x, double *par)
{
  if (reject && x[0] > x_min && x[0] < x_max && x[1] > y_min && x[1] < y_max) {
    TF1::RejectPoint();
    return 0;
  }
  return par[0] * exp(-1.*(TMath::Power(x[0]-par[1],2)/2./TMath::Power(par[2],2)+TMath::Power(x[1]-par[3],2)/2./TMath::Power(par[4],2)));
}

double fexp(double *x, double *par)
{
  if (reject && ((fit_dim==1 && x[0] > x_min && x[0] < x_max) || (fit_dim==2 && x[0] > y_min && x[0] < y_max))) {
    TF1::RejectPoint();
    return 0;
  }
  float mid=0.5*(x_min+x_max);
  //if(fit_dim==1) mid = 0.5*(x_min+x_max);
  //else if(fit_dim==2) mid = 0.5*(y_min+y_max);
  if(x[0] > mid) return par[0]*exp(par[1]*x[0]);
  else return par[2]*exp(par[3]*x[0]);
}

void EfficiencyVsTime(int channel, int fit_dim_, int doBgdSubtraction=1) {
  fit_dim=fit_dim_;
  if(fit_dim != 1 && fit_dim != 2 && fit_dim != 3) return;
  TFile *f = new TFile("hit_data.root","OPEN");
  TTree *data = (TTree*)f->Get("data");
  int eventNo, eventTime, roc1_trial, roc1_success;
  float slopeX, slopeY;
  switch(channel){
  case 13:
    x_min = -0.014; x_max = -0.008; y_min = 0.010; y_max = 0.016; break;
  case 14:
    x_min = -0.012; x_max = -0.004; y_min = -0.008; y_max = -0.004; break;
  case 16:
    x_min = -0.008; x_max = -0.002; y_min = -0.022; y_max = -0.014; break;
  case 24:
    x_min = 0.014; x_max = 0.022; y_min = -0.004; y_max = 0.004; break;
  default: 
    x_min = -0.1; x_max = 0.1; y_min = -0.1; y_max = 0.1; break;
    return;
  }
  if(!doBgdSubtraction){
    x_min = -0.1; x_max = 0.1; y_min = -0.1; y_max = 0.1; 
  }
  const int nbins = 25;
  int nbins_slope = 50;
  float lowedge=0, highedge=0;
  char branch_name[80];
  data->SetBranchAddress("EventNo",&eventNo);
  data->SetBranchAddress("EventTime",&eventTime);
  sprintf(branch_name,"roc1_trial_ch%i",channel);
  data->SetBranchAddress(branch_name,&roc1_trial);
  sprintf(branch_name,"roc1_success_ch%i",channel);
  data->SetBranchAddress(branch_name,&roc1_success);
  sprintf(branch_name,"slopeX_ch%i",channel);
  data->SetBranchAddress(branch_name,&slopeX);
  sprintf(branch_name,"slopeY_ch%i",channel);
  data->SetBranchAddress(branch_name,&slopeY);

  for(int i=0; i<data->GetEntries(); ++i){
    data->GetEntry(i);
    if(i==0){
      lowedge=eventTime;
      highedge=eventTime;
    }
    if(eventTime<lowedge) lowedge = eventTime;
    if(eventTime>highedge) highedge = eventTime;
  }
  cout<<"The time duration is "<<highedge-lowedge<<endl;

  TH1F *hTrial = new TH1F("hTrial","Trials",nbins,lowedge,highedge);
  TH1F *hSuccess = new TH1F("hSuccess","Successes",nbins,lowedge,highedge);
  TH1F *hSlopeX[nbins], *hSlopeY[nbins];
  TH2F *hSlope2D[nbins];
  TH1F *hNum_bgdeff[nbins], *hDen_bgdeff[nbins];
  for(int i=0; i<nbins; ++i){
    hSlopeX[i] = new TH1F("hSlopeX","SlopeX",nbins_slope,-0.1,0.1);
    hSlopeY[i] = new TH1F("hSlopeY","SlopeY",nbins_slope,-0.1,0.1);
    hSlope2D[i] = new TH2F("hSlope2D","Slope2D",nbins_slope,-0.1,0.1,nbins_slope,-0.1,0.1);
    hNum_bgdeff[i] = new TH1F("hNum","SlopeX",nbins_slope,-0.1,0.1);
    hDen_bgdeff[i] = new TH1F("hDen","SlopeX",nbins_slope,-0.1,0.1);
  }
  TH1F *hSlopeXTotal = new TH1F("hSlopeX","SlopeX",nbins_slope,-0.1,0.1);
  TH1F *hSlopeYTotal = new TH1F("hSlopeX","SlopeX",nbins_slope,-0.1,0.1);
  TH2F *hSlope2DTotal = new TH2F("hSlope2D","Slope2D",nbins_slope,-0.1,0.1,nbins_slope,-0.1,0.1);
  TH1F *hNum_bgdeffTotal = new TH1F("hNum","SlopeX",nbins_slope,-0.1,0.1);
  TH1F *hDen_bgdeffTotal = new TH1F("hNum","SlopeX",nbins_slope,-0.1,0.1);

  float bw_offset = hSlopeX[0]->GetBinWidth(1)*0.01;
  x_min = hSlopeX[0]->GetBinLowEdge(hSlopeX[0]->FindBin(x_min+bw_offset));
  x_max = hSlopeX[0]->GetBinLowEdge(hSlopeX[0]->FindBin(x_max-bw_offset)+1);
  y_min = hSlopeX[0]->GetBinLowEdge(hSlopeX[0]->FindBin(y_min+bw_offset));
  y_max = hSlopeX[0]->GetBinLowEdge(hSlopeX[0]->FindBin(y_max-bw_offset)+1);
  cout<<"The signal region is ["<<x_min<<", "<<x_max<<"] x ["<<y_min<<", "<<y_max<<"]\n";

  float x1_fit=0.1, x2_fit=-0.1, y1_fit=0.1, y2_fit=-0.1;
  for(int i=0; i<data->GetEntries(); ++i){
    data->GetEntry(i);
    if(slopeX > x_min && slopeX < x_max && slopeY > y_min && slopeY < y_max){
      hTrial->Fill(eventTime,roc1_trial);
      hSuccess->Fill(eventTime,roc1_success);
    }
    int binNo = hTrial->FindBin(eventTime)-1;
    if(binNo>=0 && binNo<25 && roc1_trial > 0){
      if(slopeY > y_min && slopeY < y_max){
	hSlopeX[binNo]->Fill(slopeX);
	hSlopeXTotal->Fill(slopeX);
      }
      if(slopeX > x_min && slopeX < x_max){
	hSlopeY[binNo]->Fill(slopeY);
	hSlopeYTotal->Fill(slopeY);
      }
      hSlope2D[binNo]->Fill(slopeX,slopeY);
      hSlope2DTotal->Fill(slopeX,slopeY);
      hNum_bgdeff[binNo]->Fill(slopeX,roc1_success);
      hDen_bgdeff[binNo]->Fill(slopeX,roc1_trial);
      hNum_bgdeffTotal->Fill(slopeX,roc1_success);
      hDen_bgdeffTotal->Fill(slopeX,roc1_trial);
    }
    if(roc1_trial==1){
      if(x1_fit>slopeX) x1_fit=slopeX;
      if(x2_fit<slopeX) x2_fit=slopeX;
      if(y1_fit>slopeY) y1_fit=slopeY;
      if(y2_fit<slopeY) y2_fit=slopeY;
    }
  }

  x1_fit = hSlopeX[0]->GetBinLowEdge(hSlopeX[0]->FindBin(x1_fit+bw_offset));
  x2_fit = hSlopeX[0]->GetBinLowEdge(hSlopeX[0]->FindBin(x2_fit-bw_offset)+1);
  y1_fit = hSlopeX[0]->GetBinLowEdge(hSlopeX[0]->FindBin(y1_fit+bw_offset));
  y2_fit = hSlopeX[0]->GetBinLowEdge(hSlopeX[0]->FindBin(y2_fit-bw_offset)+1);
  cout<<"The fit region is ["<<x1_fit<<", "<<x2_fit<<"] x ["<<y1_fit<<", "<<y2_fit<<"]\n";

  float fit1=x1_fit, fit2=x2_fit;
  //if(fit_dim==1) {fit1=x1_fit; fit2=x2_fit;}
  //else if(fit_dim==2) {fit1=y1_fit; fit2=y2_fit;}

  TH1F *hBgdEff[nbins];
  TH1F *hBgdEffTotal;
  float bgdEff[nbins]={0.0}, delta_bgdEff[nbins]={0.0};
  float Nbgd[nbins]={0}, Ntrial[nbins]={0.0}, Nsuccess[nbins]={0.0}, eff_sig[nbins]={0.0}, delta_eff_sig[nbins]={0.0};
  float NbgdTotal=0, NtrialTotal=0, NsuccessTotal=0, bgdEffTotal=0, delta_bgdEffTotal=0, eff_sigTotal=0, delta_eff_sigTotal=0;

  for(int i=0; i<nbins; ++i){

    if(doBgdSubtraction){

      hBgdEff[i] = new TH1F("hBgdEff","SlopeX",nbins_slope,-0.1,0.1);
      hBgdEff[i]->Divide(hNum_bgdeff[i],hDen_bgdeff[i]);
      TF1 *fBgdEff = new TF1("fBgdEff",fexp,fit1,fit2,4);
      fBgdEff->SetParameters(0.1,-10,0.1,10);
      reject=kTRUE;
      hBgdEff[i]->Fit(fBgdEff,"R");
      reject=kFALSE;
      if(fBgdEff->GetChisquare()/fBgdEff->GetNDF()>10.0 || fBgdEff->GetChisquare()/fBgdEff->GetNDF()<0.001)
	cout<<"!!! Bad fit for fBgdEff with bin index == "<<i<<". Chi2 = "<<fBgdEff->GetChisquare()<<", NDF = "<<fBgdEff->GetNDF()<<endl;

      float midpoint = 0.5*(x_min+x_max);
      double pars[4]={0};
      fBgdEff->GetParameters(pars);
      float est1 = pars[0]*exp(pars[1]*midpoint);
      float est2 = pars[2]*exp(pars[3]*midpoint);
      float est = 0.5*(est1+est2);
      float delta_est = TMath::Abs(est1-est2);
      bgdEff[i]=est;
      delta_bgdEff[i] = delta_est;
      
      TH1F *histo1D = 0;
      TF1 *fBgd = 0;
      if(fit_dim==1){
	histo1D = (TH1F*)hSlopeX[i]->Clone("SlopeX");
	fBgd = new TF1("fBgd",fgaus,x1_fit,x2_fit,3);
      }
      else if(fit_dim==2){
	histo1D = (TH1F*)hSlopeY[i]->Clone("SlopeY");
	fBgd = new TF1("fBgd",fgaus,y1_fit,y2_fit,3);
      }
      
      if(fit_dim==3){
	TF2 *f2 = new TF2("f2",fgaus2D,-0.04,0.04,-0.02,0.06,5);
	f2->SetParameters(1000,-0.01,0.01,0.02,0.02);
	reject=kFALSE;
	hSlope2D[i]->Fit(f2);
	reject=kFALSE;
	for(int x_bin=hSlopeX[i]->FindBin(x_min+bw_offset); x_bin<=hSlopeX[i]->FindBin(x_max-bw_offset); ++x_bin){
	  for(int y_bin=hSlopeY[i]->FindBin(y_min+bw_offset); y_bin<=hSlopeY[i]->FindBin(y_max-bw_offset); ++y_bin){
	    float x_value = hSlopeX[i]->GetBinCenter(x_bin);
	    float y_value = hSlopeY[i]->GetBinCenter(y_bin);
	    Nbgd[i] += f2->Eval(x_value,y_value);
	    //ntotal += histo->GetBinContent(x_bin,y_bin);
	    //cout<<x_bin<<' '<<y_bin<<' '<<f2->Eval(x_value,y_value)<<' '<<histo->GetBinContent(x_bin,y_bin)<<endl;
	  }
	}


      }
      else if (fit_dim==1 || fit_dim==2){
	fBgd->SetParameters(100.,0.01,0.01);
	reject=kTRUE;
	histo1D->Fit(fBgd,"R");
	reject=kFALSE;
	if(fBgd->GetChisquare()/fBgd->GetNDF()>10.0 || fBgd->GetChisquare()/fBgd->GetNDF()<0.001)
	  cout<<"!!! Bad fit for fBgd with bin index == "<<i<<endl;
	if(fit_dim==1)
	  for(int j=histo1D->FindBin(x_min+bw_offset); j<=histo1D->FindBin(x_max-bw_offset); ++j)
	    Nbgd[i] += fBgd->Eval(histo1D->GetBinCenter(j));
	else if(fit_dim==2)
	  for(int j=histo1D->FindBin(y_min+bw_offset); j<=histo1D->FindBin(y_max-bw_offset); ++j)
	    Nbgd[i] += fBgd->Eval(histo1D->GetBinCenter(j));
      }
    }

    Ntrial[i] = hTrial->GetBinContent(i+1);
    Nsuccess[i] = hSuccess->GetBinContent(i+1);
    
    if(Ntrial[i]>0){
      eff_sig[i] = (Nsuccess[i]-bgdEff[i]*Nbgd[i])/(Ntrial[i]-Nbgd[i]);
      delta_eff_sig[i] = sqrt(eff_sig[i]*(1.-eff_sig[i])/Ntrial[i]+TMath::Power(Nbgd[i]/Ntrial[i]*delta_bgdEff[i],2));
    }

    NtrialTotal += Ntrial[i];
    NsuccessTotal += Nsuccess[i];
    
  }

  if(doBgdSubtraction){
    hBgdEffTotal = new TH1F("hBgdEff","SlopeX",nbins_slope,-0.1,0.1);
    hBgdEffTotal->Divide(hNum_bgdeffTotal,hDen_bgdeffTotal);
    TF1 *fBgdEffTotal = new TF1("fBgdEff",fexp,fit1,fit2,4);
    fBgdEffTotal->SetParameters(0.1,-10,0.1,10);
    reject=kTRUE;
    hBgdEffTotal->Fit(fBgdEffTotal,"R");
    reject=kFALSE;
    if(fBgdEffTotal->GetChisquare()/fBgdEffTotal->GetNDF()>10.0 || fBgdEffTotal->GetChisquare()/fBgdEffTotal->GetNDF()<0.1)
      cout<<"!!! Bad fit for fBgdEffTotal. Chi2 = "<<fBgdEffTotal->GetChisquare()<<", NDF = "<<fBgdEffTotal->GetNDF()<<endl;
    float midpointTotal = 0.5*(x_min+x_max);
    double parsTotal[4]={0};
    fBgdEffTotal->GetParameters(parsTotal);
    float est1Total = parsTotal[0]*exp(parsTotal[1]*midpointTotal);
    float est2Total = parsTotal[2]*exp(parsTotal[3]*midpointTotal);
    float estTotal = 0.5*(est1Total+est2Total);
    float delta_estTotal = TMath::Abs(est1Total-est2Total);
    bgdEffTotal=estTotal;
    delta_bgdEffTotal = delta_estTotal;

    TH1F *histo1DTotal = 0;
    TF1 *fBgdTotal = 0;
    if(fit_dim==1){
      histo1DTotal = (TH1F*)hSlopeXTotal->Clone("SlopeX");
      fBgdTotal = new TF1("fBgd",fgaus,x1_fit,x2_fit,3);
    }
    else if(fit_dim==2){
      histo1DTotal = (TH1F*)hSlopeYTotal->Clone("SlopeY");
      fBgdTotal = new TF1("fBgd",fgaus,y1_fit,y2_fit,3);
    }
    if(fit_dim==3){
      TF2 *f2 = new TF2("f2",fgaus2D,-0.04,0.04,-0.02,0.06,5);
      f2->SetParameters(1000,-0.01,0.01,0.02,0.02);
      reject=kFALSE;
      hSlope2DTotal->Fit(f2);
      reject=kFALSE;
      for(int x_bin=hSlopeXTotal->FindBin(x_min+bw_offset); x_bin<=hSlopeXTotal->FindBin(x_max-bw_offset); ++x_bin){
	for(int y_bin=hSlopeYTotal->FindBin(y_min+bw_offset); y_bin<=hSlopeYTotal->FindBin(y_max-bw_offset); ++y_bin){
	  float x_value = hSlopeXTotal->GetBinCenter(x_bin);
	  float y_value = hSlopeYTotal->GetBinCenter(y_bin);
	  NbgdTotal += f2->Eval(x_value,y_value);
	  //ntotal += histo->GetBinContent(x_bin,y_bin);
	  //cout<<x_bin<<' '<<y_bin<<' '<<f2->Eval(x_value,y_value)<<' '<<histo->GetBinContent(x_bin,y_bin)<<endl;
	}
      } 
    }
    else if (fit_dim==1 || fit_dim==2){
      fBgdTotal->SetParameters(100.,0.01,0.01);
      reject=kTRUE;
      //histo1DTotal->Fit(fBgdTotal,"R");
      hSlopeXTotal->Fit(fBgdTotal,"R");
      reject=kFALSE;
      if(fBgdTotal->GetChisquare()/fBgdTotal->GetNDF()>10.0 || fBgdTotal->GetChisquare()/fBgdTotal->GetNDF()<0.1)
	cout<<"!!! Bad fit for fBgdTotal"<<endl;
      if(fit_dim==1)
	for(int j=histo1DTotal->FindBin(x_min+bw_offset); j<=histo1DTotal->FindBin(x_max-bw_offset); ++j)
	  NbgdTotal += fBgdTotal->Eval(histo1DTotal->GetBinCenter(j));
      else if(fit_dim==2)
	for(int j=histo1DTotal->FindBin(y_min+bw_offset); j<=histo1DTotal->FindBin(y_max-bw_offset); ++j)
	  NbgdTotal += fBgdTotal->Eval(histo1DTotal->GetBinCenter(j));
    }
  }

  cout<<"For the entire run:\n";
  if(doBgdSubtraction){
    cout<<"The background efficiency is "<<bgdEffTotal<<" +/- "<<delta_bgdEffTotal<<endl;
    if(NtrialTotal>0){
      eff_sigTotal = (NsuccessTotal-bgdEffTotal*NbgdTotal)/(NtrialTotal-NbgdTotal);
      delta_eff_sigTotal = sqrt(eff_sigTotal*(1.-eff_sigTotal)/NtrialTotal+TMath::Power(NbgdTotal/NtrialTotal*delta_bgdEffTotal,2));
    }
  }
  else{
    cout<<"There is no background.\n";
    if(NtrialTotal>0){
      eff_sigTotal = (NsuccessTotal)/(NtrialTotal);
      delta_eff_sigTotal = sqrt(eff_sigTotal*(1.-eff_sigTotal)/NtrialTotal);
    }
  }
  cout<<"The signal efficiency is "<<eff_sigTotal<<" +/- "<<delta_eff_sigTotal<<endl;


  TCanvas *c1 = new TCanvas("c1","Efficiencies vs. Time", 400,400);

  TGraphErrors *gEff = new TGraphErrors(nbins);
  for(int i=0; i<nbins; ++i){
    float bin_center = hTrial->GetBinCenter(i+1);
    float bin_error = hTrial->GetBinWidth(i+1)/2.0;
    if(Ntrial[i]>0) {
      gEff->SetPoint(i,bin_center,eff_sig[i]);
      gEff->SetPointError(i,bin_error,delta_eff_sig[i]);
    }
    else{
      gEff->SetPoint(i,bin_center,0.0);
      gEff->SetPointError(i,bin_error,0.25);
    }
    //cout<<i<<' '<<bin_center<<' '<<bin_error<<' '<<eff_sig[i]<<endl;
  }

  gEff->SetLineColor(2);
  gEff->SetLineWidth(3);
  gEff->SetMarkerStyle(21);
  gEff->SetMarkerSize(0.8);
  char title[80];
  if(fit_dim==1) sprintf(title,"Signal Efficiency for Channel %i (SlopeX Fit)",channel);
  else if(fit_dim==2) sprintf(title,"Signal Efficiency for Channel %i (SlopeY Fit)",channel);
  else if(fit_dim==3) sprintf(title,"Signal Efficiency for Channel %i (Slope2D Fit)",channel);
  gEff->SetTitle(title);
  gEff->GetYaxis()->SetTitle("Signal Efficiency");
  gEff->GetYaxis()->SetTitleOffset(1.3);
  gEff->GetXaxis()->SetTitle("Time (ms)");
  gEff->Draw("AP");
  gEff->SetMinimum(0.0);
  gEff->SetMaximum(1.0);

  if(fit_dim==1) sprintf(title,"eff_vs_time_slopeX_ch%i.png",channel);
  else if(fit_dim==2) sprintf(title,"eff_vs_time_slopeY_ch%i.png",channel);
  else if(fit_dim==3) sprintf(title,"eff_vs_time_slope2D_ch%i.png",channel);
  c1->SaveAs(title);

  TCanvas *c2 = new TCanvas("c2","Cross Checks",400,800);
  if(doBgdSubtraction){c2->SetWindowSize(800,800); c2->Divide(2,2); }
  else {c2->SetWindowSize(400,800); c2->Divide(1,2);}

  TGraphErrors *gNsuc = new TGraphErrors(nbins);
  TGraphErrors *gNbgd = new TGraphErrors(nbins);
  TGraphErrors *gNtri = new TGraphErrors(nbins);
  TGraphErrors *gBgdE = new TGraphErrors(nbins);
  for(int i=0; i<nbins; ++i){
    float bin_center = hTrial->GetBinCenter(i+1);
    float bin_error = hTrial->GetBinWidth(i+1)/2.0;
    gNsuc->SetPoint(i,bin_center,Nsuccess[i]);
    gNbgd->SetPoint(i,bin_center,Nbgd[i]);
    gNtri->SetPoint(i,bin_center,Ntrial[i]);
    gBgdE->SetPoint(i,bin_center,bgdEff[i]);
    gBgdE->SetPointError(i,bin_error,delta_bgdEff[i]);
    //gEff->SetPointError(i,bin_error,delta_eff_sig[i]);
    //cout<<i<<' '<<bin_center<<' '<<bin_error<<' '<<eff_sig[i]<<endl;
  }

  c2->cd(1);
  gNsuc->SetLineColor(2);
  gNsuc->SetLineWidth(3);
  gNsuc->SetMarkerStyle(21);
  gNsuc->SetMarkerSize(0.8);
  if(fit_dim==1) sprintf(title,"Number of Signal Events for Channel %i (SlopeX Fit)",channel);
  else if(fit_dim==2) sprintf(title,"Number of Signal Events for Channel %i (SlopeY Fit)",channel);
  else if(fit_dim==3) sprintf(title,"Number of Signal Events for Channel %i (Slope2D Fit)",channel);
  gNsuc->SetTitle(title);
  gNsuc->GetYaxis()->SetTitle("N_{success}");
  gNsuc->GetYaxis()->SetTitleOffset(1.5);
  gNsuc->GetXaxis()->SetTitle("Time (ms)");
  gNsuc->Draw("AP");

  c2->cd(2);
  if(doBgdSubtraction){
    gNbgd->SetLineColor(2);
    gNbgd->SetLineWidth(3);
    gNbgd->SetMarkerStyle(21);
    gNbgd->SetMarkerSize(0.8);
    if(fit_dim==1) sprintf(title,"Number of Background Events for Channel %i (SlopeX Fit)",channel);
    else if(fit_dim==2) sprintf(title,"Number of Background Events for Channel %i (SlopeY Fit)",channel);
    else if(fit_dim==3) sprintf(title,"Number of Background Events for Channel %i (Slope2D Fit)",channel);
    gNbgd->SetTitle(title);
    gNbgd->GetYaxis()->SetTitle("N_{bgd}");
    gNbgd->GetYaxis()->SetTitleOffset(1.3);
    gNbgd->GetXaxis()->SetTitle("Time (ms)");
    gNbgd->Draw("AP");
    c2->cd(3);
  }
  
  gNtri->SetLineColor(2);
  gNtri->SetLineWidth(3);
  gNtri->SetMarkerStyle(21);
  gNtri->SetMarkerSize(0.8);
  if(fit_dim==1) sprintf(title,"Number of Events for Channel %i (SlopeX Fit)",channel);
  else if(fit_dim==2) sprintf(title,"Number of Events for Channel %i (SlopeY Fit)",channel);
  else if(fit_dim==3) sprintf(title,"Number of Events for Channel %i (Slope2D Fit)",channel);
  gNtri->SetTitle(title);
  gNtri->GetYaxis()->SetTitle("N_{trial}");
  gNtri->GetYaxis()->SetTitleOffset(1.5);
  gNtri->GetXaxis()->SetTitle("Time (ms)");
  gNtri->Draw("AP");

  if(doBgdSubtraction){
    c2->cd(4);
    gBgdE->SetLineColor(2);
    gBgdE->SetLineWidth(3);
    gBgdE->SetMarkerStyle(21);
    gBgdE->SetMarkerSize(0.8);
    if(fit_dim==1) sprintf(title,"Background Efficiency for Channel %i (SlopeX Fit)",channel);
    else if(fit_dim==2) sprintf(title,"Background Efficiency for Channel %i (SlopeY Fit)",channel);
    else if(fit_dim==3) sprintf(title,"Background Efficiency for Channel %i (Slope2D Fit)",channel);
    gBgdE->SetTitle(title);
    gBgdE->GetYaxis()->SetTitle("Background Efficiency");
    gBgdE->GetYaxis()->SetTitleOffset(1.3);
    gBgdE->GetXaxis()->SetTitle("Time (ms)");
    gBgdE->Draw("AP");
    gBgdE->SetMinimum(0.0);
    gBgdE->SetMaximum(1.0);
  }

  if(fit_dim==1) sprintf(title,"cross_checks_slopeX_ch%i.png",channel);
  else if(fit_dim==2) sprintf(title,"cross_checks_slopeY_ch%i.png",channel);
  else if(fit_dim==3) sprintf(title,"cross_checks_slope2D_ch%i.png",channel);
  c2->SaveAs(title);

  TCanvas *c3 = new TCanvas("c3","Slope Distributions",400,800);
  c3->Divide(1,2);
  c3->cd(1);
  sprintf(title,"Slope X for Ch %i",channel);
  TH1F *hTotalSlopeX = new TH1F("hTotalSlopeX",title,100,-0.1,0.1);
  hTotalSlopeX->SetStats(0);
  sprintf(title,"Slope Y for Ch %i",channel);
  TH1F *hTotalSlopeY = new TH1F("hTotalSlopeY",title,100,-0.1,0.1);
  hTotalSlopeY->SetStats(0);
  char drawString[80];
  sprintf(drawString,"slopeX_ch%i>>hTotalSlopeX",channel);
  data->Draw(drawString);
  hTotalSlopeX->Draw();
  c3->cd(2);
  sprintf(drawString,"slopeY_ch%i>>hTotalSlopeY",channel);
  data->Draw(drawString);
  hTotalSlopeY->Draw();
  sprintf(title,"slopeDistributions_ch%i.png",channel);
  c3->SaveAs(title);

  f->Close();
  return;
}

int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [Ch#] [FitDim] [BgdSubtraction]" << std::endl;
    return 1;
  }

  int ChNo   = atoi(argv[1]);
  int FitDim = atoi(argv[2]);
  int BgdSub = atoi(argv[3]);

  if(ChNo!=2 && ChNo!=3 && ChNo!=7 && ChNo!=8 && ChNo!=13 && ChNo!=14 && ChNo!=16 && ChNo!=24){
    cout<<"Bad channel number. Try again.\n";
    return 1;
  }

  if(FitDim!=1 && FitDim!=2){
    cout<<"Bad fit dimension number. Try 1 or 2.\n";
    return 1;
  }

  if(BgdSub!=0 && BgdSub!=1){
    cout<<"BgdSubtraction is a bool. Try 0 or 1.\n";
    return 1;
  }

  EfficiencyVsTime(ChNo, FitDim, BgdSub);
  return 0;
}
