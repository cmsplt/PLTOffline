////////////////////////////////////////////////////////////////////
//
// Krishna Thapa <kthapa@cern.ch>
//
// Created on: Tue July 05
//
// Find appropriate model for SlopeX, get parameters
////////////////////////////////////////////////////////////////////
{
  gSystem->Load("libMinuit") ; // ROOT4 only
  gSystem->Load("libRooFit") ;
  using namespace RooFit ;

  cout << "VdM SlopeX Model" << endl;
  cout << "------------------------------------" << endl;

  // Declare variables x,mean,sigma with associated name, title, initial value and allowed range
  RooRealVar num("num","num",0,5000000);
  RooRealVar tr("tr","tr",0,5);
  RooRealVar sx("sx","sx",-0.1,0.1);
  RooRealVar sy("sy","sy",-0.1,0.1);
  //  RooRealVar pt("pt","pt",13);
  gStyle->SetOptFit(01111);

  // read dataset from file
  RooDataSet* data = RooDataSet::read("./xyVdM.txt",
                                      RooArgList(num,sx,sy));//no cut pt 13

  //  RooDataSet* data = RooDataSet::read("./xyVdMColl.txt",
  //                                      RooArgList(num,sx,sy));// colliding bunches

  //---------------------------------------------------------

  //SlopeX signal
  RooRealVar meanSsx("meanSsx","Signal Slope Y-mean",0.000285281,-0.03,0.03);
  RooRealVar sigmaSsx("sigmaSsx","Signal Y-sigma",0.0155009,0.0,0.1);
  RooGaussian Ssx("Ssx","Sx signal PDF",sx,meanSsx,sigmaSsx);

  
  //SlopeX bkgd

  //Anoter gauus
  RooRealVar G1mean("G1mean", "G1mean",-7.66321e-06,-0.01,0.01);
  RooRealVar G1sigma("G1sigma", "G1sigma",0.00673643,0.0,0.02);
  RooGaussian G1("G1","G1 bgd PDF",sx,G1mean,G1sigma);

  RooRealVar G2mean("G2mean", "G2mean",0.000171416,-0.01,0.01);
  RooRealVar G2sigma("G2sigma", "G2sigma",0.00298365,0.0,0.1);
  RooGaussian G2("G2","G2 bgd PDF",sx,G2mean,G2sigma);

  //fixed to best fit
  RooRealVar frac1("frac1","outlier fraction",0.123296,0.0,1.0);//,0.,1.0);
RooRealVar frac2("frac2","tail fraction",0.280948,0.,1.0);
  //  RooRealVar frac3("frac3","extra  gauss fraction",0.5,0.0,1.0);


  // background
   RooAddPdf bmodel ("bmodel","Model: BifG+G",
                     RooArgList(G1,G2),frac2);

  //VdM Model
  RooAddPdf VmodelX ("VmodelX","Model:S+B",
                   RooArgList(Ssx,bmodel),frac1);

  // RooAddPdf f_model ("f_model","Model:S+B",
  //                  RooArgList(VmodelX,bfgP),frac3);

  //VdM Model
  RooFitResult* r = VmodelX.fitTo(*data,Save());

  RooPlot* xframe1 =  new RooPlot("p.d.f. with data:Sx","plot",sx,-0.07,0.07,100);


  //  data->plotOn(xframe1);
  data->plotOn(xframe1);
  VmodelX.plotOn(xframe1);
  //for vdm
   VmodelX.plotOn(xframe1,RooFit::Components("Ssx"),RooFit::LineStyle(kDashed));//core
   VmodelX.plotOn(xframe1,RooFit::Components("G2"),RooFit::LineColor(kRed));//bif. gauss
   VmodelX.plotOn(xframe1,RooFit::Components("G1"),RooFit::LineColor(kGreen));//gauss

  //VmodelX+
  // f_model.plotOn(xframe1);
  // f_model.plotOn(xframe1,RooFit::Components("VmodelX"),RooFit::LineColor(kGreen));
  // f_model.plotOn(xframe1,RooFit::Components("bfgP"),RooFit::LineColor(kMagenta));//extra bif. new Gauss


  xframe1->SetXTitle("SlopeX");
  xframe1->SetTitle("SlopeX Distribution");

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

  std::cout << "Outlier fraction : " << frac1 << std::endl;

}
