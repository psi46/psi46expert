AM_CPPFLAGS = -I$(srcdir)/.. $(ROOTFLAGS) -DDATADIR='"$(pkgdatadir)"'

# PROGRAMS ----------------------------------------------------------------------------------------------------------------------------------------------------

bin_PROGRAMS = psi46expert psi46hvOff psi46hvOn psi46hvRead psi46takeData psi46debugData psi46readData

psi46expert_SOURCES = psi46expert.cpp
psi46expert_LDADD = libpsi46expert.la ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(ROOTLIBS) $(LIBFTD2XX) $(LIBFTDI) $(LIBUSB) $(LIBREADLINE) -lMinuit
psi46expert_LDFLAGS = -static

psi46hvOff_SOURCES = hvOff.cpp
psi46hvOff_LDADD = ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(LIBFTD2XX) $(LIBFTDI) $(LIBUSB)
psi46hvOff_LDFLAGS = -static

psi46hvOn_SOURCES = hvOn.cpp
psi46hvOn_LDADD = ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(LIBFTD2XX) $(LIBFTDI) $(LIBUSB)
psi46hvOn_LDFLAGS = -static

psi46hvRead_SOURCES = hvRead.cpp
psi46hvRead_LDADD = ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(LIBFTD2XX) $(LIBFTDI) $(LIBUSB)
psi46hvRead_LDFLAGS = -static

psi46takeData_SOURCES = takeData.cpp
psi46takeData_LDADD = libpsi46daq.la ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(ROOTLIBS) $(LIBFTD2XX) $(LIBFTDI) $(LIBUSB) -lMinuit
psi46takeData_LDFLAGS = -static

psi46debugData_SOURCES = debugData.cpp
psi46debugData_LDADD = libpsi46daq.la ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(ROOTLIBS) $(LIBFTD2XX) $(LIBFTDI) $(LIBUSB) -lMinuit
psi46debugData_LDFLAGS = -static

psi46readData_SOURCES = readData.cpp
psi46readData_LDADD = libpsi46readdata.la ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(ROOTLIBS) $(LIBFTD2XX) $(LIBFTDI) $(LIBUSB)
psi46readData_LDFLAGS = -static

# LIBRARIES ---------------------------------------------------------------------------------------------------------------------------------------------------

lib_LTLIBRARIES = libpsi46expert.la libpsi46daq.la libpsi46ana.la libpsi46readdata.la

libpsi46expert_la_SOURCES = AddressDecoding.cc \
			    AddressLevels.cc \
			    AnalogReadout.cc \
			    Analysis.cc \
			    BumpBonding.cc \
			    CalDelay.cc \
			    ChipVariation.cc \
			    CommandLineInterpreter.cc \
			    DacDependency.cc \
			    DacOverview.cc \
			    FigureOfMerit.cc \
			    FullTest.cc \
			    IVCurve.cc \
			    MainFrame.cc \
			    OffsetOptimization.cc \
			    PHCalibration.cc \
			    PhDacOverview.cc \
			    PhDacScan.cc \
			    PhNoise.cc \
			    PHRange.cc \
			    PHTest.cc \
			    PixelAlive.cc \
			    SCurveTestBeam.cc \
			    SCurveTest.cc \
			    TBMTest.cc \
			    TBMUbCheck.cc \
			    TemperatureCalibration.cc \
			    TemperatureTest.cc \
			    Test.cc \
			    TestControlNetwork.cc \
			    TestDoubleColumn.cc \
			    TestModule.cc \
			    TestParameters.cc \
			    TestPixel.cc \
			    TestRange.cc \
			    TestRoc.cc \
			    ThrComp.cc \
			    ThresholdMap.cc \
			    ThresholdTest.cc \
			    TimeWalkStudy.cc \
			    TrimBits.cc \
			    Trim.cc \
			    TrimLow.cc \
			    TrimVcal.cc \
			    UbCheck.cc \
			    VhldDelOptimization.cc \
			    VsfOptimization.cc \
			    VsfScan.cc \
			    Xray.cc \
			    pipe.cc \
			    DataFilter.cc \
			    HighRatePixelMap.cc \
			    HighRateEfficiency.cc \
			    HighRateTrimLow.cc \
			    HighRateSCurve.cc

nodist_libpsi46expert_la_SOURCES = MainFrameDict.cc \
				   TestRangeDict.cc



