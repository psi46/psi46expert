
// ----------------------------------------------------------------------
// Usage in interactive root on kamor:
// .L prodSummary.C
// prodSummary a
// 
// -> It will produce printout and plots 
//
// CVS: $Author: l_tester $ $Date: 2007/07/26 16:57:22 $ $Revision: 1.6 $
// ----------------------------------------------------------------------


class prodSummary {
public:
  prodSummary(const char *filename = "/home/l_tester/ptr/moduleDB/book-keeping/module-tests.dat", 
	      const char *filename2 = "/home/l_tester/ptr/moduleDB/book-keeping/half-modules.dat", 
	      const char *title = "Tested Modules in 2006/2007 (incl. half-modules)");
  
  differential();
  runningSum();

  // ----------------------------------------------------------------------
  TCanvas *c;
  TH1D *hA, *hB, *hC, *hD;
  TString fFileName, fFileName2, fTitle, fDate;

  int fLastWeek;
  
};



// ----------------------------------------------------------------------
prodSummary::prodSummary(const char *filename, const char *filename2, const char *title) {

  // --- Cleanup if this is not the first call to rootlogon.C
  c = (TCanvas*)gROOT->FindObject("c0"); 
  if (c) delete c; 
  // --- Create a new canvas.
  c = new TCanvas("c0","--c0--",615,0,656,700);
  c->ToggleEventStatus();

  gStyle->SetStatFont(132); 
  gStyle->SetTextFont(132); 
  gStyle->SetLabelFont(132, "X"); 
  gStyle->SetLabelFont(132, "Y"); 
  gStyle->SetTitleFont(132); 
  gROOT->ForceStyle();

  // -- Date and time when plots are produced
  TDatime adate;
  fDate = TString(adate.AsString());

  fFileName = TString(filename);
  fFileName2 = TString(filename2);
  fTitle    = TString(title);

  int nbins = 104; 
  hA = new TH1D("A", "A modules", nbins, 0., nbins);  
  hA->SetLineColor(kBlue); 
  hA->SetFillColor(kBlue);
  hA->SetFillStyle(1000);

  hB = new TH1D("B", "B modules", nbins, 0., nbins);  
  hB->SetLineColor(kBlack); 
  hB->SetFillColor(kBlack);
  hB->SetFillStyle(1000);

  hC = new TH1D("C", "C modules", nbins, 0., nbins);  
  hC->SetLineColor(kRed); 
  hC->SetFillColor(kRed);
  hC->SetFillStyle(1000);

  hD = new TH1D("D", "modules w/o grading", nbins, 0., nbins); 
  hD->SetLineColor(kMagenta); 
  hD->SetFillColor(kMagenta);
  hD->SetFillStyle(1000);

  gPad->SetLeftMargin(0.15);
  gPad->SetBottomMargin(0.16);
  parse();
  differential();
  runningSum();

}


// ----------------------------------------------------------------------
void prodSummary::differential() {
  
  int total;
  for (int i = 0; i < hA->GetNbinsX(); ++i) {
    total = hA->GetBinContent(i) + hB->GetBinContent(i) + hC->GetBinContent(i) + hD->GetBinContent(i);
    if (total > 0) {
      cout << Form("Week %02d: A %02d B %02d C %02d ? %02d", 
		   i, 
		   (int)hA->GetBinContent(i), 
		   (int)hB->GetBinContent(i), 
		   (int)hC->GetBinContent(i), 
		   (int)hD->GetBinContent(i)) 
	   << endl;
      fLastWeek = i;
    }
    
  }
  


  THStack *hs = new THStack("hS", fTitle);
  hs->Add(hA);
  hs->Add(hB);
  hs->Add(hC);
  hs->Add(hD);

  hs->Draw();

  for (int bin = 0; bin < hs->GetXaxis()->GetNbins(); bin++){

    if ( bin == 1 || bin == 53)  { hs->GetXaxis()->SetBinLabel(bin,"Jan"); }
    if ( bin == 14 || bin == 66) { hs->GetXaxis()->SetBinLabel(bin,"Apr"); }
    if ( bin == 27 || bin == 79) { hs->GetXaxis()->SetBinLabel(bin,"Jul"); }
    if ( bin == 40 || bin == 92) { hs->GetXaxis()->SetBinLabel(bin,"Oct"); }
    //    if ( bin == 48 || bin == 100){ rA->GetXaxis()->SetBinLabel(bin,"Dec"); }
  }

  hS->GetYaxis()->SetTitle("# modules (total)");
  hS->GetYaxis()->SetTitleFont(132);
  hS->GetYaxis()->SetTitleOffset(1.5);
  hS->GetXaxis()->SetNdivisions(512);
  hS->GetXaxis()->SetTitle("week");

  TLegend *leg = new TLegend(0.154, 0.65, 0.37, 0.895);
  TLegendEntry *legge;
  leg->SetHeader("Grade");
  leg->SetFillStyle(1000); leg->SetBorderSize(1.); leg->SetTextSize(0.05);  leg->SetFillColor(0); 
  legge = leg->AddEntry(hD, Form("n/a"), "f"); 
  legge = leg->AddEntry(hC, Form("C"), "f"); 
  legge = leg->AddEntry(hB, Form("B"), "f"); 
  legge = leg->AddEntry(hA, Form("A"), "f"); 

  leg->Draw();
  gPad->SetGridx(1);
  gPad->SetGridy(1);

  TLatex *tl = new TLatex();
  tl->SetTextSize(0.02);
  tl->SetNDC(kTRUE);
  tl->DrawText(0.15, 0.905, fFileName);

  tl->DrawText(0.66, 0.905, fDate.Data());

  tl->SetTextSize(0.05);
  tl->DrawText(0.25, 0.05, "2006");
  tl->DrawText(0.60, 0.05, "2007");

  c->SaveAs("prodSummary-1.pdf");
  c->SaveAs("prodSummary-1.ps");
  
}


