////////////////////////////////////////////////////////////////////
//
// Krishna Thapa <kthapa@cern.ch>
//
// Created on: Tue July 05
//
// Combined fit with SlopeY_Model, SlopeX_Model, and extra PDF (frac3)
// 5 Sigma cuts to SlopeY and SlopeX from VdM means
////////////////////////////////////////////////////////////////////
{
  gSystem->Load("libMinuit") ; // ROOT4 only
  gSystem->Load("libRooFit") ;
  using namespace RooFit ;

  cout << "SlopeY and SlopeX combined fit" << endl;
  cout << "------------------------------------" << endl;

  //Define variables
  RooRealVar num("num","num",0,500000000);
  RooRealVar tr("tr","tr",0,20);
  RooRealVar sy("sy","sy",-0.1,0.1);
  RooRealVar sx("sx","sx",-0.1,0.1);
  gStyle->SetOptFit(01111);

  // read dataset from file


  //Toy data--testing testing
  //  RooDataSet* data = RooDataSet::read("./toy.txt",
  //                                      RooArgList(num,tr,sx,sy));//~2.89


  ///Fill 4958
  //  RooDataSet* data = RooDataSet::read("./xy58.txt",
  //                                      RooArgList(num,tr,sx,sy));//~2.89

  //////////fill 4965//////////
  //  RooDataSet* data = RooDataSet::read("./xy4965_11.txt",
  //                                      RooArgList(num,tr,sx,sy));//3.29678
  //  RooDataSet* data = RooDataSet::read("./xy4965_67.txt",
  //                                      RooArgList(num,tr,sx,sy));//2.8381
  //  RooDataSet* data = RooDataSet::read("./xy4965_77.txt",
  //                                      RooArgList(num,tr,sx,sy));//2.74195



  /////////Mu Scan Fill 5005/////////
  //  RooDataSet* data = RooDataSet::read("./xyMu4.txt",
  //                                      RooArgList(num,sx,sy));//2.39699
  //  RooDataSet* data = RooDataSet::read("./xyMu5.txt",
  //                                      RooArgList(num,sx,sy));//3.02361
  //  RooDataSet* data = RooDataSet::read("./xyMu6.txt",
  //                                      RooArgList(num,sx,sy));//3.49734
  //  RooDataSet* data = RooDataSet::read("./xyMu7.txt",
  //                                      RooArgList(num,sx,sy));//3.69346
  //  RooDataSet* data = RooDataSet::read("./xyMu8.txt",
  //                                      RooArgList(num,sx,sy));//3.55534
  //  RooDataSet* data = RooDataSet::read("./xyMu9.txt",
  //                                      RooArgList(num,sx,sy));//3.12099
  //  RooDataSet* data = RooDataSet::read("./xyMu10.txt",
  //                                      RooArgList(num,sx,sy));//2.51917

  //5013
  //  RooDataSet* data = RooDataSet::read("./xy5013_1376.txt",
  //                                      RooArgList(num,tr,sx,sy));//3.867
  //  RooDataSet* data = RooDataSet::read("./xy5013_1544.txt",
  //                                      RooArgList(num,tr,sx,sy));//3.43898



  //5020
  //  RooDataSet* data = RooDataSet::read("./xy5020_40.txt",
  //                                      RooArgList(num,tr,sx,sy));// 4.01162
  //  RooDataSet* data = RooDataSet::read("./xy5020_60.txt",
  //                                      RooArgList(num,tr,sx,sy));//3.94995

  //5038
  //  RooDataSet* data = RooDataSet::read("./xy5038_20.txt",
  //                                      RooArgList(num,tr,sx,sy));//4.17174
  //  RooDataSet* data = RooDataSet::read("./xy5038_60.txt",
  //                                      RooArgList(num,tr,sx,sy));//4.00792
  //  RooDataSet* data = RooDataSet::read("./xy5038_120.txt",
  //                                      RooArgList(num,tr,sx,sy));//3.9075


  //5056
  //  RooDataSet* data = RooDataSet::read("./xy5056_5.txt",
  //                                      RooArgList(num,tr,sx,sy));//4.36
  //  RooDataSet* data = RooDataSet::read("./xy5056_20.txt",
  //                                      RooArgList(num,tr,sx,sy));//4.28

  //Test with VdM
  //  RooDataSet* data = RooDataSet::read("./xyVdM0.txt",
  //                                      RooArgList(num,sx,sy)); all Points
  //  RooDataSet* data = RooDataSet::read("./xyVdMColl.txt",
  //                                      RooArgList(num,sx,sy));// colliding bunches
  //  RooDataSet* data = RooDataSet::read("./xyVdM_N_Coll.txt",
  //                                      RooArgList(num,sx,sy));// non-colliding bunches

  //-----------------------------SlopeY---------------------------

  //SlopeY signal
  RooRealVar meanSsy("meanSsy","Signal Slope Y-mean",0.0269851);//,0.02,0.03);
  RooRealVar sigmaSsy("sigmaSsy","Signal Y-sigma",0.00115238);//,0.001,0.0013);
  RooGaussian Ssy("Ssy","Sy signal PDF",sy,meanSsy,sigmaSsy);

  //SlopeY bkgd
  //Bifurcated gauss
  RooRealVar bfmean("bfmean","bif mean",0.0288341);
  RooRealVar bfsigmaL("bfsigmaL","bif sigmaLeft",0.014282);
  RooRealVar bfsigmaR("bfsigmaR","bif sigmaRight",0.0155939);
  RooBifurGauss bfg("bfg","bif gauss bkgd pdf",sy,bfmean,bfsigmaL,bfsigmaR);

  //Anoter gauus
  RooRealVar BpSymean("BpSymean", "BpSymean",0.0262682);//,0.02,0.03);
  RooRealVar BpSysig("BpSysig", "BpSysig",0.00344703);//,0.003,0.004);
  RooGaussian BpSy("BpSy","BpSy bgd PDF",sy,BpSymean,BpSysig);


  //Float bifurcated gaussian
  RooRealVar bfPmean("bfPmean","bif meanP",0.0,-0.1,.1);
  RooRealVar bfPsigmaL("bfPsigmaL","bif sigmaLeftP",0.0101953,0.0,1.0);
  RooRealVar bfPsigmaR("bfPsigmaR","bif sigmaRightP",0.0141026,0.0,1.0);
  RooBifurGauss bfgP("bfgP","bif gauss bkgd pdf",sy,bfPmean,bfPsigmaL,bfPsigmaR);
  //  RooGaussian bfgP("bfgP","bfgP bgd PDF",sy,bfPmean,bfPsigmaL);//regular gaussian


  //-----------------------------FractionsY---------------------------
  // via VdM SlopeY Fit
  RooRealVar frac1y("frac1y","outlier fraction Y",0.799983);//,0.,1.0);
  RooRealVar frac2y("frac2y","tail fraction Y",0.372581);//,0.,1.0);

  RooRealVar frac3("frac3","new bif. gauss fraction Y",0.5,0.0,1.0);


  //----------------------SlopeY Model---------------------------
  // SlopeY background
  RooAddPdf bmodelY ("bmodelY","ModelY: BifG+G",
                     RooArgList(bfg,BpSy),frac2y);

  //VdM Model for SlopeY
  RooAddPdf VmodelY ("VmodelY","ModelY:S+B",
                   RooArgList(Ssy,bmodelY),frac1y);

  //SlopeY model == VdmY + frac*Bif. Gauss
  RooAddPdf f_modelY ("f_modelY","ModelY:S+B",
                   RooArgList(VmodelY,bfgP),frac3);

  //-----------------------------SlopeX---------------------------



  //SlopeX signal
  RooRealVar meanSsx("meanSsx","Signal Slope X-mean",0.000285281);
  RooRealVar sigmaSsx("sigmaSsx","Signal X-sigma", 0.0155009);
  RooGaussian Ssx("Ssx","Sx signal PDF",sx,meanSsx,sigmaSsx);

  //SlopeX bkgd

  //Anoter gauus
  RooRealVar G1mean("G1mean", "G1mean",-7.66321e-06);
  RooRealVar G1sigma("G1sigma", "G1sigma",0.00673643);
  RooGaussian G1("G1","G1 bgd PDF",sx,G1mean,G1sigma);

  RooRealVar G2mean("G2mean", "G2mean",0.000171416);//,-0.002,0.0002);
  RooRealVar G2sigma("G2sigma", "G2sigma",0.00298365);//,0.0001,0.004);
  RooGaussian G2("G2","G2 bgd PDF",sx,G2mean,G2sigma);

  //Float regular gaussian
  RooRealVar G3mean("G3mean", "G3mean",0.0,-0.01,0.01);
  RooRealVar G3sigma("G3sigma", "G3sigma",0.01,0.0,0.5);
  RooGaussian G3("G3","G3 bgd PDF",sx,G3mean,G3sigma);


  //fixed to best fit
  //SlopeX fit
  RooRealVar frac1x("frac1x","outlier fraction X",0.123612);
  RooRealVar frac2x("frac2x","tail fraction X", 0.279616);
  //  RooRealVar frac3("frac3","new bif. gauss fraction",0.9,0.0,1.0);

  //----------------------SlopeX Model---------------------------
  // background
   RooAddPdf bmodelX ("bmodelX","ModelX: G1+G2",
                     RooArgList(G1,G2),frac2x);

  //VdM Model
  RooAddPdf VmodelX ("VmodelX","ModelX:G1+G2+Ssx",
                   RooArgList(Ssx,bmodelX),frac1x);

  //SlopeX model == VdmX + frac* Gauss
  RooAddPdf f_modelX ("f_modelX","ModelX:S+B",
                     RooArgList(VmodelX,G3),frac3);



  //Final Model
  RooProdPdf f_modelXY ("f_modelXY","ModelXY:S+B",
                       RooArgList(f_modelY,f_modelX));

  //VdM Model
  RooFitResult* r = f_modelXY.fitTo(*data,Save());
  //  RooFitResult* r = f_modelXY.fitTo(*data,Extended(kTRUE));



  //----------------------Get Results---------------------------


  Int_t nby = 100;

    RooPlot* xframe1 =  new RooPlot("p.d.f. with data:Sy","plot",sy,-0.03,0.07,nby);
  //RooPlot* xframe1 =  new RooPlot("p.d.f. with data:Sy","plot",sy,0.02,0.04,nby);
  RooPlot* xframe2 =  new RooPlot("p.d.f. with data:Sx","plot",sx,-0.07,0.07,nby);

  data->plotOn(xframe1);
  f_modelY->plotOn(xframe1,DataError(RooAbsData::SumW2));
  Double_t chisy = xframe1->chiSquare();

  // Construct a histogram with the residuals of the data w.r.t. the curve
  RooHist* syh = xframe1->residHist();


  //
  // pull(N_sig) = \frac{Nsig_fit - Nsig_true}{SigmaN_fit}
  //

  // Construct a histogram with the pulls of the data w.r.t the curve
  RooHist* hypull = xframe1->pullHist();


  data->plotOn(xframe2);
  f_modelX->plotOn(xframe2);
  f_modelX->plotOn(xframe2,DataError(RooAbsData::SumW2));

  Double_t chisx = xframe2->chiSquare();
  // Construct a histogram with the residuals of the data w.r.t. the curve
  RooHist* sxh = xframe2->residHist();


  // Construct a histogram with the pulls of the data w.r.t the curve
  RooHist* hxpull = xframe2->pullHist();


  // Create a new frame to draw the residual+pull distributions and add the distributions to the frame

  RooPlot* frame10 =  new RooPlot("Residual Distribution:Sy","Residuals Distribution- Sy",sy,-0.03,0.07,nby);
  frame10->addPlotable(syh,"P");

  RooPlot* frame11 =  new RooPlot("Pull Distribution:Sy","Pull Distribution - Sy",sy,-0.03,0.07,nby);
  frame11->addPlotable(hypull,"P");


  RooPlot* frame20 =  new RooPlot("Residual Distribution:Sx","Residuals Distribution- Sx",sx,-0.07,0.07,nby);
  frame20->addPlotable(sxh,"P");

  RooPlot* frame21 =  new RooPlot("Pull Distribution:Sx","Pull Distribution- Sx",sx,-0.07,0.07,nby);
  frame21->addPlotable(hxpull,"P");

  TCanvas* c3 = new TCanvas("residpull","residpull",1200,700) ;
  c3->Divide(2,2);
  c3->cd(1);
  gPad->SetLeftMargin(0.15) ; frame10->GetYaxis()->SetTitleOffset(1.6) ;frame10->Draw() ;
  c3->cd(3);
  gPad->SetLeftMargin(0.15) ; frame11->GetYaxis()->SetTitleOffset(1.6) ;frame11->Draw() ;


  c3->cd(2);
  gPad->SetLeftMargin(0.15) ; frame20->GetYaxis()->SetTitleOffset(1.6) ;frame20->Draw();
  c3->cd(4);
  gPad->SetLeftMargin(0.15) ; frame21->GetYaxis()->SetTitleOffset(1.6) ;frame21->Draw();


  // SlopeY Model

  f_modelXY.plotOn(xframe1,RooFit::Components("VmodelY"),RooFit::LineColor(kGreen));
  f_modelXY.plotOn(xframe1,RooFit::Components("bfgP"),RooFit::LineColor(kMagenta));

  // SlopeX Model

  f_modelXY.plotOn(xframe2,RooFit::Components("VmodelX"),RooFit::LineColor(kGreen));
  f_modelXY.plotOn(xframe2,RooFit::Components("G3"),RooFit::LineColor(kMagenta));


  TCanvas* c2 = new TCanvas("c2","",600,900);
  c2->Divide(1,2);
  c2->cd(1);
  gPad->SetLogy();
  xframe1->SetXTitle("SlopeY");
  xframe1->SetTitle("SlopeY Distribution");
  xframe1->Draw();

  c2->cd(2);  gPad->SetLogy();
  xframe2->Draw();
  xframe2->SetXTitle("SlopeX");
  xframe2->SetTitle("SlopeX Distribution");

  //Summary printing: Basic info + final values of floating fit params
  r->Print();

  //Verbose printing: vasic infor+values of const. params, initial
  // and final values of flaoting params, global correlations

  std::cout <<"======Verbose======"  << "\n";
  r->Print("v");

  //Visualize correlation matrix

  gStyle->SetOptStat(0);
  gStyle->SetPalette(1);
  TH2* hcorr = r->correlationHist();

  // A c c e s s   f i t   r e s u l t   i n f o r m a t i o n
  // ---------------------------------------------------------

  // Access basic information
  cout << "EDM = " << r->edm() << endl ;
  cout << "-log(L) at minimum = " << r->minNll() << endl ;

  // Access list of final fit parameter values
  cout << "final value of floating parameters" << endl ;
  r->floatParsFinal().Print("s") ;

  // Extract covariance and correlation matrix as TMatrixDSym
  const TMatrixDSym& cor = r->correlationMatrix() ;
  const TMatrixDSym& cov = r->covarianceMatrix() ;

  // Print correlation, covariance matrix
  cout << "correlation matrix" << endl ;
  cor.Print() ;
  cout << "covariance matrix" << endl ;
  cov.Print() ;




  // ---------------------------------------------------------

  //Fraction 3
  Double_t fr = 100.*(1.- frac3->getVal());
  Double_t frhe = 100.*(frac3->getErrorHi());
  Double_t frle = 100.*(frac3->getErrorLo());
  std::cout << "f3               : " << fr <<  " + " << frhe << " - "
            << frle << std::endl;




  // ---------------------------------------------------------
  // Apply 5 Sigma Cut on each Variables
  // ---------------------------------------------------------


  int NTotal = data->numEntries();
  std::cout << "NTotal: " <<  NTotal << std::endl;


  TString cuty = "abs(sy-0.0269851)>0.0057619";
  RooDataSet* SyCut = (RooDataSet*) data.reduce(cuty.Data());

  TString cutx = "abs(sx-0.000171416)>0.01491825";
  RooDataSet* SxCut = (RooDataSet*) data.reduce(cutx.Data());

  TString cutxy = cuty + " && " + cutx;
  RooDataSet* SxSyCut = (RooDataSet*) data.reduce(cutxy.Data());

  // ---------------------------------------------------------
  // 5 Sigma cut Result
  std::cout << "Sy_out: " << SyCut->numEntries() << " %: "
            <<  100.0*(SyCut->numEntries())/NTotal  << std::endl;
  std::cout << "Sx_out: " << SxCut->numEntries() << " %: "
            <<  100.0*(SxCut->numEntries())/NTotal << std::endl;
  std::cout << "SxSy_out: " << SxSyCut->numEntries() << " %: "
            <<  100.0*(SxSyCut->numEntries())/NTotal <<std::endl;

  std::cout << "y_chi^2 = " << chisy << std::endl ;
  std::cout << "x_chi^2 = " << chisx << std::endl ;

}
