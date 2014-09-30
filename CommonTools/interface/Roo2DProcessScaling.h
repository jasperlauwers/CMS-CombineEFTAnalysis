// -*- mode: c++ -*-

#ifndef ROO2DPROCESSSCALING
#define ROO2DPROCESSSCALING

#include "RooRealProxy.h"
#include "RooAbsPdf.h"
#include "TProfile2D.h"
#include "TH2D.h"
#include "TF2.h"
#include "TString.h"
#include "TDirectoryFile.h"
#include <iostream>
#include <fstream>
#include <vector>

  
class Roo2DProcessScaling : public RooAbsReal {
public:
  
  Roo2DProcessScaling ();
  Roo2DProcessScaling (const char * name, const char * title,
				RooAbsReal& _x, 
				RooAbsReal& _operator1,
				RooAbsReal& _operator2, 
				RooAbsReal& _SM_shape,
				const char * parFilename);
  Roo2DProcessScaling (const Roo2DProcessScaling& other, const char * name);
  virtual TObject * clone(const char * newname) const { 
    return new Roo2DProcessScaling(*this, newname);
    }
  
  virtual ~Roo2DProcessScaling ();  
  
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
  mutable std::vector<double>          p3;
  mutable std::vector<double>          p4;
  mutable std::vector<double>          p5;

  TString profileFilename;
    
  void initializeProfiles();
  void initializeNormalization(const RooAbsReal& dep,
			       const RooAbsReal& shape);
  void readProfiles(Roo2DProcessScaling const& other); 
  double evaluate() const ;
  
private:
  
  ClassDef(Roo2DProcessScaling, 1) // aTGC function 
};

#endif
