/***************************************************************************** 
 * Project: RooFit                                                           * 
 *                                                                           * 
 * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/ 

// Your description goes here... 

#include "Riostream.h" 

#include "RooAbsData.h"
#include "HiggsAnalysis/CombinedLimit/interface/Roo2DSemiAnalyticPdf.h" 

#include <math.h> 
#include "TMath.h" 
#include "RooFormulaVar.h"
#include "RooRealVar.h"
#include "RooFit.h"
#include "TFile.h"

#include <iostream>

//using namespace RooFit ;

ClassImp(Roo2DSemiAnalyticPdf)

Roo2DSemiAnalyticPdf::Roo2DSemiAnalyticPdf()
{
  initializeProfiles();
}

Roo2DSemiAnalyticPdf::Roo2DSemiAnalyticPdf(const char *name, 
							     const char *title, 
							     RooAbsReal& _x,
							     RooAbsReal& _operator1,
							     RooAbsReal& _operator2,
							     RooAbsReal& _SM_shape,
							     const char * parFilename):
  RooAbsPdf(name,title),

  x("observable","observable",this,_x),
  operator1("operator1","operator1",this,_operator1),
  operator2("operator2","operator2",this,_operator2),
  SM_shape("SM_shape","SM_shape",this,_SM_shape),
  profileFilename(parFilename)

{ 
  const RooRealVar& rdep = dynamic_cast<const RooRealVar&>(_x);
  initializeBins(rdep);
  initializeProfiles();
  initializeNormalization(std::string(""),_x,_SM_shape);


   TFile *f = TFile::Open(parFilename,"READ");  
   readProfiles(f);
   f->Close();
} 

Roo2DSemiAnalyticPdf::Roo2DSemiAnalyticPdf(const Roo2DSemiAnalyticPdf& other, 
							     const char* name) :  
  RooAbsPdf(other,name),
  x("observable",this,other.x),
  operator1("operator1",this,other.operator1),
  operator2("operator2",this,other.operator2),
  SM_shape("SM_shape",this,other.SM_shape),
  integral_basis(other.integral_basis),
  bins(other.bins),
  profileFilename(other.profileFilename)
{ 
  initializeProfiles();
  initializeBins(x.arg());
  TFile *f = TFile::Open(profileFilename,"READ");  
  readProfiles(f);
  f->Close();
} 

void Roo2DSemiAnalyticPdf::initializeProfiles() {

  p0.clear();
  p1.clear();
  p2.clear();
  p3.clear();
  p4.clear();
  p5.clear();
  
}


void Roo2DSemiAnalyticPdf::initializeBins(const RooAbsReal& dep) const {

  bins.clear();
  const RooRealVar& rdep = dynamic_cast<const RooRealVar&>(dep);
  int N_bins=rdep.getBinning("").numBoundaries();
  double* array = rdep.getBinning("").array();
  for(int i=0; i<N_bins; ++i) {
    bins.push_back(array[i]);
  }
  
}


void Roo2DSemiAnalyticPdf::initializeNormalization(const std::string& rName,
							    const RooAbsReal& dep,
							    const RooAbsReal& shape) const {
  integral_basis.clear();

  int N_bins=bins.size()-1;
  int i;

  for(i=0; i<N_bins; ++i) {

    Double_t bin_low=bins[i];
    Double_t bin_high=bins[i+1];

    const RooRealVar& rdep = dynamic_cast<const RooRealVar&>(dep);
    RooRealVar& b = const_cast<RooRealVar&>(rdep);
    const string intRange="integRange";
    b.setRange((const char*)intRange.c_str(),bin_low,bin_high); 
    RooAbsReal* integral = shape.createIntegral(RooArgSet(rdep),RooArgSet(),(const char*)intRange.c_str());
    integral_basis.push_back(integral->getVal());

    delete integral;

  }


}

void Roo2DSemiAnalyticPdf::readProfiles(TDirectoryFile* dir) const {


  p0.clear(); p1.clear(); p2.clear(); p3.clear(); p4.clear(); p5.clear();
  int N_bins=bins.size()-1;
  for(int i=0; i<N_bins; ++i) {
    TF2* theFcn = (TF2*) dir->Get(TString::Format("bin_function_%i",i));
    p0.push_back(theFcn->GetParameter(0));
    p1.push_back(theFcn->GetParameter(1));
    p2.push_back(theFcn->GetParameter(2));
    p3.push_back(theFcn->GetParameter(3));
    p4.push_back(theFcn->GetParameter(4));
    p5.push_back(theFcn->GetParameter(5));
  }

}

void Roo2DSemiAnalyticPdf::readProfiles(Roo2DSemiAnalyticPdf const& other) {

  p0.clear(); p1.clear(); p2.clear(); p3.clear(); p4.clear(); p5.clear();
  int N_bins=bins.size()-1;
  for (int i = 0; i<N_bins; ++i) {
    p0.push_back(other.p0[i]);
    p1.push_back(other.p1[i]);
    p2.push_back(other.p2[i]);
    p3.push_back(other.p3[i]);
    p4.push_back(other.p4[i]);
    p5.push_back(other.p5[i]);
  }
}

Roo2DSemiAnalyticPdf::~Roo2DSemiAnalyticPdf() {}

Double_t Roo2DSemiAnalyticPdf::evaluate() const 
{ 


  double v1 = operator1;
  double v2 = operator2;
  double ret = 0.0;

  int N_bins=bins.size()-1;
  int bin_with_x=0;

  for (int idx=0;idx<N_bins;idx++) {     
    if (x<bins[idx+1] && x>bins[idx]) {
      bin_with_x=idx;
      break;
    }
  }


  ret = (p0[bin_with_x] + p1[bin_with_x]*v1 + p2[bin_with_x]*v2 + p3[bin_with_x]*v1*v2 + p4[bin_with_x]*v1*v1 + p5[bin_with_x]*v2*v2)*SM_shape;
 
  if (ret < 0.) ret = 0.;
  return ret; 

}

Int_t Roo2DSemiAnalyticPdf::
getAnalyticalIntegral(RooArgSet& allVars,RooArgSet& analVars, 
		      const char* rangeName) const {  
  if (matchArgs(allVars,analVars,x)) return 1 ;
  return 0 ;
}

Double_t Roo2DSemiAnalyticPdf::
analyticalIntegral(Int_t code, const char* rangeName) const {  

  assert(code==1 && "invalid analytic integration code!");

  double v1(0.0), v2(0.0);
  
  v1 = operator1;
  v2 = operator2;
  
  double ret(0.);

  int N_bins=bins.size()-1;
  for(Int_t i=0 ; i<N_bins ; i++) 
    ret += (p0[i] + p1[i]*v1 + p2[i]*v2 + p3[i]*v1*v2 + p4[i]*v1*v1 + p5[i]*v2*v2)*integral_basis[i];



  return ret;
}
