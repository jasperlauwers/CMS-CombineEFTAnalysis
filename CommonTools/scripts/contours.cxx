TGraph* bestFit2D(TTree *t, TString x, TString y, TCut cut) {
    int nfind = t->Draw(y+":"+x, cut + "deltaNLL == 0");
    if (nfind == 0) {
        TGraph *gr0 = new TGraph(1);
        gr0->SetPoint(0,-999,-999);
        gr0->SetMarkerStyle(34); gr0->SetMarkerSize(2.0);
        return gr0;
    } else {
        TGraph *gr0 = (TGraph*) gROOT->FindObject("Graph")->Clone();
        gr0->SetMarkerStyle(34); gr0->SetMarkerSize(2.0);
        if (gr0->GetN() > 1) gr0->Set(1);
        return gr0;
    }
}

double bestFit1D(TTree *t, TString x) {
  
  float retval = -999;
  float deltanll = -999;

  t->SetBranchAddress(x, &retval);
  t->SetBranchAddress("deltaNLL", &deltanll);

  int nentries = t->GetEntries();
  for (int i=0; i<nentries; i++) {
    t->GetEntry(i);
    if (deltanll == 0) break;
  }
    
  return retval;

}

TH1 *treeToHist1D(TTree *t, TString x, TString name, TCut cut, double xmin, double xmax, int xbins) {
    t->Draw(Form("2*deltaNLL:%s>>%s_prof(%d,%10g,%10g)", x.Data(), name.Data(), xbins, xmin, xmax), cut + "deltaNLL != 0", "PROF");
    TH1 *prof = (TH1*) gROOT->FindObject(name+"_prof");
    TH1D *h1d = new TH1D(name, name, xbins, xmin, xmax);
    for (int ix = 1; ix <= xbins; ++ix) {
      double y = prof->GetBinContent(ix);
      if (y != y) y = (name.Contains("bayes") ? 0 : 999); // protect agains NANs
      h1d->SetBinContent(ix, y);
    }
    h1d->GetXaxis()->SetTitle(x+"/#Lambda^2");
    h1d->SetDirectory(0);
    return h1d;
}



TH2 *treeToHist2D(TTree *t, TString x, TString y, TString name, TCut cut, double xmin, double xmax, double ymin, double ymax, int xbins, int ybins) {
    t->Draw(Form("2*deltaNLL:%s:%s>>%s_prof(%d,%10g,%10g,%d,%10g,%10g)", y.Data(), x.Data(), name.Data(), xbins, xmin, xmax, ybins, ymin, ymax), cut + "deltaNLL != 0", "PROF");
    TH2 *prof = (TH2*) gROOT->FindObject(name+"_prof");
    TH2D *h2d = new TH2D(name, name, xbins, xmin, xmax, ybins, ymin, ymax);
    for (int ix = 1; ix <= xbins; ++ix) {
        for (int iy = 1; iy <= ybins; ++iy) {
             double z = prof->GetBinContent(ix,iy);
             if (z != z) z = (name.Contains("bayes") ? 0 : 999); // protect agains NANs
             h2d->SetBinContent(ix, iy, z);
        }
    }
    h2d->GetXaxis()->SetTitle(x+"/#Lambda^2");
    h2d->GetYaxis()->SetTitle(y+"/#Lambda^2");
    h2d->SetDirectory(0);
    return h2d;
}

TList* contourFromTH2(TH2 *h2in, double threshold, int minPoints=20) {
    std::cout << "Getting contour at threshold " << threshold << " from " << h2in->GetName() << std::endl;
    //http://root.cern.ch/root/html/tutorials/hist/ContourList.C.html
    Double_t contours[1];
    contours[0] = threshold;
    if (h2in->GetNbinsX() * h2in->GetNbinsY() > 10000) minPoints = 50;
    if (h2in->GetNbinsX() * h2in->GetNbinsY() <= 100) minPoints = 10;

    TH2D *h2 = frameTH2D((TH2D*)h2in,threshold);

    h2->SetContour(1, contours);

    // Draw contours as filled regions, and Save points
    h2->Draw("CONT Z LIST");
    gPad->Update(); // Needed to force the plotting and retrieve the contours in TGraphs


    // Get Contours
    TObjArray *conts = (TObjArray*)gROOT->GetListOfSpecials()->FindObject("contours");
    TList* contLevel = NULL;

    if (conts == NULL || conts->GetSize() == 0){
        printf("*** No Contours Were Extracted!\n");
        return 0;
    }

    TList *ret = new TList();
    for(int i = 0; i < conts->GetSize(); i++){
        contLevel = (TList*)conts->At(i);
        //printf("Contour %d has %d Graphs\n", i, contLevel->GetSize());
        for (int j = 0, n = contLevel->GetSize(); j < n; ++j) {
            TGraph *gr1 = (TGraph*) contLevel->At(j);
            //printf("\t Graph %d has %d points\n", j, gr1->GetN());
            if (gr1->GetN() > minPoints) ret->Add(gr1->Clone());
            //break;
        }
    }
    return ret;
}

