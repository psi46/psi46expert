#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "psi46expert/TestRoc.h"
#include "psi46expert/TestControlNetwork.h"
#include <iostream>
#include <string>
#include <cmath>

#define DIGITAL 1

double check_Vana(TBAnalogInterface* tbInterface) {

  // check whether we can program Vana
  tbInterface->RocSetDAC(Vana, 0);
  tbInterface->Flush();
  sleep(0.2);        
  double currentBefore = tbInterface->GetIA();

  tbInterface->RocSetDAC(Vana, 200);
  tbInterface->Flush();
  sleep(0.2);        
  double currentAfter = tbInterface->GetIA();
  double ratio = currentAfter/currentBefore;
  if (ratio<1.) ratio=1./ratio;
  return ratio;
}


int scan_i2c(TBAnalogInterface* tbInterface) {

  // find middle of longest stretch of sda phases that work

  int lowest_valid=-1;
  int num_valid=0;
  bool previous_worked=true;
  int best_num_valid=0;
  int best_sda=-1;
  int num_good=0;
  int num_bad=0;

  for (int sda=0; sda<50; sda++) {
    tbInterface->SetTBParameter("sda", sda%25);
    tbInterface->Flush();
    double current_ratio=check_Vana(tbInterface);
    bool this_worked=(current_ratio>1.5);
    if (this_worked) {
      ++num_good;
      if (!previous_worked) {
	lowest_valid=sda;
	num_valid=1;
      } else {
	num_valid++;
      }
    } else {
      ++num_bad;
      if (previous_worked) {
	if (num_valid>best_num_valid) {
	  best_sda=(lowest_valid+num_valid/2)%25;
	  best_num_valid=num_valid;
	}
      }
    }
    previous_worked=this_worked;
  }
  std::cout << "scan_i2c: fraction of good settings is " << 1.0*num_good/(num_good+num_bad)
	    << std::endl;
  return best_sda;
}

  
int get_num_pixels_ana(short* data, unsigned short count) {

  // use 6 for original firmware and analogue chips
  // use 2 for digital firmware and analogue chips
  int empty_readout_length=1;
  if (count<empty_readout_length) return -1;
  return (count-empty_readout_length)/6;
}


int get_num_pixels_dig(short* data, unsigned short count) {

  // determine number of pixels in readout. return -1 if we don't even find a header

  // first look for header
  bool found_header=0;
  unsigned short ipos=0;
  while (!found_header && ipos<count-2) {
    found_header=(data[ipos]==0x07 && data[ipos+1]==0x0f && (data[ipos+2]&8)==0x08);
    ++ipos;
  }
  if (!found_header) return -1;

  // now check remaining data for pixels
  ipos+=2;
  int npixels=0;
  while (count-ipos>=6) {
    // enough bits for another pixel, but is this really one or just trailing 0s?
    // column and row can never be zero, so we need to check that
    if (data[ipos]!=0 || data[ipos+1]!=0 || data[ipos+2]!=0 || data[ipos+3]!=0) ++npixels;
    ipos+=6;
  }
  return npixels;
}


int get_num_pixels(short* data, unsigned short count) {
  if (DIGITAL)
    //return get_num_pixels_dig(data,count);
    return get_num_pixels_ana(data,count);
  else
    return get_num_pixels_ana(data,count);
}


