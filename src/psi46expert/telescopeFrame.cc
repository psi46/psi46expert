#include <cstdlib>
#include <string>
#include <fstream>
#include <iomanip>
#include "telescopeFrame.h"

#define DATA_HDR 46029
#define DATA_TRL 19506

#define NSTRIPS 512

using namespace std;

ClassImp(CTelescopeFrame)

CTelescopeFrame::CTelescopeFrame(const TGWindow* p, const char * dataDir) : TGMainFrame( p, 700,700)
{
  
 fp = 0;

 SetLayoutBroken(kTRUE);

  // -- Colors
  gClient->GetColorByName("white", colWhite);
  gClient->GetColorByName("pink", colPink);
  gClient->GetColorByName("black", colBlack);
  gClient->GetColorByName("cyan", colCyan);
  gClient->GetColorByName("green", colGreen);
  gClient->GetColorByName("red", colRed);
  gClient->GetColorByName("blue", colBlue);
  gClient->GetColorByName("LightBlue", colLightBlue);
  gClient->GetColorByName("DarkOliveGreen", colDarkOliveGreen);
  gClient->GetColorByName("DarkSeaGreen", colDarkSeaGreen);

  SetBackgroundColor(colDarkSeaGreen);

  // ----------------------------------------------------------------------
  // -- Frame for Run Control
  TGHorizontalFrame *wRunCtrl = new TGHorizontalFrame(this, 700, 210, kHorizontalFrame);
  AddFrame(wRunCtrl);
  wRunCtrl->SetBackgroundColor(colDarkOliveGreen);
  wRunCtrl->DrawBorder();


  fwRunNumber = new TGTextEntry(wRunCtrl, fRunTextBuffer = new TGTextBuffer(30));
  TGLabel *wRunNumberLabel = new TGLabel(wRunCtrl, "Run: ");
  wRunNumberLabel->MoveResize(5, 120, 50, fwRunNumber->GetDefaultHeight());
  wRunCtrl->AddFrame(wRunNumberLabel);

  fwRunNumber->MoveResize(70, 120, 50, fwRunNumber->GetDefaultHeight());
  //fwRunNumber->Connect("ReturnPressed()", "telescopeFrame", this, "doRunNumberUpdate()");
  wRunCtrl->AddFrame(fwRunNumber);

  // -- MemSize
  fwMemSize = new TGTextEntry(wRunCtrl, fwMemSizeBuffer = new TGTextBuffer(10));
  TGLabel *wMemSizeLabel = new TGLabel(wRunCtrl, "MemSize:");
  wMemSizeLabel->MoveResize(5, 80, 65, fwMemSize->GetDefaultHeight());
  wRunCtrl->AddFrame(wMemSizeLabel);

  //fwDuration->SetText(Form("%i", fRunDuration));
  fwMemSize->MoveResize(80, 80, 65, fwMemSize->GetDefaultHeight());
  fwMemSizeBuffer->AddText(0,"4000000");
  setMemSize();
  fwMemSize->Connect("ReturnPressed()", "CTelescopeFrame", this, "setMemSize()");
  wRunCtrl->AddFrame(fwMemSize);

 // -- SetClock
  fwSetClock = new TGTextEntry(wRunCtrl, fwSetClockBuffer = new TGTextBuffer(10));
  TGLabel *wSetClockLabel = new TGLabel(wRunCtrl, "SetClock:");
  wSetClockLabel->MoveResize(5, 20, 60, fwSetClock->GetDefaultHeight());
  wRunCtrl->AddFrame(wSetClockLabel);

  fwSetClock->MoveResize(80, 20, 30, fwSetClock->GetDefaultHeight());
  fwSetClock->Connect("ReturnPressed()", "CTelescopeFrame", this, "setClock()");
  wRunCtrl->AddFrame(fwSetClock);

  // -- SetCalDelay
  fwSetDelay = new TGTextEntry(wRunCtrl, fDelayBuffer = new TGTextBuffer(10));
  TGLabel *wSetDelayLabel = new TGLabel(wRunCtrl, "Delay:");
  wSetDelayLabel->MoveResize(540, 50, 40, fwSetDelay->GetDefaultHeight());
  wRunCtrl->AddFrame(wSetDelayLabel);

  fwSetDelay->MoveResize(590, 50, 30, fwSetDelay->GetDefaultHeight());
  wRunCtrl->AddFrame(fwSetDelay);

// -- SetTimingDelay
  fwSetTimeDelay = new TGTextEntry(wRunCtrl, fTimeDelayBuffer = new TGTextBuffer(10));
  TGLabel *wSetTimeDelayLabel = new TGLabel(wRunCtrl, "Latch Delay:");
  wSetTimeDelayLabel->MoveResize(510, 80, 70, fwSetTimeDelay->GetDefaultHeight());
  wRunCtrl->AddFrame(wSetTimeDelayLabel);

  fwSetTimeDelay->MoveResize(590, 80, 30, fwSetTimeDelay->GetDefaultHeight());
  fwSetTimeDelay->Connect("ReturnPressed()", "telescopeFrame", this, "setLatchDelay()");
  wRunCtrl->AddFrame(fwSetTimeDelay);

  // -- Directory for output
  fwOutputDir = new TGTextEntry(wRunCtrl, fwOutputDirBuffer = new TGTextBuffer(100));
  fwOutputDirBuffer->AddText(0, dataDir);
  setPath();
  TGLabel *wOutputDirLabel = new TGLabel(wRunCtrl, "Directory:");
  wOutputDirLabel->MoveResize(150, 80, 55, fwOutputDir->GetDefaultHeight());

  fwOutputDir->MoveResize(210, 80, 280, fwOutputDir->GetDefaultHeight());
  fwOutputDir->Connect("ReturnPressed()", "CTelescopeFrame", this, "setPath()");
  wRunCtrl->AddFrame(fwOutputDir);

  //Pedestal

  fwPedestalFile = new TGTextEntry(wRunCtrl, fwPedestalFileBuffer = new TGTextBuffer(100));
  string defaultPedestalFile(dataDir);
  defaultPedestalFile.append("pedestal.root");
  fwPedestalFileBuffer->AddText(0, defaultPedestalFile.c_str());
  setPedestal();
  TGLabel *wPedestalFileLabel = new TGLabel(wRunCtrl, "Pedestal:");
  wPedestalFileLabel->MoveResize(150, 110, 55, fwPedestalFile->GetDefaultHeight());

  fwPedestalFile->MoveResize(210, 110, 280, fwPedestalFile->GetDefaultHeight());
  fwPedestalFile->Connect("ReturnPressed()", "CTelescopeFrame", this, "setPedestal()");
  wRunCtrl->AddFrame(fwPedestalFile);
  

  // -- Run Control Buttons
  TGTextButton *wStart = new TGTextButton(wRunCtrl,"Start");
  wStart->Connect("Clicked()", "CTelescopeFrame", this, "doStart()");
  wRunCtrl->AddFrame(wStart);
  wStart->MoveResize(130,20,50,20);
  wStart->ChangeBackground(colGreen);

  TGTextButton *wStop = new TGTextButton(wRunCtrl,"Stop");
  wStop->MoveResize(200, 20, 50, 20);
  wStop->Connect("Clicked()", "CTelescopeFrame", this, "doStop()");
  wStop->ChangeBackground(colRed);
  wRunCtrl->AddFrame(wStop);

  TGTextButton *wExit = new TGTextButton(wRunCtrl,"Exit");
  wExit->MoveResize(270, 20, 50, 20);
  wExit->Connect("Clicked()", "CTelescopeFrame", this, "doExit()");
  wExit->ChangeBackground(colWhite);
  wRunCtrl->AddFrame(wExit);

  TGTextButton *wClear = new TGTextButton(wRunCtrl,"ClearRam");
  wClear->MoveResize(130, 50, 50, 20);
  wClear->Connect("Clicked()", "CTelescopeFrame", this, "doClear()");
  wClear->ChangeBackground(colCyan);
  wRunCtrl->AddFrame(wClear);

  TGTextButton *wRaw = new TGTextButton(wRunCtrl,"GetRaw");
  wRaw->MoveResize(200, 50, 50, 20);
  wRaw->Connect("Clicked()", "CTelescopeFrame", this, "doGetRaw()");
  wRaw->ChangeBackground(colLightBlue);
  wRunCtrl->AddFrame(wRaw);

  TGTextButton *wPon = new TGTextButton(wRunCtrl,"Pon");
  wPon->MoveResize(270, 50, 50, 20);
  wPon->Connect("Clicked()", "CTelescopeFrame", this, "doPon()");
  wPon->ChangeBackground(colLightBlue);
  wRunCtrl->AddFrame(wPon);

  TGTextButton *wCalStart = new TGTextButton(wRunCtrl,"CalStart");
  wCalStart->MoveResize(330, 50, 50, 20);
  wCalStart->Connect("Clicked()", "CTelescopeFrame", this, "doCalStart()");
  wCalStart->ChangeBackground(colLightBlue);
  wRunCtrl->AddFrame(wCalStart);

  // Event number
  fwEvtNumber = new TGTextEntry(wRunCtrl, fEvtTextBuffer = new TGTextBuffer(30));  
  TGLabel *wEvtlabel = new TGLabel(wRunCtrl, "Event:");
  wEvtlabel->MoveResize(20, 150, 80, fwEvtNumber->GetDefaultHeight());
  wRunCtrl->AddFrame(wEvtlabel);

  fwEvtNumber->MoveResize(130, 150, 90, fwEvtNumber->GetDefaultHeight());
  //fwEvtNumber->Connect("ReturnPressed()", "CTelescopeFrame", this, "setEvent()");
  wRunCtrl->AddFrame(fwEvtNumber);

  // Analog & Dig

  fwVA = new TGTextEntry(wRunCtrl, fVABuffer = new TGTextBuffer(10));
  TGLabel *wVAlabel = new TGLabel(wRunCtrl, "VA:");
  wVAlabel->MoveResize(300, 140, 30, fwVA->GetDefaultHeight());
  wRunCtrl->AddFrame(wVAlabel);

  fwVA->MoveResize(340, 140, 50, fwVA->GetDefaultHeight());
  fwVA->Connect("ReturnPressed()", "CTelescopeFrame", this, "setVA()");
  //fwVA->SetEnabled(kFALSE);
  wRunCtrl->AddFrame(fwVA);

  fwIA = new TGTextEntry(wRunCtrl, fIABuffer = new TGTextBuffer(10));
  TGLabel *wIAlabel = new TGLabel(wRunCtrl, "IA:");
  wIAlabel->MoveResize(300, 170, 30, fwIA->GetDefaultHeight());
  wRunCtrl->AddFrame(wIAlabel);

  fwIA->MoveResize(340, 170, 50, fwIA->GetDefaultHeight());
  fwIA->SetEnabled(kFALSE);
  wRunCtrl->AddFrame(fwIA);

  fwVD = new TGTextEntry(wRunCtrl, fVDBuffer = new TGTextBuffer(10));
  TGLabel *wVDlabel = new TGLabel(wRunCtrl, "VD:");
  wVDlabel->MoveResize(400, 140, 30, fwVD->GetDefaultHeight());
  wRunCtrl->AddFrame(wVDlabel);

  fwVD->MoveResize(440, 140, 50, fwVD->GetDefaultHeight());
  fwVD->Connect("ReturnPressed()", "CTelescopeFrame", this, "setVD()");
  //fwVD->SetEnabled(kFALSE);
  wRunCtrl->AddFrame(fwVD);

  fwID = new TGTextEntry(wRunCtrl, fIDBuffer = new TGTextBuffer(10));
  TGLabel *wIDlabel = new TGLabel(wRunCtrl, "ID:");
  wIDlabel->MoveResize(400, 170, 30, fwID->GetDefaultHeight());
  wRunCtrl->AddFrame(wIDlabel);

  fwID->MoveResize(440, 170, 50, fwID->GetDefaultHeight());
  fwID->SetEnabled(kFALSE);
  wRunCtrl->AddFrame(fwID);


  // Status
  TGLabel *wlabel = new TGLabel(wRunCtrl, "Status:");
  wlabel->MoveResize(5, 180, 80, 20);
  wRunCtrl->AddFrame(wlabel);
  
  fwStatus7 = new TGButton(wRunCtrl);
  wRunCtrl->AddFrame(fwStatus7);
  fwStatus7->MoveResize(105,180,20,20);
  fwStatus7->ChangeBackground(colRed);
  fwStatus7->SetEnabled(kFALSE);

  fwStatus6 = new TGButton(wRunCtrl);
  wRunCtrl->AddFrame(fwStatus6);
  fwStatus6->MoveResize(135,180,20,20);
  fwStatus6->ChangeBackground(colRed);
  fwStatus6->SetEnabled(kFALSE);

  TGButton *wEvtWrtStatus = new TGButton(wRunCtrl);
  wRunCtrl->AddFrame(wEvtWrtStatus);
  wEvtWrtStatus->MoveResize(165,180,20,20);
  wEvtWrtStatus->ChangeBackground(colRed);
  wEvtWrtStatus->SetEnabled(kFALSE);

  //Trigger Select

  fwInternalTrigger = new TGButton(wRunCtrl);
  fwInternalTrigger->MoveResize(500, 20, 20, 20);
  fwInternalTrigger->Connect("Clicked()", "CTelescopeFrame", this, "doIntTrig()");
  fwInternalTrigger->ChangeBackground(colWhite);
  fwInternalTrigger->AllowStayDown(kTRUE);
  wRunCtrl->AddFrame(fwInternalTrigger);
  
  TGLabel *wTriglabel = new TGLabel(wRunCtrl, "Int. Trig.:");
  wTriglabel->MoveResize(420, 20, 70, fwInternalTrigger->GetDefaultHeight());
  wRunCtrl->AddFrame(wTriglabel);

  //Cal Pulses

  fwCalPulse = new TGButton(wRunCtrl);
  fwCalPulse->MoveResize(500, 50, 20, 20);
  fwCalPulse->Connect("Clicked()", "CTelescopeFrame", this, "doCalPulse()");
  fwCalPulse->ChangeBackground(colWhite);
  fwCalPulse->AllowStayDown(kTRUE);
  wRunCtrl->AddFrame(fwCalPulse);

  TGLabel *wCalPulselabel = new TGLabel(wRunCtrl, "Cal. Pulses:");
  wCalPulselabel->MoveResize(420, 50, 70, fwCalPulse->GetDefaultHeight());
  wRunCtrl->AddFrame(wCalPulselabel);


  //Histograms

  
  canvas1 = new TRootEmbeddedCanvas("Canvas1",this,150,150);
  canvas1->MoveResize(190,250);
  this->AddFrame(canvas1,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));
  
  canvas0 = new TRootEmbeddedCanvas("Canvas0",this,150,150);
 
  this->AddFrame(canvas0,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));
  canvas0->MoveResize(20,250);
  
  canvas2 = new TRootEmbeddedCanvas("Canvas2",this,150,150);
  canvas2->MoveResize(360,250);
  this->AddFrame(canvas2,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));
  
  canvas3 = new TRootEmbeddedCanvas("Canvas3",this,150,150);
  canvas3->MoveResize(530,250);
  this->AddFrame(canvas3,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));
  
  canvas4 = new TRootEmbeddedCanvas("Canvas4",this,150,150);
  canvas4->MoveResize(20,420);
  this->AddFrame(canvas4,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));

  canvas5 = new TRootEmbeddedCanvas("Canvas5",this,150,150);
  canvas5->MoveResize(190,420);
  this->AddFrame(canvas5,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));
  
  canvas6 = new TRootEmbeddedCanvas("Canvas6",this,150,150);
  canvas6->MoveResize(360,420);
  this->AddFrame(canvas6,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));

  canvas7 = new TRootEmbeddedCanvas("Canvas7",this,150,150);
  canvas7->MoveResize(530,420);
  this->AddFrame(canvas7,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));

  hSensor0 = new TH1F("hSensor0","Hits in Sensor0",NSTRIPS,0,NSTRIPS-1);
  hSensor1 = new TH1F("hSensor1","Hits in Sensor1",NSTRIPS,0,NSTRIPS-1);
  hSensor2 = new TH1F("hSensor2","Hits in Sensor2",NSTRIPS,0,NSTRIPS-1);
  hSensor3 = new TH1F("hSensor3","Hits in Sensor3",NSTRIPS,0,NSTRIPS-1);
  hSensor4 = new TH1F("hSensor4","Hits in Sensor4",NSTRIPS,0,NSTRIPS-1);
  hSensor5 = new TH1F("hSensor5","Hits in Sensor5",NSTRIPS,0,NSTRIPS-1);
  hSensor6 = new TH1F("hSensor6","Hits in Sensor6",NSTRIPS,0,NSTRIPS-1);
  hSensor7 = new TH1F("hSensor7","Hits in Sensor7",NSTRIPS,0,NSTRIPS-1);
 
  hSensor0->Fill(50,100);
 
  canvas0->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)",
                                   "CTelescopeFrame",this,"SelectCanvas(Int_t,Int_t,Int_t,TObject*)");
  canvas1->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)",
                                  "CTelescopeFrame",this,"SelectCanvas(Int_t,Int_t,Int_t,TObject*)");
  canvas2->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)",
                                   "CTelescopeFrame",this,"SelectCanvas(Int_t,Int_t,Int_t,TObject*)");
  canvas3->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)",
                                   "CTelescopeFrame",this,"SelectCanvas(Int_t,Int_t,Int_t,TObject*)");
  canvas4->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)",
                                   "CTelescopeFrame",this,"SelectCanvas(Int_t,Int_t,Int_t,TObject*)");
  canvas5->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)",
                                   "CTelescopeFrame",this,"SelectCanvas(Int_t,Int_t,Int_t,TObject*)");
  canvas6->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)",
                                   "CTelescopeFrame",this,"SelectCanvas(Int_t,Int_t,Int_t,TObject*)");
  canvas7->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)",
                                   "CTelescopeFrame",this,"SelectCanvas(Int_t,Int_t,Int_t,TObject*)");

  //canvas0->GetCanvas()->Connect("Clicked()", "CTelescopeFrame", this, "SelectCanvas()");
 
  canvas1->GetCanvas()->cd();
  hSensor1->Draw();
  canvas2->GetCanvas()->cd();
  hSensor2->Draw();
  canvas3->GetCanvas()->cd();
  hSensor3->Draw();
  canvas4->GetCanvas()->cd();
  hSensor4->Draw();
  canvas5->GetCanvas()->cd();
  hSensor5->Draw();
  canvas6->GetCanvas()->cd();
  hSensor6->Draw();
  canvas7->GetCanvas()->cd();
  hSensor7->Draw();
  canvas0->GetCanvas()->cd();
  hSensor0->Draw();

  SetWindowName("takeData telescope");

  MapSubwindows();
  MapWindow();
}


