// -*- mode: c++ -*-

#ifndef ROO2DSEMIANALYTICPDF
#define ROO2DSEMIANALYTICPDF

#include "RooRealProxy.h"
#include "RooAbsPdf.h"
#include "TF2.h"
#include "TH2D.h"
#include "TString.h"
#include "TDirectoryFile.h"
  
class Roo2DSemiAnalyticPdf : public RooAbsPdf {
public:
  
  Roo2DSemiAnalyticPdf ();
  Roo2DSemiAnalyticPdf (const char * name, const char * title,
				 RooAbsReal& _x, 
				 RooAbsReal& _operator1, 
				 RooAbsReal& _operator2,
				 RooAbsReal& _SM_shape,
				 const char * parFilename);
  Roo2DSemiAnalyticPdf (const Roo2DSemiAnalyticPdf& other, const char * name);

  virtual TObject * clone(const char * newname) const { 
    return new Roo2DSemiAnalyticPdf(*this, newname);
  }
  
  virtual ~Roo2DSemiAnalyticPdf ();
  
  Int_t getAnalyticalIntegral(RooArgSet& allVars, 
			      RooArgSet& analVars, 
			      const char* rangeName = 0) const;
  
  Double_t analyticalIntegral(Int_t code, const char* rangeName = 0) const;

  void readProfiles(TDirectoryFile* dir) const ;
  TString getProfileFilename() const { return profileFilename; }
  
protected:
  
  RooRealProxy x;
  RooRealProxy operator1;
  RooRealProxy operator2;
  RooRealProxy SM_shape;
  
  mutable std::vector<double>                integral_basis;
  mutable std::vector<double>                bins; // important to be mutable!!!
  mutable std::vector<double>          p0;
  mutable std::vector<double>          p1;
  mutable std::vector<double>          p2;
  mutable std::vector<double>          p3;
  mutable std::vector<double>          p4;
  mutable std::vector<double>          p5;
  
  TString profileFilename;  
  
  void initializeProfiles();
  void initializeBins(const RooAbsReal& shape) const;
  void initializeNormalization(const std::string& rName,
			       const RooAbsReal& dep,
			       const RooAbsReal& shape) const;

  void readProfiles(Roo2DSemiAnalyticPdf const& other);  
  virtual double evaluate() const ;
  
private:
  
  ClassDef(Roo2DSemiAnalyticPdf, 4) // im6 function 
};

#endif