libpsi46daq_la_SOURCES = daqFrame.cc \
			 daqLoggingManager.cc \
			 UsbDaq.cc \
			 histogrammer.cc

nodist_libpsi46daq_la_SOURCES = daqFrameDict.cc

libpsi46daq_la_LIBADD = libpsi46expert.la



libpsi46ana_la_SOURCES = SCurve.cc \
			 ../BasePixel/CalibrationTable.cc \
			 PHCalibrationFit.cc

nodist_libpsi46ana_la_SOURCES = SCurveDict.cc \
				PHCalibrationFitDict.cc



libpsi46readdata_la_SOURCES = UsbDaq.cc \
			      histogrammer.cc

# CUSTOM MAKE RULES -------------------------------------------------------------------------------------------------------------------------------------------

TestRangeDict.cc: TestRange.h
	rootcint -f $@ -c -I$(srcdir)/../../src $^
MainFrameDict.cc: MainFrame.h MainFrameLinkDef.h
	rootcint -f $@ -c -I$(srcdir)/../../src $^
SCurveDict.cc: SCurve.h SCurveLinkDef.h
	rootcint -f $@ -c -I$(srcdir)/../../src $^
PHCalibrationFitDict.cc: PHCalibrationFit.h PHCalibrationFitLinkDef.h
	rootcint -f $@ -c -I$(srcdir)/../../src $^
daqFrameDict.cc: daqFrame.hh daqFrameLinkDef.h
	rootcint -f $@ -c -I$(srcdir)/../../src $^

# MENTION ALL HEADERS SO THEY GET DISTRIBUTED -----------------------------------------------------------------------------------------------------------------

noinst_HEADERS = AddressDecoding.h \
		 AddressLevels.h \
		 AnalogReadout.h \
		 Analysis.h \
		 anaQA.h \
		 anaQALinkDef.h \
		 BumpBonding.h \
		 CalDelay.h \
		 ChipVariation.h \
		 CommandLineInterpreter.h \
		 DacDependency.h \
		 DacOverview.h \
		 daqDACSettings.hh \
		 daqDACSettingsLinkDef.h \
		 daqFrame.hh \
		 daqFrameLinkDef.h \
		 daqLoggingManager.hh \
		 daqMultiDACSettings.hh \
		 daqMultiDACSettingsLinkDef.h \
		 daqTBSettings.hh \
		 daqTBSettingsLinkDef.h \
		 FigureOfMerit.h \
		 FullTest.h \
		 histogrammer.h \
		 IVCurve.h \
		 MainFrame.h \
		 MainFrameLinkDef.h \
		 OffsetOptimization.h \
		 PHCalibrationFit.h \
		 PHCalibrationFitLinkDef.h \
		 PHCalibration.h \
		 PhDacOverview.h \
		 PhDacScan.h \
		 PhNoise.h \
		 PHRange.h \
		 PHTest.h \
		 PixelAlive.h \
		 psi46expert.h \
		 SCurve.h \
		 SCurveLinkDef.h \
		 SCurveTestBeam.h \
		 SCurveTest.h \
		 TBMTest.h \
		 TBMUbCheck.h \
		 TemperatureCalibration.h \
		 TemperatureTest.h \
		 TestControlNetwork.h \
		 TestDoubleColumn.h \
		 Test.h \
		 TestModule.h \
		 TestParameters.h \
		 TestPixel.h \
		 TestRange.h \
		 TestRoc.h \
		 ThrComp.h \
		 ThresholdMap.h \
		 ThresholdTest.h \
		 TimeWalkStudy.h \
		 TrimBits.h \
		 Trim.h \
		 TrimLow.h \
		 TrimVcal.h \
		 UbCheck.h \
		 UsbDaq.h \
		 VhldDelOptimization.h \
		 VsfOptimization.h \
		 VsfScan.h \
		 Xray.h \
		 pipe.h \
		 DataFilter.h \
		 HighRatePixelMap.h \
		 HighRateEfficiency.h \
		 HighRateTrimLow.h \
		 HighRateSCurve.h

# ADDITIONAL CLEANING RULES -----------------------------------------------------------------------------------------------------------------------------------

CLEANFILES = MainFrameDict.cc \
             TestRangeDict.cc \
             MainFrameDict.h \
             TestRangeDict.h \
             SCurveDict.cc \
             SCurveDict.h \
             PHCalibrationFitDict.cc \
             PHCalibrationFitDict.h \
             daqFrameDict.cc \
             daqFrameDict.h