void CTelescopeFrame::SelectCanvas(Int_t event,Int_t i1 ,Int_t i2 ,TObject* gTQSender)
{
//   TRootEmbeddedCanvas *c = (TRootEmbeddedCanvas*)gTQSender;
   if (event==kButton1Down)
   {
	TCanvas *c1; 
	Int_t wx,wy;
	UInt_t ww,wh;
	TString name(((TCanvas*)gTQSender)->GetName());
	if(name.BeginsWith("Canvas"))
	 {
	   c1 = new TCanvas();
	   c1->cd();
	   switch((name.Remove(0,6)).Atoi())
	  {
	  case 0:
	
		hSensor0->Draw();
		break;
	  case 1:
		hSensor1->Draw();
		break;
	  case 2:
		hSensor2->Draw();
		break;
	  case 3:
		hSensor3->Draw();
		break;
	  case 4:
		hSensor4->Draw();
		break;
	  case 5:
		hSensor5->Draw();
		break;
	  case 6:
		hSensor6->Draw();
		break;
	  case 7:
		hSensor7->Draw();
		break;
		
	  }
	  c1->Update();
        } 
   }
}


void CTelescopeFrame::doClear()
{
  cout << " Clear Ram!" << endl;
  ts->ClearRam();

}

void CTelescopeFrame::LoadPedestals()
{

	TFile *tf = new TFile(fPedestals.c_str());

	hPed0 = (TH1F*)tf->Get("hSensor0"); 
	hPed1 = (TH1F*)tf->Get("hSensor1"); 
	hPed2 = (TH1F*)tf->Get("hSensor2"); 
	hPed3 = (TH1F*)tf->Get("hSensor3"); 
	hPed4 = (TH1F*)tf->Get("hSensor4"); 
	hPed5 = (TH1F*)tf->Get("hSensor5"); 
	hPed6 = (TH1F*)tf->Get("hSensor6"); 
	hPed7 = (TH1F*)tf->Get("hSensor7"); 

}