TList* contourFromTH1(TH1 *h1in, double threshold) {

    std::cout << "Getting contour at threshold " << threshold << " from " << h1in->GetName() << std::endl;
    //http://root.cern.ch/root/html/tutorials/hist/ContourList.C.html

    TGraphErrors* retval = new TGraphErrors();
    int npt = 0;

    TH1D *h1 = frameTH1D((TH1D*)h1in,threshold);

    for (Int_t i = 1; i < h1->GetNbinsX(); ++i) {
      double thisx = h1->GetXaxis()->GetBinCenter(i);
      double thisy = h1->GetBinContent(i);
      double nextx = h1->GetXaxis()->GetBinCenter(i+1);
      double nexty = h1->GetBinContent(i+1);
      double x;

      if (((threshold <= thisy) && (threshold > nexty)) || ((threshold >= thisy) && (threshold < nexty))) {
	x = thisx + (threshold-thisy)*(nextx-thisx)/(nexty-thisy);
	retval->SetPoint(npt, x, threshold/2.);
	retval->SetPointError(npt, 0, threshold/2.);
	npt++;			 
      }
    }

    if (npt == 0){
        printf("*** No Contours Were Extracted!\n");
        return 0;
    }

    TGraphErrors* line = new TGraphErrors(1);
    line->SetPoint(0, 0., threshold);
    line->SetPointError(0, 999., 0.);

    // Just to be similar to the 2D case that is defined by the TH2 method
    TList *ret = new TList();
    ret->Add(retval);
    ret->Add(line);
    return ret;

}

TH2D* frameTH2D(TH2D *in, double threshold){
        // NEW LOGIC:
        //   - pretend that the center of the last bin is on the border if the frame
        //   - add one tiny frame with huge values
        double frameValue = 1000;
        if (TString(in->GetName()).Contains("bayes")) frameValue = -1000;

	Double_t xw = in->GetXaxis()->GetBinWidth(1);
	Double_t yw = in->GetYaxis()->GetBinWidth(1);

	Int_t nx = in->GetNbinsX();
	Int_t ny = in->GetNbinsY();

	Double_t x0 = in->GetXaxis()->GetXmin();
	Double_t x1 = in->GetXaxis()->GetXmax();

	Double_t y0 = in->GetYaxis()->GetXmin();
	Double_t y1 = in->GetYaxis()->GetXmax();
        Double_t xbins[999], ybins[999]; 
        double eps = 0.1;

        xbins[0] = x0 - eps*xw - xw; xbins[1] = x0 + eps*xw - xw;
        for (int ix = 2; ix <= nx; ++ix) xbins[ix] = x0 + (ix-1)*xw;
        xbins[nx+1] = x1 - eps*xw + xw; xbins[nx+2] = x1 + eps*xw + xw;

        ybins[0] = y0 - eps*yw - yw; ybins[1] = y0 + eps*yw - yw;
        for (int iy = 2; iy <= ny; ++iy) ybins[iy] = y0 + (iy-1)*yw;
        ybins[ny+1] = y1 - eps*yw + yw; ybins[ny+2] = y1 + eps*yw + yw;
        
	TH2D *framed = new TH2D(
			Form("%s framed",in->GetName()),
			Form("%s framed",in->GetTitle()),
			nx + 2, xbins,
			ny + 2, ybins 
			);

	//Copy over the contents
	for(int ix = 1; ix <= nx ; ix++){
		for(int iy = 1; iy <= ny ; iy++){
			framed->SetBinContent(1+ix, 1+iy, in->GetBinContent(ix,iy));
		}
	}
	//Frame with huge values
	nx = framed->GetNbinsX();
	ny = framed->GetNbinsY();
	for(int ix = 1; ix <= nx ; ix++){
		framed->SetBinContent(ix,  1, frameValue);
		framed->SetBinContent(ix, ny, frameValue);
	}
	for(int iy = 2; iy <= ny-1 ; iy++){
		framed->SetBinContent( 1, iy, frameValue);
		framed->SetBinContent(nx, iy, frameValue);
	}

	return framed;
}

