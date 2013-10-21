////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Jul  4 19:20:41 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTEvent.h"
#include "PLTU.h"
#include "TLatex.h"


class HitCounter
{
  public:
    HitCounter () {
      NFiducial[0] = 0;
      NFiducial[1] = 0;
      NFiducial[2] = 0;
      NFiducialAndHit[0] = 0;
      NFiducialAndHit[1] = 0;
      NFiducialAndHit[2] = 0;
    }
    ~HitCounter () {}

    int NFiducial[3];
    int NFiducialAndHit[3];
};



// FUNCTION DEFINITIONS HERE
int CheckAlignment (std::string const, std::string const, std::string const);







// CODE BELOW
TH1F* FidHistFrom2D (TH2F* hIN, TString const NewName, int const NBins, PLTPlane::FiducialRegion FidRegion)
{
  // This function returns a TH1F* and YOU are then the owner of
  // that memory.  please delete it when you are done!!!

  int const NBinsX = hIN->GetNbinsX();
  int const NBinsY = hIN->GetNbinsY();
  float const ZMin = 0;//hIN->GetMinimum();
  float const ZMax = hIN->GetMaximum() * (1.0 + 1.0 / (float) NBins);
  std::cout << hIN->GetMaximum() << "  " << ZMax << std::endl;
  int const MyNBins = NBins + 1;

  TString const hNAME = NewName == "" ? TString(hIN->GetName()) + "_1DZFid" : NewName;

  TH1F* h;
  h = new TH1F(hNAME, hNAME, MyNBins, ZMin, ZMax);
  h->SetXTitle("Number of Hits");
  h->SetYTitle("Number of Pixels");
  h->GetXaxis()->CenterTitle();
  h->GetYaxis()->CenterTitle();
  h->SetTitleOffset(1.4, "y");
  h->SetFillColor(40);

  for (int ix = 1; ix <= NBinsX; ++ix) {
    for (int iy = 1; iy <= NBinsY; ++iy) {
      int const px = hIN->GetXaxis()->GetBinLowEdge(ix);
      int const py = hIN->GetYaxis()->GetBinLowEdge(iy);
      if (PLTPlane::IsFiducial(FidRegion, px, py)) {
        if (hIN->GetBinContent(ix, iy) > ZMax) {
          h->Fill(ZMax - hIN->GetMaximum() / (float) NBins);
        } else {
          h->Fill( hIN->GetBinContent(ix, iy) );
        }
      }
    }
  }

  return h;
}