void CTelescopeFrame::doStart()
{
  //cout << " Status: " << ts->GetStatus() << endl;
  //Get Run Number

  //Load Pedestals

  LoadPedestals();
 
  SetRunNumber(getCurrentRun());
  if(fRun == -1)
  {
	cout << "DUT takeData not running!" << endl;
	//return; 
  }

  //Open File
  
  OpenData(fPath.c_str());

  //Init Tree
  InitTree();

  bRunning = true;  
  cout <<	"Start run: "<<fMemSize << endl;
  ts->StartRun(fMemSize,atoi(fEvtTextBuffer->GetString()));
  

  if(ts->GetDaqSize() != fMemSize)
	ts->StartRun(fMemSize,atoi(fEvtTextBuffer->GetString()));

  if(ts->GetDaqSize() != fMemSize)
  {
 	cout << "Can't start run!?!?" <<endl;
	return;
  }

  UShort_t buffer[fMemSize+10];
  try
  {
    while(bRunning)
    {
        cout << "\rSize " << setw(4) << ts->GetDaqSize() / (1024. * 1024.) << " MB";  
	if(ts->ReadyForReadout())
	{
		cout << endl << "I think you're ready ..." << endl;
		GetData(buffer);
		ParseData(buffer);
	}
	
	else
	{
		cout << " Status: " << ts->GetStatus();
		cout << " Daq Ready "<< ts->DaqReady();
	}
	cout << " " << flush;
	UpdateStatus();
	gSystem->ProcessEvents();	
	gSystem->Sleep(1);
    }
  }
  catch(...)
  {
	cout << "An error occured. Stopping run." << endl;
	doStop();
  }
  cout << endl;
}

