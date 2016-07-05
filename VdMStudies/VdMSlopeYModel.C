////////////////////////////////////////////////////////////////////
//
// Krishna Thapa <kthapa@cern.ch>
//
// Created on: Tue July 05
//
//
// Find appropriate model for SlopeY, get parameters
////////////////////////////////////////////////////////////////////

{
  gSystem->Load("libMinuit") ; // ROOT4 only
  gSystem->Load("libRooFit") ;
  using namespace RooFit ;

  cout << "VdM SlopeY Model" << endl;
  cout << "------------------------------------" << endl;

  // Declare variables x,mean,sigma with associated name, title, initial value and allowed range
  RooRealVar num("num","num",0,5000000);
  //  RooRealVar tr("tr","tr",0,5);
  RooRealVar sy("sy","sy",-0.1,0.1);
  RooRealVar sx("sx","sx",-0.1,0.1);
  gStyle->SetOptFit(01111);

  // read dataset from file
  RooDataSet* data = RooDataSet::read("./xyVdM.txt",
                                      RooArgList(num,sx,sy));//no cut pt 13
  //  RooDataSet* data = RooDataSet::read("./xyVdMColl.txt",
  //                                      RooArgList(num,sx,sy));// colliding bunches

  //---------------------------------------------------------

  //SlopeY signal
  RooRealVar meanSsy("meanSsy","Signal Slope Y-mean",0.0269566,0.,0.04);
  RooRealVar sigmaSsy("sigmaSsy","Signal Y-sigma",0.00126419,0.,0.01);
  RooGaussian Ssy("Ssy","Sy signal PDF",sy,meanSsy,sigmaSsy);

  //SlopeY bkgd

  //gaussian bkgd
  //  RooRealVar meanBsy("meanBsy","Bkgd Slope Y-mean", 0.015, -0.01, 0.02);
  //  RooRealVar sigmaBsy("sigmaBsy","Bkgd Y-sigma", 0.02,0.0,0.04);
  // RooGaussian Bsy("Bsy","Sy bgd PDF",sy,meanBsy,sigmaBsy);


  //Crystall ball bkgd
  // RooRealVar cbmean("cbmean","cb mean",0.015,-0.02,0.02);
  // RooRealVar cbsigma("cbsigma","cb sigma",0.03,0.01,0.08);
  // RooRealVar n("n","cb n",2);
  // RooRealVar alpha("alpha","cb alpha",-1.5);
  // RooCBShape cball("cball","crystal ball bkgd pdf",sy,cbmean,cbsigma,alpha,n);


  //Bifurcated gauss
  RooRealVar bfmean("bfmean","bif mean",0.02465610,0.,0.04);
  RooRealVar bfsigmaL("bfsigmaL","bif sigmaLeft",0.01,0.,0.1);
  RooRealVar bfsigmaR("bfsigmaR","bif sigmaRight",0.011,0.,0.1);
  RooBifurGauss bfg("bfg","bif gauss bkgd pdf",sy,bfmean,bfsigmaL,bfsigmaR);


  //Polynomial bkgd
  //  RooRealVar bkgd_poly_c1("bkgd_poly_c1", "x^1 term coeff",0.,0,100);
  //  RooRealVar bkgd_poly_c2("bkgd_poly_c2", "x^2 term coeff",0.,-100,100);
  //    RooPolynomial BpSy("BpSy", "quadratic polynomial",sy,RooArgList(bkgd_poly_c1,bkgd_poly_c2));
  //  RooPolynomial BpSy("BpSy", "Bkgd: Linear polynomial",sy);
  //                     RooArgList(bkgd_poly_c1));//1+x

  RooRealVar BpSymean("BpSymean", "BpSymean",0.03,0.01,0.05);
  RooRealVar BpSysig("BpSysig", "BpSysig",0.1,0.0,1.);

  RooGaussian BpSy("BpSy","BpSy bgd PDF",sy,BpSymean,BpSysig);



  //fixed to best fit
  //  RooRealVar ns("ns","signal yield",0,10000);
  // RooRealVar nb("nb","outlier yield",0,10000);

  RooRealVar frac1("frac1","outlier fraction",0.5,0.0,1.0);
  RooRealVar frac2("frac2","tail fraction",0.5,0.0,1.0);

  //polynomial
  // RooAddPdf bModel ("bModel","bModel",
  //                  RooArgList(Bsy,BpSy),RooArgList(frac2));

  // background
  RooAddPdf bmodel ("bmodel","Model: B",
                     RooArgList(bfg,BpSy),frac2);

  // final model
  RooAddPdf model ("model","Model:S+B",
                   RooArgList(Ssy,bmodel),frac1);

  //gaussian bkgd
  //  RooAddPdf model("model","Model",
  //                RooArgList(Ssy,Bsy),RooArgList(frac1));//,frac2));

  // Fit pdf to data
  //  model.fitTo(*data,Extended(kTRUE));
  //  model.fitTo(*data);
  RooFitResult* r = model.fitTo(*data,Save());

  RooPlot* xframe1 =  new RooPlot("p.d.f. with data:Sy","plot",sy,-0.03,0.07,20);


  //  data->plotOn(xframe1);
  data->plotOn(xframe1);
  model.plotOn(xframe1);

  model.plotOn(xframe1,RooFit::Components("Ssy"),RooFit::LineStyle(kDashed));
  model.plotOn(xframe1,RooFit::Components("bfg"),RooFit::LineColor(kRed));
  model.plotOn(xframe1,RooFit::Components("BpSy"),RooFit::LineColor(kGreen));

  xframe1->SetXTitle("SlopeY");
  xframe1->SetTitle("SlopeY Distribution");

  xframe1->Draw();


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

  // Visualize ellipse corresponding to single correlation matrix element
  //RooPlot* frame = new RooPlot(sigma1,sig1frac,0.45,0.60,0.65,0.90) ;
  //  frame->SetTitle("Covariance between sigma1 and sig1frac") ;
  //  r->plotOn(frame,sigma1,sig1frac,"ME12ABHV") ;




  // A c c e s s   f i t   r e s u l t   i n f o r m a t i o n
  // ---------------------------------------------------------

  // Access basic information
  cout << "EDM = " << r->edm() << endl ;
  cout << "-log(L) at minimum = " << r->minNll() << endl ;

  // Access list of final fit parameter values
  cout << "final value of floating parameters" << endl ;
  r->floatParsFinal().Print("s") ;

  // Access correlation matrix elements
  //  cout << "correlation between sig1frac and a0 is  " << r->correlation(sig1frac,a0) << endl ;
  //  cout << "correlation between bkgfrac and mean is " << r->correlation("bkgfrac","mean") << endl ;

  // Extract covariance and correlation matrix as TMatrixDSym
  const TMatrixDSym& cor = r->correlationMatrix() ;
  const TMatrixDSym& cov = r->covarianceMatrix() ;

  // Print correlation, covariance matrix
  cout << "correlation matrix" << endl ;
  cor.Print() ;
  cout << "covariance matrix" << endl ;
  cov.Print() ;


  //  TCanvas* c = new TCanvas("testing testing","testing testing",800,400) ;
  //  c->Divide(1) ;
  //  c->cd(1) ; gPad->SetLeftMargin(0.15) ; xframe1->GetYaxis()->SetTitleOffset(1.6) ; xframe1->Draw() ;

  /*
  std::cout << "=========================" << std::endl;
  std::cout << "Printing slopeYModel..."<<std::endl;
  model.Print();
  std::cout << "Printing SignalModel..."<<std::endl;
  Ssy.Print();
  std::cout << "Printing SignalModel..."<<std::endl;
  bfg.Print();
  */
  std::cout << "Outlier fraction : " << frac1 << std::endl;
  // std::cout << "Frac_2 fraction : " << frac2 << std::endl;

}
