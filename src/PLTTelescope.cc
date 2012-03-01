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
  int const NT = NTracks();

  float X[NH];
  float Y[NH];
  float Z[NH];

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

  for (int i = 0; i != NT; ++i) {
    PLTTrack* T = fTracks[i];

    // XZ
    Line[0][i].SetX1(0);
    Line[0][i].SetX2(5);
    Line[0][i].SetY1(T->TX(0));
    Line[0][i].SetY2(T->TX(5));

    // YZ
    Line[1][i].SetX1(0);
    Line[1][i].SetX2(5);
    Line[1][i].SetY1(T->TY(0));
    Line[1][i].SetY2(T->TY(5));

    // XY
    Line[2][i].SetX1(T->TX(0));
    Line[2][i].SetX2(T->TX(5));
    Line[2][i].SetY1(T->TY(0));
    Line[2][i].SetY2(T->TY(5));

    printf("XY 0 5: %9.3f %9.3f   %9.3f %9.3f\n", T->TX(0), T->TY(0), T->TX(5), T->TY(5));
  }

  for (int i = 0; i != NT; ++i) {
    for (int j = 0; j != 3; ++j) {
      Line[i][j].SetLineColor(4);
    }
  }

  TCanvas C("TelescopeTrack", "TelescopeTrack", 400, 600);;
  C.Divide(1, 3);

  C.cd(1);
  TGraph gXZ(NH, Z, X);
  gXZ.SetTitle("");
  gXZ.GetXaxis()->SetTitle("Z (cm)");
  gXZ.GetYaxis()->SetTitle("X (cm)");
  gXZ.GetXaxis()->SetTitleSize(0.08);
  gXZ.GetYaxis()->SetTitleSize(0.08);
  gXZ.GetXaxis()->SetTitleOffset(0.7);
  gXZ.GetYaxis()->SetTitleOffset(0.5);
  gXZ.SetMarkerColor(2);
  gXZ.GetXaxis()->SetLimits(-0.5, 5.5);
  gXZ.SetMinimum(-0.3);
  gXZ.SetMaximum( 0.3);
  gXZ.Draw("A*");
  for (int i = 0; i != NT; ++i) {
    Line[0][i].Draw();
  }

  C.cd(2);
  TGraph gYZ(NH, Z, Y);
  gYZ.SetTitle("");
  gYZ.GetXaxis()->SetTitle("Z (cm)");
  gYZ.GetYaxis()->SetTitle("Y (cm)");
  gYZ.GetXaxis()->SetTitleSize(0.08);
  gYZ.GetYaxis()->SetTitleSize(0.08);
  gYZ.GetXaxis()->SetTitleOffset(0.7);
  gYZ.GetYaxis()->SetTitleOffset(0.5);
  gYZ.SetMarkerColor(2);
  gYZ.GetXaxis()->SetLimits(-0.5, 5.5);
  gYZ.SetMinimum(-0.3);
  gYZ.SetMaximum( 0.3);
  gYZ.Draw("A*");
  for (int i = 0; i != NT; ++i) {
    Line[1][i].Draw();
  }

  //TVirtualPad* Pad = C.cd(3);
  //Pad->DrawFrame(-30, -30, 30, 30);
  C.cd(3);
  TGraph gXY(NH, X, Y);
  gXY.SetTitle("");
  gXY.GetXaxis()->SetTitle("X (cm)");
  gXY.GetYaxis()->SetTitle("Y (cm)");
  gXY.GetXaxis()->SetTitleSize(0.08);
  gXY.GetYaxis()->SetTitleSize(0.08);
  gXY.GetXaxis()->SetTitleOffset(0.7);
  gXY.GetYaxis()->SetTitleOffset(0.5);
  gXY.SetMarkerColor(2);
  gXY.GetXaxis()->SetLimits(-0.3, 0.3);
  gXY.SetMinimum(-0.3);
  gXY.SetMaximum( 0.3);
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


void PLTTelescope::TrackFinderFromBeamCrossing ()
{
  // Find tracks in this telescope given some range for the beam crossing
  // which is assumed to be at 0,0,0.
  // Draw a cone from hit in plane 0 backwards looking for hits

  // Check that tracks haven't already been filled
  if (fTracks.size() != 0) {
    std::cerr << "ERROR: It looks like tracks have already been filled here: PLTTelescope::TrackFinderFromBeamCrossing()" << std::endl;
    return;
  }

  if (this->HitPlaneBits() != 0x7) {
    return;
  }

  std::vector<int> UsedHits;

  for (size_t iCluster = 0; iCluster != fPlanes[0]->NClusters(); ++iCluster) {
    std::pair<float, float> const XY = fPlanes[0]->Cluster(iCluster)->GCenterOfMass();
    float const Z = fPlanes[0]->Cluster(iCluster)->GZ();

    float const SlopeX = XY.first  / (Z - 0.0);
    float const SlopeY = XY.second / (Z - 0.0);

    for (size_t iPlane = 1; iPlane != fPlanes.size(); ++iPlane) {
      for (int iClusterP = 0; iClusterP != fPlanes[iPlane]->NClusters(); ++iClusterP) {
        // check used list
        //if (fPlanes[iPlane]->Cluster(iClusterP).ResidualXY() < 0.5) {
          // think about keeping this plane
          // add to used list
        //}
      }
    }
  }



  for (size_t iPlane = 0; iPlane != fPlanes.size(); ++iPlane) {
  }

  return;
}