int CTelescopeFrame::ParseData(UShort_t* buffer)
{
   cout << "ParseData" << endl;
   int curpos = 0;
   int indata = 0;
   int iStart = 0;
   int iCnt = 0;
   int Evt = 0 ; 
   while(curpos < fMemSize)
   {	

	switch(buffer[curpos])
  	{
		case DATA_HDR:
		  cout << "Found Hdr" << endl;
		  indata = 1;
		  Evt = buffer[curpos+1]<<16;
		  event.event = Evt+buffer[curpos+2];
		  iStart = curpos+3;
		  cout << buffer[iStart] << endl;
			
		  break;	
 
		case DATA_TRL:
                  cout << "Found Trl" << endl;
		  indata = 0;
		  if(curpos-iStart == NSTRIPS*8)
		  {
			copy(buffer+iStart,buffer+iStart+4096,event.RawData);			
		        cout << *(buffer+iStart) << endl;
			cout << event.RawData[0] << endl ;
			cout << event.RawData[1] << endl ;
		/*
			int avg1 = 0,avg2 = 0;
			for(int i = 0 ; i < 512 ; i+=2)
			{
			   if(i<30)
				{
				//cout << "ADC2["<<i<<"]:" << ConvertTwos(event.RawData[i]);
				//cout << endl << event.Sensor0[i]<<endl;
				cout << "ADC1["<<i<<"]:" << ConvertTwos(event.RawData[i+1]);
				cout << endl;
				}
			   avg1+=ConvertTwos(event.RawData[i]);
			   avg2+=ConvertTwos(event.RawData[i+1]);
			}	
		 	
		*/	
	
			cout << "Writing event: " << event.event << endl;
			//cout << "Avg. 2: " << avg1/256 << endl;
			//cout << "Avg. 1: " << avg2/256 << endl;

			if(event.event % 100 == 0) FillHistogram(buffer,iStart);
			WriteRecord();
		  }
		  else
		  {
			cout << "Number of entries: " << curpos-iStart << "  " << iStart << endl;
		        cout << "Badly formed event!!" << endl;
		  }
		  break;
	
		default:
		   break;
		
	}
	curpos++;		
   
    }	
	return 1;	
}
short CTelescopeFrame::ConvertTwos(unsigned short value)
{
   short temp = value;

   //cout << "Raw: " << value << " Conv: " << ((0x800&temp)? -(((~temp)&0x07FF)+1):(temp&0x07FF));
   //cout << endl;
   return (0x800&temp)? -(((~temp)&0x07FF)+1):(temp&0x07FF);

}