int scan_tin(TBAnalogInterface* tbInterface) {

  // find middle of longest stretch of tin values that work

  unsigned short count;
  short data[FIFOSIZE];

  int lowest_valid=-1;
  int num_valid=0;
  bool previous_worked=true;
  int best_num_valid=0;
  int best_tin=-1;
  int num_good=0;
  int num_bad=0;

  for (int tin=0; tin<26; tin++) {
    tbInterface->SetTBParameter("tin", tin);
    tbInterface->Flush();
    tbInterface->ADCRead(data, count);
    std::cout << "         tin=" << tin << ", count=" << count << std::endl;
    //std::cout << "         data:";
    //for (int i=0; i<count; i++) std::cout << " " << data[i];
    //std::cout << std::endl;
    int nPixels=get_num_pixels(data,count);
    // digital chip: check whether header is there (nPixels>0)
    // analogue chip: check whether readout stopped due to tout (count<4096ish)
    bool found_header=(nPixels>=0 && count<4000);
    if (found_header) {
      ++num_good;
      if (!previous_worked) {
	lowest_valid=tin;
	num_valid=1;
      } else {
	num_valid++;
      }
    } else {
      ++num_bad;
      if (previous_worked) {
	if (num_valid>best_num_valid) {
	  best_tin=(lowest_valid+num_valid/2)%25;
	  best_num_valid=num_valid;
	}
      }
    }
    previous_worked=found_header;
  }
  std::cout << "scan_tin: fraction of good settings is " << 1.0*num_good/(num_good+num_bad)
	    << std::endl;
  if (num_bad==0) best_tin=42;
  return best_tin; 
}

  
int main(int argc, char* argv[]) {

  //**********************
  //*** INIT              
  //**********************


  // read config parameters
  std::cout << "KHDEBUG: read config parameters" << std::endl;
  string directory;
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i],"-dir")) directory=argv[++i];
  }
  ConfigParameters* configParameters = ConfigParameters::Singleton();
  strcpy(configParameters->directory,directory.c_str());
  configParameters->ReadConfigParameterFile(Form("%s/configParameters.dat",
						 directory.c_str()));
  // open connection to testboard
  std::cout << "KHDEBUG: open connection to testboard" << std::endl;
  TBAnalogInterface* tbInterface = new TBAnalogInterface(configParameters);
  if (!tbInterface->IsPresent()) {
    std::cout << "tbInterface not present" << std::endl;
    return -1;
  }
  // setup control network
  std::cout << "KHDEBUG: setup control network" << std::endl;
  TestControlNetwork* controlNetwork = new TestControlNetwork(tbInterface,
							      configParameters);
  // create testboard object and run the welcome LED message
  std::cout << "KHDEBUG: create testboard object and say hello" << std::endl;
  CTestboard tb;
  tb.Welcome();
  tbInterface->Pon();
  tbInterface->HVoff();

  if (DIGITAL) {
    // switch to digital readout
    std::cout << "KHDEBUG: using DIGITAL readout (i.e. \"second TBM channel\")!!!" << std::endl;
    tbInterface->SetTBMChannel(1);
  } else {
    std::cout << "KHDEBUG: using ANALOGUE readout (i.e. \"first TBM channel\")!!!" << std::endl;
    tbInterface->SetTBMChannel(0);
  }


  // disable TBM emulator. We are only interested in ROC readout right now.
  std::cout << "KHDEBUG: disable TBM emulator" << std::endl;
  tb.TBMEmulatorOff();
  tb.SetTriggerMode(TRIGGER_ROC);


  //*******************************
  //*** COARSE TIMING CONFIGURATION
  //*******************************

  // set delay between reset and cal (5-255)
  tbInterface->SetTBParameter("trc", 20);

  // set number of calibration signals (1-63)
  tbInterface->SetTBParameter("cc", 1);

  // set delay between two cal signals in burst mode (5-255)
  tbInterface->SetTBParameter("tcc", 6);

  // set delay between cal and trigger (5-255). we need this value later to determine wbc.
  int tct=25;
  if (!DIGITAL) tct=104;
  tbInterface->SetTBParameter("tct", tct);

  // delay between trigger and token (5-255)
  tbInterface->SetTBParameter("ttk", 16);

  // repetition time in loop mode (in multiples of 6.4 microseconds beginning to beginning)
  tbInterface->SetTBParameter("trep", 40);

  tbInterface->Flush();


  //**************************
  //*** SCAN TIMING PARAMETERS
  //**************************


  // check all possible clock phases
  for (int clk=0; clk<26; clk+=4) {
    std::cout <<"trying clk=" << clk << std::endl;
    tbInterface->SetTBParameter("clk", clk);
    tbInterface->Flush();

    // find suitable sda setting.
    // only if sda is right, we can see chip currents change with DAC settings
    int sda=scan_i2c(tbInterface);
    if (sda<0) {
      std::cout << "could not find working sda setting. skipping." << std::endl;
      continue;
    }
    tbInterface->SetTBParameter("sda", sda);
    tbInterface->Flush();
    std::cout << "clk=" << clk << ", sda=" << sda << std::endl;

    // now that I2C is configured properly, program DACs
    if (DIGITAL) {
      // Wolfram's values
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vdig", 4);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vana", 70);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vsf", 40);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vcomp", 12);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VwllPr", 35);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VwllSh", 35);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VhldDel", 117);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vtrim", 29);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VthrComp", 75);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VIBias_Bus", 30);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vbias_sf", 6);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VoffsetOp", 40);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VOffsetR0", 140);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VIon", 130);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VIbias_PH", 100);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Ibias_DAC", 80);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VIbias_roc", 150);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VIColOr", 99);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vcal", 200);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("CalDel", 100);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("CtrlReg", 0);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("WBC", 20);
    } else {
      // analogue settings that work without adapter
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vdig", 6);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vana", 124);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vsf", 150);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vcomp", 10);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vleak_comp", 0);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VrgPr", 0);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VwllPr", 35);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VrgSh", 0);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VwllSh", 35);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VhldDel", 160);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vtrim", 7);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VthrComp", 80);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VIBias_Bus", 30);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vbias_sf", 6);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VoffsetOp", 24);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VIbiasOp", 50);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VOffsetR0", 120);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VIon", 130);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VIbias_PH", 120);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Ibias_DAC", 110);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VIbias_roc", 150);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VIColOr", 99);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vnpix", 0);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("VSumCol", 0);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("Vcal", 199);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("CalDel", 81);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("RangeTemp", 7);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("CtrlReg", 0);
      controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("WBC", 100);
    }


    // now find tin phase.

    // analog chip:
    // if tin phase is wrong, there will not be token out,
    // which usually leads to "endless" readout (limited by 4k buffer size).

    // digital chip:
    // end of readout is gated by availability of data from the digital adapter.
    // maybe we can look at the existence of a ROC header instead. there will be no
    // readout if the chip has not recognized its token.

    // if we cannot find any proper setting here, the reason may well be the clock phase.
    // if the clock phase is wrong, we won't be able to read digital data back into the
    // Altera board. But we can't tell from looking at the clk setting alone whether that is
    // the case. Minimum requirement for receiving a proper ROC header is a good combination
    // of clk and tin phases.

    int tin=scan_tin(tbInterface);;
    if (tin<0) {
      std::cout << "could not find working tin setting. skipping." << std::endl;
      continue;
    }
    tbInterface->SetTBParameter("tin", tin);
    tbInterface->Flush();
    std::cout << "clk=" << clk << ", sda=" << sda << ", tin=" << tin << std::endl;



    // remaining parameters that need adjustment:
    // - ctr
    // - WBC
    // - CalDel
    // There is no way to verify the ctr phase independently, because the ROC will
    // respond to a token even if it did not recognize a prior trigger. Therefore we
    // cannot distinguish whether no cal/trig was received (ctr wrong) or whether we
    // are not looking for it at the right time (WBC/CalDel wrong). Let's scan...

    short data[FIFOSIZE];
    unsigned short count;

    /*
    int ctr=0;
    tbInterface->SetTBParameter("tct",10);
    do {
      ctr=(ctr+1)%50;
      tbInterface->SetTBParameter("ctr", ctr);
      tbInterface->Flush();
      tbInterface->Single(5);
      tbInterface->ADCRead(data, count);
      sleep(0.1);
      if (ctr==0) {
	sleep(5);
	std::cout << "start at ctr=0 again..." << std::endl;
      }
    } while (1);
    */


    for (int ctr=0; ctr<26; ctr+=5) {
      std::cout << "wbc/caldel scan for clk=" << clk << ", sda=" << sda << ", tin=" << tin
		<< ", ctr=" << ctr << std::endl;
      for (int wbc=tct-10; wbc<tct; wbc+=1) {
	for (int caldelay=0; caldelay<150; caldelay+=20) { // steps of 10 correspond to 3.2ns
	  tbInterface->SetTBParameter("ctr", ctr);
	  controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("WBC", wbc);
	  controlNetwork->GetModule(0)->GetRoc(0)->SetDAC("CalDel", caldelay);
	  tbInterface->Flush();

	  // arm a few pixels
	  for (int i=0; i<26; i++) {
	    controlNetwork->GetModule(0)->GetRoc(0)->GetDoubleColumn(i)->EnableDoubleColumn();
	  }
	  controlNetwork->GetModule(0)->GetRoc(0)->ArmPixel(5,10);
	  controlNetwork->GetModule(0)->GetRoc(0)->ArmPixel(3,8);

	  tbInterface->Flush();
       	  tbInterface->ADCRead(data, count);
	  int mincount=count;
	  int maxcount=count;
	  double npixels=get_num_pixels(data,count);
          if (npixels > 0)
		       {
          for(int i=0; i<100;i++)
	    {
              tbInterface->ADCRead(data, count);
              if (count>maxcount)
		maxcount=count;
              if (count<mincount)
                mincount=count;
            }
		       }
	  npixels=get_num_pixels(data,maxcount);
	  if (npixels>0 && maxcount<4000) {
	    std::cout << "wbc=" << wbc << ", caldel=" << caldelay
		      << ", clk=" << clk << ", sda=" << sda
		      << ", tin=" << tin << ", ctr=" << ctr
		      << " => maxcount=" << maxcount
		      << " => mincount=" << mincount
		      << " - WORKING SETTINGS" << std::endl;
	  } else {
	    std::cout << "wbc=" << wbc << ", caldel=" << caldelay
		      << ", clk=" << clk << ", sda=" << sda
		      << ", tin=" << tin << ", ctr=" << ctr
		      << " => maxcount=" << maxcount << std::endl;
	  }

	  // disarm pixels again
	  controlNetwork->GetModule(0)->GetRoc(0)->DisarmPixel(5,10);
	  controlNetwork->GetModule(0)->GetRoc(0)->DisarmPixel(3,8);

	}
      }
    }
  }

  tbInterface->Poff();

  return 0;
}
