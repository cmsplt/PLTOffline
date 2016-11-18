{
  gSystem->Load("libMinuit") ; // ROOT4 only
  gSystem->Load("libRooFit") ;
  using namespace RooFit ;

  cout << "SlopeY, SlopeX fit" << endl;
  cout << "------------------------------------" << endl;

  RooRealVar num("num","num",0,500000000);
  RooRealVar tr("tr","tr",0,20);
  RooRealVar sy("sy","sy",-0.1,0.1);
  RooRealVar sx("sx","sx",-0.1,0.1);
  gStyle->SetOptFit(01111);

  // read dataset from file


  //5038
  //  RooDataSet* data = RooDataSet::read("./xy5038_60.txt",
  //                                      RooArgList(num,tr,sx,sy));//4.00792
  //-----------------------------SlopeY---------------------------

  //SlopeY signal
  RooRealVar meanSsy("meanSsy","Signal Slope Y-mean",0.0269851);
  RooRealVar sigmaSsy("sigmaSsy","Signal Y-sigma",0.00115238);
  RooGaussian Ssy("Ssy","Sy signal PDF",sy,meanSsy,sigmaSsy);

  //SlopeY bkgd
  //Bifurcated gauss
  RooRealVar bfmean("bfmean","bif mean",0.0288341);
  RooRealVar bfsigmaL("bfsigmaL","bif sigmaLeft",0.014282);
  RooRealVar bfsigmaR("bfsigmaR","bif sigmaRight",0.0155939);
  RooBifurGauss bfg("bfg","bif gauss bkgd pdf",sy,bfmean,bfsigmaL,bfsigmaR);

  //Anoter gauus
  RooRealVar BpSymean("BpSymean", "BpSymean",0.0262682);
  RooRealVar BpSysig("BpSysig", "BpSysig",0.00344703);
  RooGaussian BpSy("BpSy","BpSy bgd PDF",sy,BpSymean,BpSysig);


  //Float bifurcated gaussian
  RooRealVar bfPmean("bfPmean","bif meanP", 0.0240438);//0.0,-0.1,.1);
  RooRealVar bfPsigmaL("bfPsigmaL","bif sigmaLeftP", 0.0178329);//0.0101953,0.0,1.0);
  RooRealVar bfPsigmaR("bfPsigmaR","bif sigmaRightP",0.019176);//0.0141026,0.0,1.0);
  RooBifurGauss bfgP("bfgP","bif gauss bkgd pdf",sy,bfPmean,bfPsigmaL,bfPsigmaR);
  //  RooGaussian bfgP("bfgP","bfgP bgd PDF",sy,bfPmean,bfPsigmaL);//regular gaussian


  //-----------------------------FractionsY---------------------------
  // via VdM SlopeY Fit
  RooRealVar frac1y("frac1y","outlier fraction Y",0.799983);
  RooRealVar frac2y("frac2y","tail fraction Y",0.372581);

  RooRealVar frac3("frac3","new bif. gauss fraction Y",.78,0.0,1.0);


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

  RooRealVar G2mean("G2mean", "G2mean",0.000171416);
  RooRealVar G2sigma("G2sigma", "G2sigma",0.00298365);
  RooGaussian G2("G2","G2 bgd PDF",sx,G2mean,G2sigma);

  //Float regular gaussian
  RooRealVar G3mean("G3mean", "G3mean",-0.00117066);//0.0,-0.01,0.01);
  RooRealVar G3sigma("G3sigma", "G3sigma",0.0178274);//0.01,0.0,0.5);
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

  RooAddPdf f_modelX ("f_modelX","ModelX:S+B",
                     RooArgList(VmodelX,G3),frac3);



  //Final Model
  RooProdPdf f_modelXY ("f_modelXY","ModelXY:S+B",
                       RooArgList(f_modelY,f_modelX));

  // Generate Monte Carlo Toys
  RooDataSet* data = f_modelXY.generate(RooArgSet(sx,sy),100000);
  RooFitResult* r = f_modelXY.fitTo(*data,Save());
  r->Print("v");

  
  RooDataSet* d4 = (RooDataSet*) data.reduce(RooArgSet(sx,sy),
   //  "abs(sx-0.000171416)>(0.00298365*5" || abs(sy-0.0269851)>(0.00115238*5)) ;
         "abs(sx-0.0000947)>(0.004163*5) || abs(sy-0.02694)>(0.001397*5)") ;
  Double_t fr = 100.*(1.- frac3->getVal());
  Double_t frhe = 100.*(frac3->getErrorHi());
  Double_t frle = 100.*(frac3->getErrorLo());
  std::cout << "f3               : " << fr <<  " + " << frhe << " - "
            << frle << std::endl; 

  std::cout << "Outlier Fraction : " << d4->numEntries()/100000. *100<< std::endl;


  
}