TH1D* frameTH1D(TH1D *in, double threshold){


  double frameValue = 1000;
  if (TString(in->GetName()).Contains("bayes")) frameValue = -1000;

  Int_t nx = in->GetNbinsX();

  Double_t x0 = in->GetXaxis()->GetXmin();
  Double_t x1 = in->GetXaxis()->GetXmax();

  TH1D* framed = new TH1D(Form("%s framed", in->GetName()), Form("%s framed", in->GetTitle()), nx, x0, x1);
  for (int ix = 1; ix <= framed->GetNbinsX(); ix++)
    framed->SetBinContent(ix, in->GetBinContent(ix));

  for (int ix = 1; ix <= framed->GetNbinsX(); ix++) {
    if (framed->GetBinContent(ix) == 0) 
      framed->SetBinContent(ix, frameValue);
    else
      break;
  }

  for (int ix = framed->GetNbinsX(); ix >= 1; ix--) {
    if (framed->GetBinContent(ix) == 0)
      framed->SetBinContent(ix, frameValue);
    else
      break;
  }

  return framed;

}


void styleMultiGraph(TList *tmg, int lineColor, int lineWidth, int lineStyle) {
    for (int i = 0; i < tmg->GetSize(); ++i) {
        TGraph *g = (TGraph*) tmg->At(i);
        g->SetLineColor(lineColor); g->SetLineWidth(lineWidth); g->SetLineStyle(lineStyle);
    }
}
void styleMultiGraphMarker(TList *tmg, int markerColor, int markerSize, int markerStyle) {
    for (int i = 0; i < tmg->GetSize(); ++i) {
        TGraph *g = (TGraph*) tmg->At(i);
        g->SetMarkerColor(markerColor); g->SetMarkerSize(markerSize); g->SetMarkerStyle(markerStyle);
    }
}


/** Make a 2D contour plot from the output of MultiDimFit
 *
 * Inputs:
 *  - gFile should point to the TFile containing the 'limit' TTree
 *  - xvar should be the variable to use on the X axis, with xbins bins in the [xmin, xmax] range
 *  - yvar should be the variable to use on the Y axis, with ybins bins in the [ymin, ymax] range
 *  - (smx, smy) are the coordinates at which to put a diamond representing the SM expectation
 *  - if fOut is not null, then the output objects will be saved to fOut:
 *     - the 2D histogram will be saved as a TH2 with name name+"_h2d"
 *     - the 68% CL contour will be saved as a TList of TGraphs with name name+"_c68"
 *     - the 95% CL contour will be saved as a TList of TGraphs with name name+"_c95"
 *     - the 99.7% CL contour will be saved as a TList of TGraphs with name name+"_c997"
 *     - the best fit point will be saved as a TGraph with name name+"_best"
 *
 * Notes:
 *     - it's up to you to make sure that the binning you use for this plot matches the one used
 *       when running MultiDimFit (but you can just plot a subset of the points; e.g. if you had
 *       100x100 points in [-1,1]x[-1,1] you can make a 50x50 plot for [0,1]x[0,1])
 *     - the 99.7 contour is not plotted by default
 *     - the SM marker is not saved
*/
TList* contour2D(TFile* inputfile, TString xvar, int xbins, float xmin, float xmax, TString yvar, int ybins, float ymin, float ymax, float smx=1.0, float smy=1.0, TFile *fOut=0, TString name="contour2D") {
    TTree *tree = (TTree*) inputfile->Get("limit") ;
    TH2 *hist2d = treeToHist2D(tree, xvar, yvar, "h2d", "", xmin, xmax, ymin, ymax, xbins, ybins);
    hist2d->SetContour(200);
    hist2d->GetZaxis()->SetRangeUser(0,21);
    TGraph *fit = bestFit2D(tree, xvar, yvar, "");
    TList *c68 = contourFromTH2(hist2d, 2.30);
    TList *c95 = contourFromTH2(hist2d, 5.99);
    TList *c997 = contourFromTH2(hist2d, 11.83);
    styleMultiGraph(c68,  /*color=*/1, /*width=*/3, /*style=*/1);
    styleMultiGraph(c95,  /*color=*/1, /*width=*/3, /*style=*/9);
    styleMultiGraph(c997, /*color=*/1, /*width=*/3, /*style=*/2);
    hist2d->Draw("COLZ"); gStyle->SetOptStat(0);
    c68->Draw("L SAME");
    c95->Draw("L SAME");
    fit->Draw("P SAME");
    TMarker* m1 = new TMarker();
    TMarker* m2 = new TMarker();
    m1->SetMarkerSize(3.0); m1->SetMarkerColor(97); m1->SetMarkerStyle(33); 
    m1->DrawMarker(smx,smy);
    m2->SetMarkerSize(1.8); m2->SetMarkerColor(89); m2->SetMarkerStyle(33); 
    m2->DrawMarker(smx,smy);
    if (fOut != 0) {
        hist2d->SetName(name+"_h2d");  fOut->WriteTObject(hist2d,0);
        fit->SetName(name+"_best");    fOut->WriteTObject(fit,0);
        c68->SetName(name+"_c68");     fOut->WriteTObject(c68,0,"SingleKey");
        c95->SetName(name+"_c95");     fOut->WriteTObject(c95,0,"SingleKey");
        c997->SetName(name+"_c997");   fOut->WriteTObject(c997,0,"SingleKey");
    }
    TList* retval = new TList();
    retval->Add(hist2d);
    retval->Add(fit);
    retval->Add(c68);
    retval->Add(c95);
    retval->Add(m1);
    retval->Add(m2);
    return retval;
}

