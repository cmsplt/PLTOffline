#include "PLTTelescope.h"


PLTTelescope::PLTTelescope ()
{
  // Con me
}


PLTTelescope::~PLTTelescope ()
{
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

  int j = 0;
  for (size_t ip = 0; ip != NPlanes(); ++ip) {
    PLTPlane* P = Plane(ip);
    for (size_t ih = 0; ih != P->NHits(); ++ih) {
      PLTHit* H = P->Hit(ih);
      X[j] = H->TX();
      Y[j] = H->TY();
      Z[j] = H->TZ();
      ++j;
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

  TCanvas C("TelescopeTrack", "TelescopeTrack", 400, 600);;
  C.Divide(1, 3);

  C.cd(1);
  TGraph gXZ(NC, CZ, CX);
  gXZ.SetTitle("");
  gXZ.GetXaxis()->SetTitle("Z (cm)");
  gXZ.GetYaxis()->SetTitle("X (cm)");
  gXZ.GetXaxis()->SetTitleSize(0.08);
  gXZ.GetYaxis()->SetTitleSize(0.08);
  gXZ.GetXaxis()->SetTitleOffset(0.7);
  gXZ.GetYaxis()->SetTitleOffset(0.5);
  gXZ.SetMarkerColor(40);
  gXZ.GetXaxis()->SetLimits(-0.5, 8);
  gXZ.SetMinimum(-0.3);
  gXZ.SetMaximum( 0.3);
  gXZ.Draw("A*");
  for (int i = 0; i != NT; ++i) {
    Line[0][i].Draw();
  }

  C.cd(2);
  TGraph gYZ(NC, CZ, CY);
  gYZ.SetTitle("");
  gYZ.GetXaxis()->SetTitle("Z (cm)");
  gYZ.GetYaxis()->SetTitle("Y (cm)");
  gYZ.GetXaxis()->SetTitleSize(0.08);
  gYZ.GetYaxis()->SetTitleSize(0.08);
  gYZ.GetXaxis()->SetTitleOffset(0.7);
  gYZ.GetYaxis()->SetTitleOffset(0.5);
  gYZ.SetMarkerColor(40);
  gYZ.GetXaxis()->SetLimits(-0.5, 8);
  gYZ.SetMinimum(-0.3);
  gYZ.SetMaximum( 0.6);
  gYZ.Draw("A*");
  for (int i = 0; i != NT; ++i) {
    Line[1][i].Draw();
  }

  //TVirtualPad* Pad = C.cd(3);
  //Pad->DrawFrame(-30, -30, 30, 30);
  C.cd(3);
  TGraph gXY(NC, CX, CY);
  gXY.SetTitle("");
  gXY.GetXaxis()->SetTitle("X (cm)");
  gXY.GetYaxis()->SetTitle("Y (cm)");
  gXY.GetXaxis()->SetTitleSize(0.08);
  gXY.GetYaxis()->SetTitleSize(0.08);
  gXY.GetXaxis()->SetTitleOffset(0.7);
  gXY.GetYaxis()->SetTitleOffset(0.5);
  gXY.SetMarkerColor(40);
  gXY.GetXaxis()->SetLimits(-0.3, 0.3);
  gXY.SetMinimum(-0.3);
  gXY.SetMaximum( 0.6);
  gXY.Draw("A*");
  for (int i = 0; i != NT; ++i) {
    Line[2][i].Draw();
  }

  C.SaveAs(Name.c_str());

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