int CheckAlignment (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;

  // Set some basic style for plots
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  PLTEvent Event2(DataFileName, GainCalFileName, AlignmentFileName);

  PLTPlane::FiducialRegion FidRegionHits  = PLTPlane::kFiducialRegion_Diamond;
  PLTPlane::FiducialRegion FidRegionTrack = PLTPlane::kFiducialRegion_m5_m5;
  Event.SetPlaneFiducialRegion(FidRegionHits);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching,PLTPlane::kFiducialRegion_All);
  Event2.SetPlaneFiducialRegion(FidRegionHits);
  Event2.SetPlaneClustering(PLTPlane::kClustering_AllTouching,PLTPlane::kFiducialRegion_All);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Maps for alignment
  std::map<int, TH1F*> hMapTelXROC1;
  std::map<int, TH1F*> hMapTelYROC1;
  std::map<int, TH1F*> hMapTelXROC2;
  std::map<int, TH1F*> hMapTelYROC2;
  std::map<int, TH2F*> hMapTelRotROC1;
  std::map<int, TH2F*> hMapTelRotROC2;
  std::map<int, TH1F*> hMapGloX;
  std::map<int, TH1F*> hMapGloY;
  std::map<int, TH2F*> hMapGloRot;

  TString Name = TString::Format("Telescope_Alignment");
  TCanvas Can1(Name, Name, 600, 600);
  gStyle->SetOptStat(0);
  gStyle->SetStripDecimals(kFALSE);
  gStyle->SetPadLeftMargin(0.10);
  gStyle->SetPadRightMargin(0.10);

  float const PixelDist = 5;

  float bsGX=0.0, bsGY=0.0, bsGZ=0.0; //global coords of the beamspot

  int NEvents=0, NEventsTot=0;

  // Loop over all events in file for telescope alignment
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {
      PLTTelescope* Telescope = Event.Telescope(it);

      int    const Channel = Telescope->Channel();
      size_t const NPlanes = Telescope->NPlanes();

      ++NEventsTot;

      // make them clean events
      if (Telescope->NHitPlanes() < 3 || Telescope->NHitPlanes() != Telescope->NClusters()) {
        continue;
      }

      // require each cluster to have a single pixel
      if (Telescope->Plane(0)->Cluster(0)->NHits() != 1 || Telescope->Plane(1)->Cluster(0)->NHits() != 1 || Telescope->Plane(2)->Cluster(0)->NHits() != 1){
	continue;
      }

      //The event is good. Count it.
      ++NEvents;

      // Make some hists for this telescope
      if (!hMapTelXROC1.count(Channel)) {

	Name = TString::Format("TelescopeXROC1_Ch%i",Channel);
	hMapTelXROC1[Channel] = new TH1F(Name, Name, PLTU::NCOL, -0.5*PLTU::PIXELWIDTH*PLTU::NCOL, 0.5*PLTU::PIXELWIDTH*PLTU::NCOL);
	Name = TString::Format("TelescopeYROC1_Ch%i",Channel);
	hMapTelYROC1[Channel] = new TH1F(Name, Name, PLTU::NROW, -0.5*PLTU::PIXELHEIGHT*PLTU::NROW, 0.5*PLTU::PIXELHEIGHT*PLTU::NROW);
	Name = TString::Format("TelescopeXROC2_Ch%i",Channel);
	hMapTelXROC2[Channel] = new TH1F(Name, Name, PLTU::NCOL, -0.5*PLTU::PIXELWIDTH*PLTU::NCOL, 0.5*PLTU::PIXELWIDTH*PLTU::NCOL);
	Name = TString::Format("TelescopeYROC2_Ch%i",Channel);
	hMapTelYROC2[Channel] = new TH1F(Name, Name, PLTU::NROW, -0.5*PLTU::PIXELHEIGHT*PLTU::NROW, 0.5*PLTU::PIXELHEIGHT*PLTU::NROW);
	Name = TString::Format("TelescopeRotROC1_Ch%i",Channel);
	hMapTelRotROC1[Channel] = new TH2F(Name, Name, PLTU::NCOL*2/3, -0.5*PLTU::PIXELWIDTH*PLTU::NCOL, 0.525*PLTU::PIXELWIDTH*PLTU::NCOL, PLTU::NROW/2, -0.5*PLTU::PIXELHEIGHT*PLTU::NROW+Alignment.LY(Channel,1), 0.525*PLTU::PIXELHEIGHT*PLTU::NROW+Alignment.LY(Channel,1));
	Name = TString::Format("TelescopeRotROC2_Ch%i",Channel);
	hMapTelRotROC2[Channel] = new TH2F(Name, Name, PLTU::NCOL*2/3, -0.5*PLTU::PIXELWIDTH*PLTU::NCOL, 0.525*PLTU::PIXELWIDTH*PLTU::NCOL, PLTU::NROW/2, -0.5*PLTU::PIXELHEIGHT*PLTU::NROW+Alignment.LY(Channel,2), 0.525*PLTU::PIXELHEIGHT*PLTU::NROW+Alignment.LY(Channel,2));
      }

      PLTPlane* Plane[3] = {0x0, 0x0, 0x0};
      for (size_t ip = 0; ip != NPlanes; ++ip) {
        Plane[ Telescope->Plane(ip)->ROC() ] = Telescope->Plane(ip);
      }

      float slopeXwithBS = (Plane[0]->Cluster(0)->GX()-bsGX)/(Plane[0]->Cluster(0)->GZ()-bsGZ);
      float slopeYwithBS = (Plane[0]->Cluster(0)->GY()-bsGY)/(Plane[0]->Cluster(0)->GZ()-bsGZ);

      std::vector<float> telROC1;
      std::vector<float> telROC2;
      Alignment.GtoTXYZ(telROC1,(Plane[1]->Cluster(0)->GZ()-bsGZ)*slopeXwithBS+bsGX,(Plane[1]->Cluster(0)->GZ()-bsGZ)*slopeYwithBS+bsGY,Plane[1]->Cluster(0)->GZ(),Plane[1]->Channel(),Plane[1]->ROC());
      Alignment.GtoTXYZ(telROC2,(Plane[2]->Cluster(0)->GZ()-bsGZ)*slopeXwithBS+bsGX,(Plane[2]->Cluster(0)->GZ()-bsGZ)*slopeYwithBS+bsGY,Plane[2]->Cluster(0)->GZ(),Plane[2]->Channel(),Plane[2]->ROC());

      float deltaXroc1 = telROC1[0]-Plane[1]->Cluster(0)->TX();
      float deltaYroc1 = telROC1[1]-Plane[1]->Cluster(0)->TY();
      float deltaXroc2 = telROC2[0]-Plane[2]->Cluster(0)->TX();
      float deltaYroc2 = telROC2[1]-Plane[2]->Cluster(0)->TY();

      hMapTelXROC1[Channel]->Fill(deltaXroc1);
      hMapTelYROC1[Channel]->Fill(deltaYroc1);
      hMapTelXROC2[Channel]->Fill(deltaXroc2);
      hMapTelYROC2[Channel]->Fill(deltaYroc2);
      hMapTelRotROC1[Channel]->Fill(deltaXroc1,Plane[1]->Cluster(0)->TY());
      hMapTelRotROC2[Channel]->Fill(deltaXroc2,Plane[2]->Cluster(0)->TY());
    }
  }


  //Adjust alignment for the local displacements
  for (std::map<int, TH1F*>::iterator it = hMapTelXROC1.begin(); it != hMapTelXROC1.end(); ++it) {

    int const id = it->first;
    int Channel = id;

    TLatex *lat = new TLatex();
    lat->SetNDC();
    lat->SetTextSize(0.03);
    lat->SetTextColor(kRed+2);

    float originalLXROC1 = Alignment.LX(Channel,1);
    float originalLYROC1 = Alignment.LY(Channel,1);
    float originalLXROC2 = Alignment.LX(Channel,2);
    float originalLYROC2 = Alignment.LY(Channel,2);
    float originalLRROC1 = Alignment.LR(Channel,1);
    float originalLRROC2 = Alignment.LR(Channel,2);

    Alignment.AddToLX(Channel,1,hMapTelXROC1[Channel]->GetMean());
    Alignment.AddToLY(Channel,1,hMapTelYROC1[Channel]->GetMean());
    Alignment.AddToLX(Channel,2,hMapTelXROC2[Channel]->GetMean());
    Alignment.AddToLY(Channel,2,hMapTelYROC2[Channel]->GetMean());
    Alignment.AddToLR(Channel,1,-1.0*(acos(hMapTelRotROC1[Channel]->GetCorrelationFactor())-3.141593/2.0));
    Alignment.AddToLR(Channel,2,-1.0*(acos(hMapTelRotROC2[Channel]->GetCorrelationFactor())-3.141593/2.0));

    Can1.cd(1);
    Name = TString::Format("Telescope Alignment in X for ROC1 Ch%i", Channel);
    hMapTelXROC1[id]->SetTitle(Name);
    hMapTelXROC1[id]->GetXaxis()->SetTitle("deviation in X (cm)");
    hMapTelXROC1[id]->SetFillColor(42);
    hMapTelXROC1[id]->DrawNormalized();
    lat->DrawLatex(0.55,0.85,TString::Format("Avg. dev. = %.3f cm",hMapTelXROC1[id]->GetMean()));
    lat->DrawLatex(0.55,0.81,TString::Format("Original X = %.3f cm",originalLXROC1));
    lat->DrawLatex(0.55,0.77,TString::Format("New X = %.3f cm",Alignment.LX(id,1)));
    Name = TString::Format("plots/TelescopeXalignment_Ch%i_ROC1", Channel);
    Can1.SaveAs(Name + ".gif");
    Can1.SaveAs(Name + ".pdf");

    Can1.Clear();
    Name = TString::Format("Telescope Alignment in Y for ROC1 Ch%i", Channel);
    hMapTelYROC1[id]->SetTitle(Name);
    hMapTelYROC1[id]->GetXaxis()->SetTitle("deviation in Y (cm)");
    hMapTelYROC1[id]->SetFillColor(42);
    hMapTelYROC1[id]->DrawNormalized();
    lat->DrawLatex(0.55,0.85,TString::Format("Avg. dev. = %.3f cm",hMapTelYROC1[id]->GetMean()));
    lat->DrawLatex(0.55,0.81,TString::Format("Original Y = %.3f cm",originalLYROC1));
    lat->DrawLatex(0.55,0.77,TString::Format("New Y = %.3f cm",Alignment.LY(id,1)));
    Name = TString::Format("plots/TelescopeYalignment_Ch%i_ROC1", Channel);
    Can1.SaveAs(Name + ".gif");
    Can1.SaveAs(Name + ".pdf");

    Can1.Clear();
    Name = TString::Format("Telescope Alignment in X for ROC2 Ch%i", Channel);
    hMapTelXROC2[id]->SetTitle(Name);
    hMapTelXROC2[id]->GetXaxis()->SetTitle("deviation in X (cm)");
    hMapTelXROC2[id]->SetFillColor(42);
    hMapTelXROC2[id]->DrawNormalized();
    lat->DrawLatex(0.55,0.85,TString::Format("Avg. dev. = %.3f cm",hMapTelXROC2[id]->GetMean()));
    lat->DrawLatex(0.55,0.81,TString::Format("Original X = %.3f cm",originalLXROC2));
    lat->DrawLatex(0.55,0.77,TString::Format("New X = %.3f cm",Alignment.LX(id,2)));
    Name = TString::Format("plots/TelescopeXalignment_Ch%i_ROC2", Channel);
    Can1.SaveAs(Name + ".gif");
    Can1.SaveAs(Name + ".pdf");

    Can1.Clear();
    Name = TString::Format("Telescope Alignment in Y for ROC2 Ch%i", Channel);
    hMapTelYROC2[id]->SetTitle(Name);
    hMapTelYROC2[id]->GetXaxis()->SetTitle("deviation in Y (cm)");
    hMapTelYROC2[id]->SetFillColor(42);
    hMapTelYROC2[id]->DrawNormalized();
    lat->DrawLatex(0.55,0.85,TString::Format("Avg. dev. = %.3f cm",hMapTelYROC2[id]->GetMean()));
    lat->DrawLatex(0.55,0.81,TString::Format("Original Y = %.3f cm",originalLYROC2));
    lat->DrawLatex(0.55,0.77,TString::Format("New Y = %.3f cm",Alignment.LY(id,2)));
    Name = TString::Format("plots/TelescopeYalignment_Ch%i_ROC2", Channel);
    Can1.SaveAs(Name + ".gif");
    Can1.SaveAs(Name + ".pdf");

    lat->SetTextColor(kBlack);
    Can1.Clear();
    Name = TString::Format("Telescope Rotation for ROC1 Ch%i", Channel);
    hMapTelRotROC1[id]->SetTitle(Name);
    hMapTelRotROC1[id]->GetXaxis()->SetTitle("deviation in X (cm)");
    hMapTelRotROC1[id]->GetYaxis()->SetTitleOffset(1.40);
    hMapTelRotROC1[id]->GetYaxis()->SetTitle("Y position (cm)");
    hMapTelRotROC1[id]->DrawNormalized("col");
    lat->DrawLatex(0.55,0.85,TString::Format("Correlation = %.3f",hMapTelRotROC1[id]->GetCorrelationFactor()));
    lat->DrawLatex(0.55,0.81,TString::Format("Original Angle = %.3f",originalLRROC1));
    lat->DrawLatex(0.55,0.77,TString::Format("New Angle = %.3f",Alignment.LR(id,1)));
    Name = TString::Format("plots/TelescopeRotation_Ch%i_ROC1", Channel);
    Can1.SaveAs(Name + ".gif");
    Can1.SaveAs(Name + ".pdf");

    Can1.Clear();
    Name = TString::Format("Telescope Rotation for ROC2 Ch%i", Channel);
    hMapTelRotROC2[id]->SetTitle(Name);
    hMapTelRotROC2[id]->GetXaxis()->SetTitle("deviation in X (cm)");
    hMapTelRotROC2[id]->GetYaxis()->SetTitleOffset(1.40);
    hMapTelRotROC2[id]->GetYaxis()->SetTitle("Y position (cm)");
    hMapTelRotROC2[id]->DrawNormalized("col");
    lat->DrawLatex(0.55,0.85,TString::Format("Correlation = %.3f",hMapTelRotROC2[id]->GetCorrelationFactor()));
    lat->DrawLatex(0.55,0.81,TString::Format("Original Angle = %.3f",originalLRROC2));
    lat->DrawLatex(0.55,0.77,TString::Format("New Angle = %.3f",Alignment.LR(id,2)));
    Name = TString::Format("plots/TelescopeRotation_Ch%i_ROC2", Channel);
    Can1.SaveAs(Name + ".gif");
    Can1.SaveAs(Name + ".pdf");
  }

  // Loop over all events in file for global alignment
  for (int ientry = 0; Event2.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Reprocessing entry: " << ientry << std::endl;
    }

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event2.NTelescopes(); ++it) {
      PLTTelescope* Telescope = Event2.Telescope(it);

      int    const Channel = Telescope->Channel();
      size_t const NPlanes = Telescope->NPlanes();

      // make them clean events
      if (Telescope->NHitPlanes() < 3 || Telescope->NHitPlanes() != Telescope->NClusters()) {
        continue;
      }

      // require each cluster to have a single pixel
      if (Telescope->Plane(0)->Cluster(0)->NHits() != 1 || Telescope->Plane(1)->Cluster(0)->NHits() != 1 || Telescope->Plane(2)->Cluster(0)->NHits() != 1){
	continue;
      }

      // Make some hists for this telescope
      if (!hMapGloX.count(Channel)) {
	Name = TString::Format("GlobalX_Ch%i",Channel);
	hMapGloX[Channel] = new TH1F(Name, Name, PLTU::NCOL, -4.9*PLTU::PIXELWIDTH*PLTU::NCOL, 5*PLTU::PIXELWIDTH*PLTU::NCOL);
	Name = TString::Format("GlobalY_Ch%i",Channel);
	hMapGloY[Channel] = new TH1F(Name, Name, PLTU::NROW*2/3, -4.9*PLTU::PIXELHEIGHT*PLTU::NROW, 5*PLTU::PIXELHEIGHT*PLTU::NROW);
	Name = TString::Format("GlobalRot_Ch%i",Channel);
	hMapGloRot[Channel] = new TH2F(Name, Name, PLTU::NCOL, -4.9*PLTU::PIXELWIDTH*PLTU::NCOL, 5*PLTU::PIXELWIDTH*PLTU::NCOL, PLTU::NROW*2/3, -0.5*PLTU::PIXELHEIGHT*PLTU::NROW, 0.525*PLTU::PIXELHEIGHT*PLTU::NROW);
      }

      PLTPlane* Plane[3] = {0x0, 0x0, 0x0};
      for (size_t ip = 0; ip != NPlanes; ++ip) {
        Plane[ Telescope->Plane(ip)->ROC() ] = Telescope->Plane(ip);
      }

      PLTTrack* Track_3hit = new PLTTrack();

      Track_3hit->AddCluster(Plane[0]->Cluster(0));
      Track_3hit->AddCluster(Plane[1]->Cluster(0));
      Track_3hit->AddCluster(Plane[2]->Cluster(0));
      Track_3hit->MakeTrack(Alignment);

      //std::vector<float> bsT;
      //Alignment.GtoTXYZ(bsT,bsGX,bsGY,bsGZ,Plane[0]->Channel(),Plane[0]->ROC());
      //
      //float deltaX = Track_3hit->fTOX+Track_3hit->fTVX/Track_3hit->fTVZ*(bsT[2]-Track_3hit->fTOZ)-bsT[0];
      //float deltaY = Track_3hit->fTOY+Track_3hit->fTVY/Track_3hit->fTVZ*(bsT[2]-Track_3hit->fTOZ)-bsT[1];
      //
      //hMapGloX[Channel]->Fill(deltaX);
      //hMapGloY[Channel]->Fill(deltaY);
      //hMapGloRot[Channel]->Fill(deltaX,Track_3hit->fTOY);

      float deltaX = bsGX+Track_3hit->fGVX/Track_3hit->fGVZ*(Track_3hit->fGOZ-bsGZ)-Track_3hit->fGOX;
      float deltaY = bsGY+Track_3hit->fGVY/Track_3hit->fGVZ*(Track_3hit->fGOZ-bsGZ)-Track_3hit->fGOY;

      hMapGloX[Channel]->Fill(deltaX);
      hMapGloY[Channel]->Fill(deltaY);
      hMapGloRot[Channel]->Fill(deltaX,Track_3hit->fGOY);
    }
  }

  //Adjust alignment for the global displacements
  for (std::map<int, TH1F*>::iterator it = hMapTelXROC1.begin(); it != hMapTelXROC1.end(); ++it) {
   
    int const id = it->first;
    int Channel = id;

    TLatex *lat = new TLatex();
    lat->SetNDC();
    lat->SetTextSize(0.03);
    lat->SetTextColor(kRed+2);

    float originalGX = Alignment.GX(Channel,0);
    float originalGY = Alignment.GY(Channel,0);
    float originalGR = Alignment.LR(Channel,0);

    Alignment.AddToGX(Channel,hMapGloX[Channel]->GetMean());
    Alignment.AddToGY(Channel,hMapGloY[Channel]->GetMean());
    Alignment.AddToLR(Channel,0,-1.0*(acos(hMapGloRot[Channel]->GetCorrelationFactor())-3.141593/2.0));
    Alignment.AddToLR(Channel,1,-1.0*(acos(hMapGloRot[Channel]->GetCorrelationFactor())-3.141593/2.0));
    Alignment.AddToLR(Channel,2,-1.0*(acos(hMapGloRot[Channel]->GetCorrelationFactor())-3.141593/2.0));

    Can1.Clear();
    Name = TString::Format("Global Alignment in X for Ch%i", Channel);
    hMapGloX[id]->SetTitle(Name);
    hMapGloX[id]->GetXaxis()->SetTitle("deviation in X (cm)");
    hMapGloX[id]->SetFillColor(42);
    hMapGloX[id]->DrawNormalized();
    lat->DrawLatex(0.55,0.85,TString::Format("Avg. dev. = %.3f cm",hMapGloX[id]->GetMean()));
    lat->DrawLatex(0.55,0.81,TString::Format("Original X = %.3f cm",originalGX));
    lat->DrawLatex(0.55,0.77,TString::Format("New X = %.3f cm",Alignment.GX(id,0)));
    Name = TString::Format("plots/GlobalXalignment_Ch%i", Channel);
    Can1.SaveAs(Name + ".gif");
    Can1.SaveAs(Name + ".pdf");

    Can1.Clear();
    Name = TString::Format("Global Alignment in Y for Ch%i", Channel);
    hMapGloY[id]->SetTitle(Name);
    hMapGloY[id]->GetXaxis()->SetTitle("deviation in Y (cm)");
    hMapGloY[id]->SetFillColor(42);
    hMapGloY[id]->DrawNormalized();
    lat->DrawLatex(0.55,0.85,TString::Format("Avg. dev. = %.3f cm",hMapGloY[id]->GetMean()));
    lat->DrawLatex(0.55,0.81,TString::Format("Original Y = %.3f cm",originalGY));
    lat->DrawLatex(0.55,0.77,TString::Format("New Y = %.3f cm",Alignment.GY(id,0)));
    Name = TString::Format("plots/GlobalYalignment_Ch%i", Channel);
    Can1.SaveAs(Name + ".gif");
    Can1.SaveAs(Name + ".pdf");

    lat->SetTextColor(kBlack);
    Can1.Clear();
    Name = TString::Format("Global Rotation for Ch%i", Channel);
    hMapGloRot[id]->SetTitle(Name);
    hMapGloRot[id]->GetXaxis()->SetTitle("deviation in X (cm)");
    hMapGloRot[id]->GetYaxis()->SetTitleOffset(1.40);
    hMapGloRot[id]->GetYaxis()->SetTitle("Y position (cm)");
    hMapGloRot[id]->DrawNormalized("col");
    lat->DrawLatex(0.55,0.85,TString::Format("Correlation = %.3f",hMapGloRot[id]->GetCorrelationFactor()));
    lat->DrawLatex(0.55,0.81,TString::Format("Original Angle = %.3f",originalGR));
    lat->DrawLatex(0.55,0.77,TString::Format("New Angle = %.3f",Alignment.LR(id,0)));
    Name = TString::Format("plots/GlobalRotation_Ch%i", Channel);
    Can1.SaveAs(Name + ".gif");
    Can1.SaveAs(Name + ".pdf");
  }

  printf("\nAnalyzed %d good events of %d total events.\n\n",NEvents,NEventsTot);
  Alignment.WriteAlignmentFile("alignment_output.dat");

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat] [GainCal.dat] [AlignmentFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::string const AlignmentFileName = argv[3];

  CheckAlignment(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