TList* contour1D(TFile* inputfile, TString xvar, int xbins, float xmin, float xmax, TFile *fOut=0, TString name="contour1D") {

    TTree *tree = (TTree*) inputfile->Get("limit") ;
    TH1 *hist1d = treeToHist1D(tree, xvar, "h1d", "", xmin, xmax, xbins);
    double fit = bestFit1D(tree, xvar);
    TGraph* best = new TGraph(1);
    best->SetPoint(0, fit, 0.);
    TList *c68 = contourFromTH1(hist1d, 1.00);
    TList *c95 = contourFromTH1(hist1d, 3.84);
    TList *c997 = contourFromTH1(hist1d, 9.00);
    ((TGraphErrors*) c68->At(0))->SetLineColor(kRed);
    ((TGraphErrors*) c95->At(0))->SetLineColor(kRed);
    ((TGraphErrors*) c997->At(0))->SetLineColor(kRed);
    ((TGraphErrors*) c68->At(0))->SetLineWidth(2);
    ((TGraphErrors*) c95->At(0))->SetLineWidth(2);
    ((TGraphErrors*) c997->At(0))->SetLineWidth(2);
    ((TGraphErrors*) c68->At(0))->SetMarkerSize(0);
    ((TGraphErrors*) c95->At(0))->SetMarkerSize(0);
    ((TGraphErrors*) c997->At(0))->SetMarkerSize(0);

    ((TGraphErrors*) c68->At(1))->SetLineColor(kRed);
    ((TGraphErrors*) c95->At(1))->SetLineColor(kRed);
    ((TGraphErrors*) c997->At(1))->SetLineColor(kRed);
    ((TGraphErrors*) c68->At(1))->SetLineWidth(2);
    ((TGraphErrors*) c95->At(1))->SetLineWidth(2);
    ((TGraphErrors*) c997->At(1))->SetLineWidth(2);
    ((TGraphErrors*) c68->At(1))->SetMarkerSize(0);
    ((TGraphErrors*) c95->At(1))->SetMarkerSize(0);
    ((TGraphErrors*) c997->At(1))->SetMarkerSize(0);
    TGraph* gr1d = new TGraph(hist1d);
    gr1d->Draw("A");
    gr1d->SetMarkerSize(0);
    gr1d->SetLineWidth(3);
    gr1d->SetLineColor(kBlack);
    gr1d->GetYaxis()->SetRangeUser(0,7);
    gr1d->GetXaxis()->SetTitle(xvar.Data());
    gr1d->Draw("ALP"); gStyle->SetOptStat(0);
    c68->Draw("Z SAME");
    c95->Draw("Z SAME");
    if (fOut != 0) {
        gr1d->SetName(name+"_gr1d");  fOut->WriteTObject(hist2d,0);
        c68->SetName(name+"_c68");     fOut->WriteTObject(c68,0,"SingleKey");
        c95->SetName(name+"_c95");     fOut->WriteTObject(c95,0,"SingleKey");
        c997->SetName(name+"_c997");   fOut->WriteTObject(c997,0,"SingleKey");
    }
    
    TList* ret = new TList();
    ret->Add(gr1d);
    ret->Add(c68);
    ret->Add(c95);
    ret->Add(best);
    return ret;
}