// ----------------------------------------------------------------------
void prodSummary::runningSum() {
  TH1D *rA = new TH1D(*hA);  rA->SetName("rA"); 
  rA->SetTitle(""); rA->Reset(); rA->SetFillStyle(0); rA->SetLineStyle(kSolid); rA->SetLineWidth(2);
  TH1D *rB = new TH1D(*hB);  rB->SetName("rB"); 
  rB->SetTitle(""); rB->Reset(); rB->SetFillStyle(0); rB->SetLineStyle(kDashed); rB->SetLineWidth(2);
  TH1D *rC = new TH1D(*hC);  rC->SetName("rC"); 
  rC->SetTitle(""); rC->Reset(); rC->SetFillStyle(0); rC->SetLineStyle(kDotted); rC->SetLineWidth(2);
  TH1D *rD = new TH1D(*hD);  rD->SetName("rD"); 
  rD->SetTitle(""); rD->Reset(); rD->SetFillStyle(0); rD->SetLineStyle(kSolid); rD->SetLineWidth(2);

  for (int i = 0; i < hA->GetNbinsX(); ++i) {
    if (i <= fLastWeek) {
      rA->SetBinContent(i, hA->Integral(0, i));
      rB->SetBinContent(i, hB->Integral(0, i));
      rC->SetBinContent(i, hC->Integral(0, i));
      rD->SetBinContent(i, hD->Integral(0, i));
    } else {
      rA->SetBinContent(i, 0);
      rB->SetBinContent(i, 0);
      rC->SetBinContent(i, 0);
      rD->SetBinContent(i, 0);
    }    
  }


  gStyle->SetOptStat(0);
  rA->SetTitle(fTitle.Data());
  

  for (int bin = 0; bin < rA->GetNbinsX(); bin++){

    if ( bin == 1 || bin == 53)  { rA->GetXaxis()->SetBinLabel(bin,"Jan"); }
    if ( bin == 14 || bin == 66) { rA->GetXaxis()->SetBinLabel(bin,"Apr"); }
    if ( bin == 27 || bin == 79) { rA->GetXaxis()->SetBinLabel(bin,"Jul"); }
    if ( bin == 40 || bin == 92) { rA->GetXaxis()->SetBinLabel(bin,"Oct"); }
    //    if ( bin == 48 || bin == 100){ rA->GetXaxis()->SetBinLabel(bin,"Dec"); }
  }

  rA->Draw();
  rB->Draw("same");
  rC->Draw("same");
  rD->Draw("same");

  rA->GetYaxis()->SetNdivisions(515);
  rA->GetYaxis()->SetTitle("# modules (per grade)");
  rA->GetYaxis()->SetTitleFont(132);
  rA->GetYaxis()->SetTitleOffset(1.7);
  rA->GetXaxis()->SetNdivisions(512);
  rA->GetXaxis()->SetTitle("week");

  TLegend *leg = new TLegend(0.154, 0.65, 0.37, 0.895);
  TLegendEntry *legge;
  leg->SetHeader("Grade");
  leg->SetFillStyle(1000); leg->SetBorderSize(1.); leg->SetTextSize(0.05);  leg->SetFillColor(0); 
  legge = leg->AddEntry(rA, Form("A"), "l"); 
  legge = leg->AddEntry(rB, Form("B"), "l"); 
  legge = leg->AddEntry(rC, Form("C"), "l"); 
  legge = leg->AddEntry(rD, Form("n/a"), "l"); 

  leg->Draw();
  gPad->SetGridx(1);
  gPad->SetGridy(1);

  TLatex *tl = new TLatex();
  tl->SetTextSize(0.02);
  tl->SetNDC(kTRUE);
  tl->DrawText(0.15, 0.905, fFileName);

  tl->DrawText(0.68, 0.905, fDate.Data());
  tl->SetTextSize(0.05);
  tl->DrawText(0.25, 0.05, "2006");
  tl->DrawText(0.60, 0.05, "2007");

  c->SaveAs("prodSummary-2.pdf");
  c->SaveAs("prodSummary-2.ps");
}