void CTelescopeFrame::FillHistogram(unsigned short *buffer,int iStart)
{
        ClearHistograms();
    
        short temp[8][NSTRIPS];
        for(int j = 0 ; j < NSTRIPS ; j+=2)
        {
		temp[4][j] = ConvertTwos(*(buffer+iStart+j));
                temp[0][j] = ConvertTwos(*(buffer+iStart+j+1));
                temp[5][j] = ConvertTwos(*(buffer+iStart+j+NSTRIPS*2));
                temp[1][j] = ConvertTwos(*(buffer+iStart+j+NSTRIPS*2+1));
                temp[6][j] = ConvertTwos(*(buffer+iStart+j+NSTRIPS*4));
                temp[2][j] = ConvertTwos(*(buffer+iStart+j+NSTRIPS*4+1));
                temp[7][j] = ConvertTwos(*(buffer+iStart+j+NSTRIPS*6));
                temp[3][j] = ConvertTwos(*(buffer+iStart+j+NSTRIPS*6+1));
        }
        for(int j = 1 ; j< NSTRIPS ; j+=2)
        {
	//	cout << "S3-"<<j<<" : "<< -j+NSTRIPS*8 << endl;
                temp[4][j] = ConvertTwos(*(buffer+iStart-j+NSTRIPS*2-1));
                temp[0][j] = ConvertTwos(*(buffer+iStart-j+NSTRIPS*2));
                temp[5][j] = ConvertTwos(*(buffer+iStart-j+NSTRIPS*4-1));
                temp[1][j] = ConvertTwos(*(buffer+iStart-j+NSTRIPS*4));
                temp[6][j] = ConvertTwos(*(buffer+iStart-j+NSTRIPS*6-1));
                temp[2][j] = ConvertTwos(*(buffer+iStart-j+NSTRIPS*6));
                temp[7][j] = ConvertTwos(*(buffer+iStart-j+NSTRIPS*8-1));
                temp[3][j] = ConvertTwos(*(buffer+iStart-j+NSTRIPS*8));

        }

        for(int i = 0 ; i < NSTRIPS ; i++)
	{
	
		hSensor0->Fill(i,temp[0][i]);
		hSensor1->Fill(i,temp[1][i]);
		hSensor2->Fill(i,temp[2][i]);
		hSensor3->Fill(i,temp[3][i]);
		hSensor4->Fill(i,temp[4][i]);
		hSensor5->Fill(i,temp[5][i]);
		hSensor6->Fill(i,temp[6][i]);
		hSensor7->Fill(i,temp[7][i]);
		
	}
	hSensor0->Add(hPed0,-1);
	hSensor1->Add(hPed1,-1);
	hSensor2->Add(hPed2,-1);
	hSensor3->Add(hPed3,-1);
	hSensor4->Add(hPed4,-1);
	hSensor5->Add(hPed5,-1);
	hSensor6->Add(hPed6,-1);
	hSensor7->Add(hPed7,-1);


	canvas0->GetCanvas()->cd();
	hSensor0->Draw();
	canvas1->GetCanvas()->cd();
	hSensor1->Draw();
	canvas2->GetCanvas()->cd();
	hSensor2->Draw();
	canvas3->GetCanvas()->cd();
	hSensor3->Draw();
	canvas4->GetCanvas()->cd();
	hSensor4->Draw();
	canvas5->GetCanvas()->cd();
	hSensor5->Draw();
	canvas6->GetCanvas()->cd();
	hSensor6->Draw();
	canvas7->GetCanvas()->cd();
	hSensor7->Draw();
	canvas0->GetCanvas()->Update();
	canvas1->GetCanvas()->Update();
	canvas2->GetCanvas()->Update();
	canvas3->GetCanvas()->Update();
	canvas4->GetCanvas()->Update();
	canvas5->GetCanvas()->Update();
	canvas6->GetCanvas()->Update();
	canvas7->GetCanvas()->Update();
}