void drawExpected2D(TString expectedFile, TString expectedNSFile, TString xvar, int xbins, float xmin, float xmax, TString yvar, int ybins, float ymin, float ymax, float smx, float smy, TString titlex, TString titley, TString colorName, TString BWname) {

  tdrstyle();
  TFile* _file0 = TFile::Open(expectedFile);
  TFile* _file1 = TFile::Open(expectedNSFile);
    
  TList* expList = contour2D(_file0, xvar, xbins, xmin, xmax, yvar, ybins, ymin, ymax, 0.0, 0.0);
  TList* expNSList = contour2D(_file1, xvar, xbins, xmin, xmax, yvar, ybins, ymin, ymax, 0.0, 0.0);
  
  TH2D* h2exp = (TH2D*) expList->At(0);
  fixTH2D(h2exp,21);
  h2exp->GetXaxis()->SetTitle(titlex);
  h2exp->GetYaxis()->SetTitle(titley);

  TCanvas* expectedColor_canvas = new TCanvas("expectedColor_canvas", "");
  h2exp->Draw("colz");
  expList->At(2)->Draw("L SAME");
  expList->At(3)->Draw("L SAME");
  ((TMarker*) expList->At(4))->DrawMarker(smx,smy);
  ((TMarker*) expList->At(5))->DrawMarker(smx,smy);

  TLatex lt_ver;
  lt_ver.SetTextFont(42);
  lt_ver.SetTextColor(1);
  lt_ver.SetTextSize(0.06);
  lt_ver.SetTextAlign(12);
  lt_ver.SetTextAngle(90);
  lt_ver.SetNDC();

  TLatex lt_hor;
  lt_hor.SetTextFont(42);
  lt_hor.SetTextColor(1);
  lt_hor.SetTextSize(0.05);
  lt_hor.SetTextAlign(32);
  lt_hor.SetTextAngle(0);
  lt_hor.SetNDC();

  TLatex lt_hor2;
  lt_hor2.SetTextFont(42);
  lt_hor2.SetTextColor(1);
  lt_hor2.SetTextSize(0.05);
  lt_hor2.SetTextAlign(12);
  lt_hor2.SetTextAngle(0);
  lt_hor2.SetNDC();

  lt_ver.DrawLatex(0.95, 0.75,"-2#Delta ln L");
  lt_hor.DrawLatex(0.8, 0.96,"L=19.4 fb^{-1} (8 TeV)");
  lt_hor2.DrawLatex(0.2, 0.88, "#bf{CMS} (preliminary)");

  expectedColor_canvas->Print(colorName);

  TCanvas* expectedBW_canvas = new TCanvas("expectedBW_canvas", "");
  h2exp->Draw("axis");
  expList->At(2)->Draw("L SAME");
  ((TGraph*) ((TList*) expNSList->At(2))->At(0))->SetLineWidth(1);
  expNSList->At(2)->Draw("L SAME");
  expList->At(3)->Draw("L SAME");
  ((TGraph*) ((TList*) expNSList->At(3))->At(0))->SetLineWidth(1);
  expNSList->At(3)->Draw("L SAME");
  expList->At(1)->Draw("P SAME");
  lt_hor.DrawLatex(0.8, 0.96,"L=19.4 fb^{-1} (8 TeV)");
  lt_hor2.DrawLatex(0.2, 0.88, "#bf{CMS} (preliminary)");
  
  TLegend* leg = new TLegend(0.2, 0.75, 0.6, 0.85);
  leg->AddEntry(((TGraph*) ((TList*) expList->At(2))->At(0)), "Expected 68\% CL", "l");
  leg->AddEntry(((TGraph*) ((TList*) expList->At(3))->At(0)), "Expected 95\% CL", "l");
  leg->SetFillColor(0);
  leg->SetBorderSize(0);
  leg->Draw();

  expectedBW_canvas->Print(BWname);

}

