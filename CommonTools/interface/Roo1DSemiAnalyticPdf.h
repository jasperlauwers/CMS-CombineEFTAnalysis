// -*- mode: c++ -*-

#ifndef ROO1DSEMIANALYTICPDF
#define ROO1DSEMIANALYTICPDF

#include "RooRealProxy.h"
#include "RooAbsPdf.h"
#include "TF1.h"
#include "TH1D.h"
#include "TString.h"
#include "TDirectoryFile.h"
  
class Roo1DSemiAnalyticPdf : public RooAbsPdf {
public:
  
  Roo1DSemiAnalyticPdf ();
  Roo1DSemiAnalyticPdf (const char * name, const char * title,
				 RooAbsReal& _x, 
				 RooAbsReal& _operator1, 
				 RooAbsReal& _SM_shape,
				 const char * parFilename);
  Roo1DSemiAnalyticPdf (const Roo1DSemiAnalyticPdf& other, const char * name);

  virtual TObject * clone(const char * newname) const { 
    return new Roo1DSemiAnalyticPdf(*this, newname);
  }
  
  virtual ~Roo1DSemiAnalyticPdf ();
  
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
  
  TString profileFilename;  
  
  void initializeProfiles();
  void initializeBins(const RooAbsReal& shape) const;
  void initializeNormalization(const std::string& rName,
			       const RooAbsReal& dep,
			       const RooAbsReal& shape) const;

  void readProfiles(Roo1DSemiAnalyticPdf const& other);  
  virtual double evaluate() const ;
  
private:
  
  ClassDef(Roo1DSemiAnalyticPdf, 4) // im6 function 
};

#endif
