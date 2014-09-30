// -*- mode: c++ -*-

#ifndef ROO1DPROCESSSCALING
#define ROO1DPROCESSSCALING

#include "RooRealProxy.h"
#include "RooAbsPdf.h"
#include "TProfile2D.h"
#include "TH1D.h"
#include "TF1.h"
#include "TString.h"
#include "TDirectoryFile.h"
#include <iostream>
#include <fstream>
#include <vector>

  
class Roo1DProcessScaling : public RooAbsReal {
public:
  
  Roo1DProcessScaling ();
  Roo1DProcessScaling (const char * name, const char * title,
				RooAbsReal& _x, 
				RooAbsReal& _operator1,
				RooAbsReal& _SM_shape,
				const char * parFilename);
  Roo1DProcessScaling (const Roo1DProcessScaling& other, const char * name);
  virtual TObject * clone(const char * newname) const { 
    return new Roo1DProcessScaling(*this, newname);
    }
  
  virtual ~Roo1DProcessScaling ();  
  
  void readProfiles(TDirectoryFile* dir) const ;
  TString getProfileFilename() const { return profileFilename; }
  
protected:
  
  RooRealProxy operator1;
  RooRealProxy operator2;
  
  double SM_integral;
  mutable std::vector<double> integral_basis;
  mutable std::vector<double> bins;
  mutable std::vector<double>          p0;
  mutable std::vector<double>          p1;
  mutable std::vector<double>          p2;

  TString profileFilename;
    
  void initializeProfiles();
  void initializeNormalization(const RooAbsReal& dep,
			       const RooAbsReal& shape);
  void readProfiles(Roo1DProcessScaling const& other); 
  double evaluate() const ;
  
private:
  
  ClassDef(Roo1DProcessScaling, 1) // aTGC function 
};

#endif