void drawExpected1D(TString expectedFile, TString expectedNSFile, TString xvar, int xbins, float xmin, float xmax, TString titlex, TString expName) {

  tdrstyle();
  TFile* _file0 = TFile::Open(expectedFile);
  TFile* _file1 = TFile::Open(expectedNSFile);
    
  TList* expList = contour1D(_file0, xvar, xbins, xmin, xmax);
  TList* expNSList = contour1D(_file1, xvar, xbins, xmin, xmax);
 
  TCanvas* expected_canvas = new TCanvas("expected_canvas", "");
  TGraph* gr1d = (TGraph*) expList->At(0);
  gr1d->Draw("A");
  gr1d->GetXaxis()->SetTitle(titlex);
  gr1d->GetYaxis()->SetTitle("-2#Delta ln L");
  gr1d->Draw("AL");
  
  expList->At(1)->Draw("Z,SAME");
  expList->At(2)->Draw("Z,SAME");
  ((TGraph*) expNSList->At(0))->SetLineWidth(1);
  ((TGraph*) expNSList->At(0))->SetLineStyle(9);
  expNSList->At(0)->Draw("L SAME");

  TLatex lt_hor;
  lt_hor.SetTextFont(42);
  lt_hor.SetTextColor(1);
  lt_hor.SetTextSize(0.05);
  lt_hor.SetTextAlign(32);
  lt_hor.SetTextAngle(0);
  lt_hor.SetNDC();

  TLatex lt_hor2;
  lt_hor2.SetTextFont(42);
  lt_hor2.SetTextColor(1);
  lt_hor2.SetTextSize(0.05);
  lt_hor2.SetTextAlign(12);
  lt_hor2.SetTextAngle(0);
  lt_hor2.SetNDC();

  TPave* bbox = new TPave(0.2, 0.85, 0.55, 0.92, 0, "NDC");;
  bbox->SetFillColor(kWhite);
  bbox->Draw();
 
  lt_hor.DrawLatex(0.8, 0.96,"L=19.4 fb^{-1} (8 TeV)");
  lt_hor2.DrawLatex(0.2, 0.88, "#bf{CMS} (preliminary)");
  
  TLegend* leg = new TLegend(0.2, 0.72, 0.55, 0.83);
  leg->AddEntry(gr1d, "Stat. + Syst.", "l");
  leg->AddEntry(((TGraph*) expNSList->At(0)), "Stat. only", "l");
  leg->SetFillColor(0);
  leg->SetBorderSize(0);
  leg->Draw();

  expected_canvas->Print(expName);

}
  
void drawObserved2D(TString observedFile, TString expectedFile, TString xvar, int xbins, float xmin, float xmax, TString yvar, int ybins, float ymin, float ymax, float smx, float smy, TString titlex, TString titley, TString colorName, TString BWname) {

  tdrstyle();
  TFile* _file0 = TFile::Open(observedFile);
  TFile* _file1 = TFile::Open(expectedFile);
    
  TList* obsList = contour2D(_file0, xvar, xbins, xmin, xmax, yvar, ybins, ymin, ymax, 0.0, 0.0);
  TList* expList = contour2D(_file1, xvar, xbins, xmin, xmax, yvar, ybins, ymin, ymax, 0.0, 0.0);
  
  TH2D* h2obs = (TH2D*) obsList->At(0);
  fixTH2D(h2obs,21);
  h2obs->GetXaxis()->SetTitle(titlex);
  h2obs->GetYaxis()->SetTitle(titley);

  TCanvas* observedColor_canvas = new TCanvas("observedColor_canvas", "");
  h2obs->Draw("colz");
  obsList->At(1)->Draw("P SAME");
  obsList->At(2)->Draw("L SAME");
  obsList->At(3)->Draw("L SAME");
  ((TMarker*) obsList->At(4))->DrawMarker(smx,smy);
  ((TMarker*) obsList->At(5))->DrawMarker(smx,smy);

  TLatex lt_ver;
  lt_ver.SetTextFont(42);
  lt_ver.SetTextColor(1);
  lt_ver.SetTextSize(0.06);
  lt_ver.SetTextAlign(12);
  lt_ver.SetTextAngle(90);
  lt_ver.SetNDC();

  TLatex lt_hor;
  lt_hor.SetTextFont(42);
  lt_hor.SetTextColor(1);
  lt_hor.SetTextSize(0.05);
  lt_hor.SetTextAlign(32);
  lt_hor.SetTextAngle(0);
  lt_hor.SetNDC();

  TLatex lt_hor2;
  lt_hor2.SetTextFont(42);
  lt_hor2.SetTextColor(1);
  lt_hor2.SetTextSize(0.05);
  lt_hor2.SetTextAlign(12);
  lt_hor2.SetTextAngle(0);
  lt_hor2.SetNDC();

  lt_ver.DrawLatex(0.95, 0.75,"-2#Delta ln L");
  lt_hor.DrawLatex(0.8, 0.96,"L=19.4 fb^{-1} (8 TeV)");
  lt_hor2.DrawLatex(0.2, 0.88, "#bf{CMS} (preliminary)");

  observedColor_canvas->Print(colorName);

  TCanvas* observedBW_canvas = new TCanvas("observedBW_canvas", "");
  h2obs->Draw("axis");
  obsList->At(1)->Draw("P SAME");
  obsList->At(2)->Draw("L SAME");
  ((TGraph*) ((TList*) expList->At(2))->At(0))->SetLineWidth(1);
  expList->At(2)->Draw("L SAME");
  obsList->At(3)->Draw("L SAME");
  ((TGraph*) ((TList*) expList->At(3))->At(0))->SetLineWidth(1);
  expList->At(3)->Draw("L SAME");
  obsList->At(1)->Draw("P SAME");
  ((TMarker*) obsList->At(4))->DrawMarker(smx,smy);
  ((TMarker*) obsList->At(5))->DrawMarker(smx,smy);
  lt_hor.DrawLatex(0.8, 0.96,"L=19.4 fb^{-1} (8 TeV)");
  lt_hor2.DrawLatex(0.2, 0.88, "#bf{CMS} (preliminary)");
  
  TLegend* leg = new TLegend(0.2, 0.75, 0.6, 0.85);
  leg->AddEntry(((TGraph*) ((TList*) obsList->At(2))->At(0)), "Observed 68\% CL", "l");
  leg->AddEntry(((TGraph*) ((TList*) obsList->At(3))->At(0)), "Observed 95\% CL", "l");
  leg->SetFillColor(0);
  leg->SetBorderSize(0);
  leg->Draw();

  observedBW_canvas->Print(BWname);

}
  