/*
void CTelescopeFrame::SetTB(CTestboard_TS* pTB)
{
	ts = pTB;

}
*/

void CTelescopeFrame::GetData(UShort_t* buffer)
{
   ts->GetData(buffer);

}

void CTelescopeFrame::InitTree()
{

  tree = new TTree("Telescope","Event Info");
  tree->Branch("TelescopeEvent",&event,"run/I:event/I:Thresh/I:VA/F:VD/F:IA/F:ID/F:RawData[4096]/s");

}

void CTelescopeFrame::WriteRecord()
{
	event.run = fRun;
	tree->Fill();
	fwEvtNumber->SetText(Form("%i", event.event));
	
}

void CTelescopeFrame::UpdateStatus()
{
	event.VA = ts->GetVA();
	event.IA = ts->GetIA();
	event.VD = ts->GetVD();
	event.ID = ts->GetID();

	//fwVA->SetText(Form("%2.4f",event.VA));
	fwIA->SetText(Form("%2.4f",event.IA));
	//fwVD->SetText(Form("%2.4f",event.VD));
	fwID->SetText(Form("%2.4f",event.ID));

	event.Thresh = ts->GetThresh();

	int status = ts->GetStatus();
	
	fwStatus7->ChangeBackground(status&128?colRed:colGreen);
	fwStatus6->ChangeBackground(status&64?colRed:colGreen);
	
	  

}

