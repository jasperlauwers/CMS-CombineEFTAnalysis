#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TCanvas.h"

void save1DPlot(TString param)
{
    TFile *file = new TFile("higgsCombinetest_VBS_" + param +".MultiDimFit.mH120.root","READ");
    TTree *limit = (TTree*) file->Get("limit");
    TCanvas *c = new TCanvas("c","c");
    limit->Draw("2*deltaNLL:"+param);
    c->SaveAs(("deltaNLL_"+param+".png").Data(),"png");
}