void drawObserved1D(TString observedFile, TString expectedFile, TString xvar, int xbins, float xmin, float xmax, TString titlex, TString name) {

  tdrstyle();
  TFile* _file0 = TFile::Open(observedFile);
  TFile* _file1 = TFile::Open(expectedFile);
    
  TList* obsList = contour1D(_file0, xvar, xbins, xmin, xmax);
  TList* expList = contour1D(_file1, xvar, xbins, xmin, xmax);

  cout << "Summary for " << observedFile.Data() << ", variable: "  << titlex.Data() << endl;
  cout << "Expected " << ((TGraph*) expList->At(3))->GetX()[0] << " " << ((TGraph*) ((TList*) expList->At(1))->At(0))->GetX()[0] << " " << ((TGraph*) ((TList*) expList->At(1))->At(0))->GetX()[1] << " " << ((TGraph*) ((TList*) expList->At(2))->At(0))->GetX()[0] << " " << ((TGraph*) ((TList*) expList->At(2))->At(0))->GetX()[1] << endl;
  cout << "Observed " << ((TGraph*) obsList->At(3))->GetX()[0] << " " << ((TGraph*) ((TList*) obsList->At(1))->At(0))->GetX()[0] << " " << ((TGraph*) ((TList*) obsList->At(1))->At(0))->GetX()[1] << " " << ((TGraph*) ((TList*) obsList->At(2))->At(0))->GetX()[0] << " " << ((TGraph*) ((TList*) obsList->At(2))->At(0))->GetX()[1] << endl;
 
  TCanvas* observed_canvas = new TCanvas("observed_canvas", "");
  TGraph* gr1d = (TGraph*) obsList->At(0);
  gr1d->Draw("A");
  gr1d->GetXaxis()->SetTitle(titlex);
  gr1d->GetYaxis()->SetTitle("-2#Delta ln L");
  gr1d->Draw("AL");
  
  obsList->At(1)->Draw("Z,SAME");
  obsList->At(2)->Draw("Z,SAME");
  ((TGraph*) expList->At(0))->SetLineWidth(1);
  ((TGraph*) expList->At(0))->SetLineStyle(9);
  expList->At(0)->Draw("L SAME");

  TLatex lt_hor;
  lt_hor.SetTextFont(42);
  lt_hor.SetTextColor(1);
  lt_hor.SetTextSize(0.05);
  lt_hor.SetTextAlign(32);
  lt_hor.SetTextAngle(0);
  lt_hor.SetNDC();

  TLatex lt_hor2;
  lt_hor2.SetTextFont(42);
  lt_hor2.SetTextColor(1);
  lt_hor2.SetTextSize(0.05);
  lt_hor2.SetTextAlign(12);
  lt_hor2.SetTextAngle(0);
  lt_hor2.SetNDC();

  TPave* bbox = new TPave(0.2, 0.85, 0.55, 0.92, 0, "NDC");;
  bbox->SetFillColor(kWhite);
  bbox->Draw();
 
  lt_hor.DrawLatex(0.8, 0.96,"L=19.4 fb^{-1} (8 TeV)");
  lt_hor2.DrawLatex(0.2, 0.88, "#bf{CMS} (preliminary)");
  
  TLegend* leg = new TLegend(0.2, 0.72, 0.55, 0.83);
  leg->AddEntry(gr1d, "Observed", "l");
  leg->AddEntry(((TGraph*) expList->At(0)), "Expected", "l");
  leg->SetFillColor(0);
  leg->SetBorderSize(0);
  leg->Draw();

  observed_canvas->Print(name);

}

void fixOverlay() {
  gPad->RedrawAxis();
}