// ----------------------------------------------------------------------
void prodSummary::parse() {
  char buffer[200];
  ifstream is(fFileName);
  ifstream is2(fFileName2);
  int mod1, mod2, year, month, day, hour, min;
  int week;
  int grade; 
  char grade1, grade2;
  
  while (is.getline(buffer, 200, '\n')) {
    day = -1; 
    sscanf(buffer, "M%d %c M%d-%2d%2d%2d.%d:%d %c", 
	   &mod1, &grade1, &mod2, &year, &month, &day, &hour, &min, &grade2);
    year += 2000;
    //     cout << buffer << endl;
    //     cout << "Module " << mod1 << " graded " << grade1 << " on date " 
    //      	 << Form("%02d %02d %02d", year, month, day) << endl;
    
    if (day > 0) {
      week = weeknumber(year, month, day);
      week += (year-2006)*52;
    }
   

    grade = grade1; 
    if (grade == 'A') {
      hA->Fill(week);
    } elseif (grade == 'B') {
      hB->Fill(week);
    } elseif (grade == 'C') {
      hC->Fill(week);
    } elseif (grade == 'T') {
      hD->Fill(week);
    }      
    
  }
  is.close();
  
  while (is2.getline(buffer, 200, '\n')) {
    day = -1; 
    sscanf(buffer, "M%d %c M%d-%2d%2d%2d.%d:%d %c", 
	   &mod1, &grade1, &mod2, &year, &month, &day, &hour, &min, &grade2);
    year += 2000;
    //     cout << buffer << endl;
    //     cout << "Module " << mod1 << " graded " << grade1 << " on date " 
    //      	 << Form("%02d %02d %02d", year, month, day) << endl;
    
    if (day > 0) {
      week = weeknumber(year, month, day);
      week += (year-2006)*52;
    }
   

    grade = grade1; 
    if (grade == 'A') {
      hA->Fill(week);
    } elseif (grade == 'B') {
      hB->Fill(week);
    } elseif (grade == 'C') {
      hC->Fill(week);
    } elseif (grade == 'T') {
      hD->Fill(week);
    }      
    
  }
  is2.close();
  
  
}



/*
 * From http://ce.et.tudelft.nl/~knop/weekno.c
 * weeknumber:		Compute week number from the date.
 * arguments:
 * int year:		Four digit year number.
 * int month:		Two digit month number.
 * int day:		Number of day of the month.
 * returns:		Int, the weeknumber.
 * remark:		If in month 1 the returned week number is 52 or 53,
 *			the day belonged to the last week of the previous year
 *			(the year-number should be decremented).
 *			If in month 12 the returned week number is 1, the day
 *			belonged to the first week of the next year (the year-
 *			number should be incremented).
 */
int weeknumber(int year, int month, int day) {
  long daynumber;
  long firstdaynumber;
  long lastdaynumber;
  /*
   * This code is incorrect for dates before March 1 1900 or after
   * February 28 2100 (or some such range)...
   * This is not considered a bug.
   */
  daynumber = 4L + 365L * ((long) year) +
    31L * (((long) month) - 1L) +
    ((long) ((year - 1) / 4)) +
    (long) (day);
  if((month > 2) && (year % 4 == 0))
    daynumber++;			/* after leapday */
  if((month > 2) && (month < 8))
    daynumber -= (3L + ((month - 3) / 2));
  else if(month > 7)
    daynumber -= (3L + ((month - 4) / 2));
  firstdaynumber = 5L + 365L * ((long) year) +
    ((long) ((year - 1) / 4));
  lastdaynumber = 4L + 365L * ((long) (year + 1)) +
    ((long) ((year) / 4));
  
  if(((lastdaynumber % 7L) <= 2L) &&
     ((lastdaynumber - daynumber) <= (lastdaynumber % 7L)))
    return (1);/* wrap to 1st week next year */
  else if((firstdaynumber % 7L) <= 3L)/* This year starts Mo..Th */
    return ((daynumber - firstdaynumber / 7L * 7L) / 7L + 1L);
  else if(daynumber < (firstdaynumber + 7L - firstdaynumber % 7L))
    {
      /*
       * Day daynumber belongs to last week of previous year.
       */
      firstdaynumber = 4L + 365L * ((long) (year - 1)) +
	((long) ((year - 1) / 4)) + 1L;
      if((firstdaynumber % 7L) <= 3L)/* Previous year starts Mo..Th */
	return ((daynumber - firstdaynumber / 7L * 7L) / 7L + 1L);
      else
	return ((daynumber - firstdaynumber / 7L * 7L) / 7L);
    }
  else
    return ((daynumber - firstdaynumber / 7L * 7L) / 7L);
}

// ----------------------------------------------------------------------
void prodSummary::shrinkPad(double b, double l, double r=0.1, double t=0.1) {
  gPad->SetBottomMargin(b);
  gPad->SetLeftMargin(l);
  gPad->SetRightMargin(r);
  gPad->SetTopMargin(t);
}