void CTelescopeFrame::doStop()
{
   unsigned short buff[4096];
   cout << "Stop run" <<endl;
   ts->TrigPause();
   bRunning = false;
   //doClear();
   //GetData(buff);
   //ParseData(buff);
   fp->Write();
	cout << "After write." << endl;
   fp->Close();
   ts->StopRun();
   fp=0;

}
void CTelescopeFrame::doGetRaw()
{

   ts->GetRawData(fMemSize);

}

void CTelescopeFrame::doExit()
{
   
   if(fp)
	{
   	fp->Write();
     	fp->Close();
	}
   gApplication->Terminate();

}

void CTelescopeFrame::setMemSize()
{
	// Set memory size for start run.
	fMemSize = atoi(fwMemSizeBuffer->GetString());
	cout << "Memsize set to " << fMemSize << endl;
}

void CTelescopeFrame::OpenData(const char* sPath)
{
	TString sFilename = TString::Format("%s/bt05r%06d/",sPath,fRun);
	
	fp = new TFile(sFilename+"telescopedata_"+TString::Format("%d",fRun)+".root","create");

	if(!fp)
	  cout << "Unable to open file." << endl;

}

void CTelescopeFrame::doPon()
{

	ts->Pon();

}
void CTelescopeFrame::setClock()
{

	ts->SetClock(atoi(fwSetClockBuffer->GetString()));

}
int CTelescopeFrame::getCurrentRun()
{

//int daqLoggingManager::initialRunNumber() {
  float run, maxRun(-1);
  const char *file;
  string maxFile;

  void *pDir = gSystem->OpenDirectory(fPath.c_str());
  while (file = gSystem->GetDirEntry(pDir)) {
    if (sscanf(file, "bt05r%f", &run)) {
      if (run > maxRun)
	{ 
	  maxRun = run;
	  maxFile = file;
	}
    }
  }
  string filename(fPath);

  string line;
  filename.append(maxFile);  
  filename.append("/");
  filename.append(maxFile);
  filename.append(".log");
  cout << filename << endl;
  ifstream stream;
  stream.open(filename.c_str());
  stream.seekg(-50,ios::end);
  getline(stream,line);
  cout << line << endl;
  if(line.find("MTB:") == string::npos)
	maxRun = -1;

  stream.close();

  return maxRun;
}