void setTDRStyle(bool force) {
//   TStyle *tdrStyle = new TStyle("tdrStyle","Style for P-TDR");

// For the canvas:
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasColor(kWhite);
  gStyle->SetCanvasDefH(600); //Height of canvas
  gStyle->SetCanvasDefW(700); //Width of canvas
  gStyle->SetCanvasDefX(0);   //POsition on screen
  gStyle->SetCanvasDefY(0);

// For the Pad:
  gStyle->SetPadBorderMode(0);
  gStyle->SetPadColor(kWhite);
  gStyle->SetPadGridX(false);
  gStyle->SetPadGridY(false);
  gStyle->SetGridColor(0);
  gStyle->SetGridStyle(3);
  gStyle->SetGridWidth(1);

// For the frame:
  gStyle->SetFrameBorderMode(0);
  gStyle->SetFrameBorderSize(1);
  gStyle->SetFrameFillColor(0);
  gStyle->SetFrameFillStyle(0);
  gStyle->SetFrameLineColor(1);
  gStyle->SetFrameLineStyle(1);
  gStyle->SetFrameLineWidth(1);

// For the histo:
  if (force) {
      gStyle->SetHistLineColor(1);
      gStyle->SetHistLineStyle(0);
      gStyle->SetHistLineWidth(1);
  }

  gStyle->SetEndErrorSize(2);
  //gStyle->SetErrorMarker(20);
  gStyle->SetErrorX(0.);
  
  gStyle->SetMarkerStyle(20);

//For the fit/function:
  gStyle->SetOptFit(1);
  gStyle->SetFitFormat("5.4g");
  gStyle->SetFuncColor(2);
  gStyle->SetFuncStyle(1);
  gStyle->SetFuncWidth(1);

//For the date:
  gStyle->SetOptDate(0);

// For the statistics box:
  gStyle->SetOptFile(0);
  gStyle->SetOptStat("mr");
  gStyle->SetStatColor(kWhite);
  gStyle->SetStatFont(42);
  gStyle->SetStatFontSize(0.04);///---> gStyle->SetStatFontSize(0.025);
  gStyle->SetStatTextColor(1);
  gStyle->SetStatFormat("6.4g");
  gStyle->SetStatBorderSize(1);
  gStyle->SetStatH(0.1);
  gStyle->SetStatW(0.2);///---> gStyle->SetStatW(0.15);

// Margins:
  gStyle->SetPadTopMargin(0.07);
  gStyle->SetPadBottomMargin(0.13);
  gStyle->SetPadLeftMargin(0.16);
  gStyle->SetPadRightMargin(0.20);

// For the Global title:

  gStyle->SetOptTitle(0);
  gStyle->SetTitleFont(42);
  gStyle->SetTitleColor(1);
  gStyle->SetTitleTextColor(1);
  gStyle->SetTitleFillColor(10);
  gStyle->SetTitleFontSize(0.05);

// For the axis titles:

  gStyle->SetTitleColor(1, "XYZ");
  gStyle->SetTitleFont(42, "XYZ");
  gStyle->SetTitleSize(0.06, "XYZ");
  gStyle->SetTitleXOffset(0.9);
  gStyle->SetTitleYOffset(1.25);

// For the axis labels:

  gStyle->SetLabelColor(1, "XYZ");
  gStyle->SetLabelFont(42, "XYZ");
  gStyle->SetLabelOffset(0.007, "XYZ");
  gStyle->SetLabelSize(0.05, "XYZ");

// For the axis:

  gStyle->SetAxisColor(1, "XYZ");
  gStyle->SetStripDecimals(kTRUE);
  gStyle->SetTickLength(0.03, "XYZ");
  gStyle->SetNdivisions(510, "XYZ");
  gStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
  gStyle->SetPadTickY(1);

// Change for log plots:
  gStyle->SetOptLogx(0);
  gStyle->SetOptLogy(0);
  gStyle->SetOptLogz(0);

// Postscript options:
  gStyle->SetPaperSize(20.,20.);
  gROOT->ForceStyle();

}

void tdrstyle(bool force=true) {
    setTDRStyle(force);
}


void fixTH2D(TH2D* in, double max) {

  for (int ix=1; ix<=in->GetNbinsX(); ix++) {
    bool done = false;
    for (int iy=in->GetNbinsY()/2+1; iy<=in->GetNbinsY(); iy++) {
      if (in->GetBinContent(ix,iy) > 21)
	done = true;
      if (done)
	in->SetBinContent(ix,iy, 999);
    }
    done = false;
    for (int iy=in->GetNbinsY()/2; iy>=1; iy--) {
      if (in->GetBinContent(ix,iy) > 21)
	done = true;
      if (done)
	in->SetBinContent(ix,iy, 999);
    }
  }

}
