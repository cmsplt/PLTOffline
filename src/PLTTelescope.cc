#include "PLTTelescope.h"


PLTTelescope::PLTTelescope ()
{
  // Con me
}


PLTTelescope::~PLTTelescope ()
{
  // Telescopes own Tracks in them.
  for (size_t itrack = 0; itrack != fTracks.size(); ++itrack) {
    delete fTracks[itrack];
  }

  // Byebye
}


void PLTTelescope::AddPlane (PLTPlane* Plane)
{
  // Add a plane
  fPlanes.push_back(Plane);
  fChannel = Plane->Channel();
  return;
}


int PLTTelescope::Channel ()
{
  // Get the channel
  return fChannel;
}


PLTPlane* PLTTelescope::Plane(size_t i)
{
  // Get a specific plane
  return fPlanes[i];
}


void PLTTelescope::DrawTracksAndHits (std::string const Name)
{
  int const NH = NHits();
  int const NC = NClusters();
  int const NT = NTracks();

  float X[NH];
  float Y[NH];
  float Z[NH];

  float CX[NC];
  float CY[NC];
  float CZ[NC];

  TLine Line[3][NT];

  TH2F* HistCharge[3];
  for (int i = 0; i != 3; ++i) {
    TString Name;
    Name.Form("ChargeMap_Ch%i_ROC%i", Channel(), i);
    HistCharge[i] = new TH2F(Name, Name, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL+1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
    HistCharge[i]->GetZaxis()->SetRangeUser(0, 50000);
  }

  TH2F* HistChargeUnclustered[3];
  for (int i = 0; i != 3; ++i) {
    TString Name;
    Name.Form("ChargeMapUnclustered_Ch%i_ROC%i", Channel(), i);
    HistChargeUnclustered[i] = new TH2F(Name, Name, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL+1, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
    HistChargeUnclustered[i]->GetZaxis()->SetRangeUser(0, 50000);
  }



  int j = 0;
  for (size_t ip = 0; ip != NPlanes(); ++ip) {
    PLTPlane* P = Plane(ip);
    for (size_t ih = 0; ih != P->NHits(); ++ih) {
      PLTHit* H = P->Hit(ih);
      X[j] = H->TX();
      Y[j] = H->TY();
      Z[j] = H->TZ();
      ++j;

      HistCharge[ip]->SetBinContent(H->Column() + 1 - PLTU::FIRSTCOL, H->Row() + 1 - PLTU::FIRSTROW, H->Charge());
    }
    for (size_t ih = 0; ih != P->NUnclusteredHits(); ++ih) {
      PLTHit* H = P->UnclusteredHit(ih);
      HistChargeUnclustered[ip]->SetBinContent(H->Column() + 1 - PLTU::FIRSTCOL, H->Row() + 1 - PLTU::FIRSTROW, H->Charge());
    }



  }
  int jc = 0;
  for (size_t ip = 0; ip != NPlanes(); ++ip) {
    PLTPlane* P = Plane(ip);
    for (size_t ic = 0; ic != P->NClusters(); ++ic) {
      PLTCluster* C = P->Cluster(ic);
      CX[jc] = C->TX();
      CY[jc] = C->TY();
      CZ[jc] = C->TZ();
      ++jc;
    }
  }

  std::vector<PLTHit*> UsedHits;

  for (int i = 0; i != NT; ++i) {
    PLTTrack* T = fTracks[i];

    // XZ
    Line[0][i].SetX1(0);
    Line[0][i].SetX2(7.5);
    Line[0][i].SetY1(T->TX(0));
    Line[0][i].SetY2(T->TX(7.5));
    Line[0][i].SetLineColor(i+1);

    // YZ
    Line[1][i].SetX1(0);
    Line[1][i].SetX2(7.5);
    Line[1][i].SetY1(T->TY(0));
    Line[1][i].SetY2(T->TY(7.5));
    Line[1][i].SetLineColor(i+1);

    // XY
    Line[2][i].SetX1(T->TX(0));
    Line[2][i].SetX2(T->TX(7.5));
    Line[2][i].SetY1(T->TY(0));
    Line[2][i].SetY2(T->TY(7.5));
    Line[2][i].SetLineColor(i+1);

    //printf("XY 0 7: %9.3f %9.3f   %9.3f %9.3f\n", T->TX(0), T->TY(0), T->TX(7), T->TY(7));
  }

  //for (int i = 0; i != 3; ++i) {
  //  for (int j = 0; j != NT; ++j) {
  //    Line[i][j].SetLineColor(4);
  //  }
  //}

  TCanvas C("TelescopeTrack", "TelescopeTrack", 800, 800);;
  C.Divide(3, 3);

  C.cd(1);
  TGraph gXZ(NC, CZ, CX);
  gXZ.SetTitle("");
  gXZ.GetXaxis()->SetTitle("Z (cm)");
  gXZ.GetYaxis()->SetTitle("X (cm)");
  gXZ.GetXaxis()->SetTitleSize(0.06);
  gXZ.GetYaxis()->SetTitleSize(0.08);
  gXZ.GetXaxis()->SetTitleOffset(0.7);
  gXZ.GetYaxis()->SetTitleOffset(0.5);
  gXZ.SetMarkerColor(40);
  gXZ.GetXaxis()->SetLimits(-0.5, 8);
  gXZ.SetMinimum(-0.3);
  gXZ.SetMaximum( 0.3);
  if (NC) {
    gXZ.Draw("A*");
  }
  for (int i = 0; i != NT; ++i) {
    Line[0][i].Draw();
  }

  C.cd(4);
  TGraph gYZ(NC, CZ, CY);
  gYZ.SetTitle("");
  gYZ.GetXaxis()->SetTitle("Z (cm)");
  gYZ.GetYaxis()->SetTitle("Y (cm)");
  gYZ.GetXaxis()->SetTitleSize(0.06);
  gYZ.GetYaxis()->SetTitleSize(0.08);
  gYZ.GetXaxis()->SetTitleOffset(0.7);
  gYZ.GetYaxis()->SetTitleOffset(0.5);
  gYZ.SetMarkerColor(40);
  gYZ.GetXaxis()->SetLimits(-0.5, 8);
  gYZ.SetMinimum(-0.3);
  gYZ.SetMaximum( 0.6);
  if (NC) {
    gYZ.Draw("A*");
  }
  for (int i = 0; i != NT; ++i) {
    Line[1][i].Draw();
  }

  //TVirtualPad* Pad = C.cd(3);
  //Pad->DrawFrame(-30, -30, 30, 30);
  C.cd(7);
  TGraph gXY(NC, CX, CY);
  gXY.SetTitle("");
  gXY.GetXaxis()->SetTitle("X (cm)");
  gXY.GetYaxis()->SetTitle("Y (cm)");
  gXY.GetXaxis()->SetTitleSize(0.06);
  gXY.GetYaxis()->SetTitleSize(0.08);
  gXY.GetXaxis()->SetTitleOffset(0.7);
  gXY.GetYaxis()->SetTitleOffset(0.5);
  gXY.SetMarkerColor(40);
  gXY.GetXaxis()->SetLimits(-0.3, 0.3);
  gXY.SetMinimum(-0.3);
  gXY.SetMaximum( 0.6);
  if (NC) {
    gXY.Draw("A*");
  }
  for (int i = 0; i != NT; ++i) {
    Line[2][i].Draw();
  }

  C.cd(2);
  HistCharge[0]->Draw("colz");
  C.cd(5);
  HistCharge[1]->Draw("colz");
  C.cd(8);
  HistCharge[2]->Draw("colz");

  C.cd(3);
  HistChargeUnclustered[0]->Draw("colz");
  C.cd(6);
  HistChargeUnclustered[1]->Draw("colz");
  C.cd(9);
  HistChargeUnclustered[2]->Draw("colz");

  C.SaveAs(Name.c_str());

  for (int i = 0; i != 3; ++i) {
    delete HistCharge[i];
    delete HistChargeUnclustered[i];
  }

  return;
}




void PLTTelescope::Draw2D (int const np, TString const Name)
{
  // This is to draw a telescope.  I'll get back to this later on
  std::vector<TH2F*> h;
  TCanvas c("TelescopeHits", "Telescope Hits", 400, 900);
  c.Divide(1,3);
  for (size_t i = 0; i != fPlanes.size(); ++i) {
    c.cd(fPlanes[i]->ROC());
    h.push_back( fPlanes[i]->DrawHits2D() );
    //h[i]->SetMaximum(70000);
    //h[i]->SetMinimum(0);
    //h[i]->Draw("colz");
    h[i]->Draw("box");
  }

  c.SaveAs(Name);
  for (size_t i = 0; i != fPlanes.size(); ++i) {
    delete h[i];
  }

  return;
}



size_t PLTTelescope::NPlanes ()
{
  // Number of planes in this telescope with at least one hit
  return fPlanes.size();
}


size_t PLTTelescope::NHits ()
{
  // Get the total number of hits in all planes in this telescope
  size_t nhits = 0;
  for (std::vector<PLTPlane*>::iterator it = fPlanes.begin(); it != fPlanes.end(); ++it) {
    nhits += (*it)->NHits();
  }

  return nhits;
}


size_t PLTTelescope::NClusters ()
{
  // Get the total number of clusters in all planes in this telescope
  size_t nclusters = 0;
  for (std::vector<PLTPlane*>::iterator it = fPlanes.begin(); it != fPlanes.end(); ++it) {
    nclusters += (*it)->NClusters();
  }

  return nclusters;
}



size_t PLTTelescope::NTracks ()
{
  return fTracks.size();
}

PLTTrack* PLTTelescope::Track (size_t i)
{
  return fTracks[i];
}

int PLTTelescope::HitPlaneBits ()
{
  // This function return a binary representation of hit planes
  int HitPlanes = 0x0;

  for (std::vector<PLTPlane*>::iterator it = fPlanes.begin(); it != fPlanes.end(); ++it) {
    if ((*it)->NHits() > 0) {
      HitPlanes |= (0x1 << (*it)->ROC());
    }
  }

  return HitPlanes;
}



int PLTTelescope::NHitPlanes ()
{
  // This function return a binary representation of hit planes
  int HitPlanes = 0;

  for (std::vector<PLTPlane*>::iterator it = fPlanes.begin(); it != fPlanes.end(); ++it) {
    if ((*it)->NHits() > 0) {
      ++HitPlanes;
    }
  }

  return HitPlanes;
}



void PLTTelescope::AddTrack (PLTTrack* T)
{
  fTracks.push_back(T);
  return;
}


void PLTTelescope::FillAndOrderTelescope ()
{
  // This functino takes forces the size of a telescope to be 3 ROCs
  // It then orders the ROCs which exist and makes a new plane for missing ones =)

  std::vector<PLTPlane*> Ordered(3, (PLTPlane*) 0x0);

  for (size_t i = 0; i != fPlanes.size(); ++i) {
    Ordered[ fPlanes[i]->ROC() ] = fPlanes[i];
  }

  fPlanes = Ordered;
  return;
}