void CTelescopeFrame::SetRunNumber(int iRun)
{

   fRun = iRun;
   fwRunNumber->SetText(Form("%i",iRun));   

}
void CTelescopeFrame::setPedestal()
{

        fPedestals = fwPedestalFileBuffer->GetString();
	cout << "Pedestals set to " << fPedestals << endl; 
}

void CTelescopeFrame::setPath()
{

	fPath = fwOutputDirBuffer->GetString();

	if(fPath[fPath.length()-1] != '/')
	     fPath.append("/");	

	cout << "Outputdir set to " << fPath << endl;
}
void CTelescopeFrame::ClearHistograms()
{

	hSensor0->Reset();
	hSensor1->Reset();
	hSensor2->Reset();
	hSensor3->Reset();
	hSensor4->Reset();
	hSensor5->Reset();
	hSensor6->Reset();
	hSensor7->Reset();

}

void CTelescopeFrame::doCalPulse()
{
	if(fwCalPulse->IsDown())
	{
		cout << "Setting cal pulses with delay " << atoi(fDelayBuffer->GetString())<<endl;
		fwCalPulse->ChangeBackground(colGreen);
		ts->SendCalibrationPulsesOnInternalTriggers(atoi(fDelayBuffer->GetString()));
	}
	else
	{
		fwCalPulse->ChangeBackground(colWhite);
		ts->DisableCalPulses();
	}
	

}

void CTelescopeFrame::doIntTrig()
{
	if(fwInternalTrigger->IsDown())
	{
		cout << "Internal trigger on" << endl;
		fwInternalTrigger->ChangeBackground(colGreen);
		ts->UseInternalTriggers();
	}
	else
	{
		cout << "Internal trigger off" << endl;
		fwInternalTrigger->ChangeBackground(colWhite);
		ts->DisableInternalTriggers();
	}

}
void CTelescopeFrame::doCalStart()
{
	cout << "CalStart"<< endl;
	ts->CalStart();

}
void CTelescopeFrame::setVA()
{
	cout << "set VA" <<endl;
	ts->SetVA(atoi(fVABuffer->GetString()));

}
void CTelescopeFrame::setVD()
{
        cout << "set VD" <<endl;
        ts->SetVD(atoi(fVDBuffer->GetString()));

}
void CTelescopeFrame::setLatchDelay()
{
	cout << "Set Latch delay to " << atoi(fVDBuffer->GetString()) <<endl;
	ts->SetLatchDelay(atoi(fVDBuffer->GetString()));

}

