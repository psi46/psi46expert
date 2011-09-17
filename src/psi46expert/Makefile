# define compile command

CC = g++

# ifeq (,$(PSI46CC))
# CC = g++296
# endif

CFLAGS  = -Wall -g -Wno-deprecated -m32
LDFLAGS = -lusb
LDFLAGS = -L /usr/local/lib -lusb --allow-shlib-undefined
LDFLAGS = -L /usr/local/lib -lusb -m32
SOFLAGS = -shared -m32

ROOTCFLAGS    = $(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTLIBS      = $(shell $(ROOTSYS)/bin/root-config --libs)
ROOTGLIBS     = $(shell $(ROOTSYS)/bin/root-config --glibs)

CFLAGS       += $(ROOTCFLAGS)

# -- Default rule
$(addprefix ../obj/,%.o) : %.cc 
	$(CC) $(CFLAGS) -I../ -c $< -o $@

PSI46EXPERT = MainFrame.o \
              MainFrameDict.o \
              CommandLineInterpreter.o \
              TestParameters.o \
              TestControlNetwork.o \
              TestModule.o \
              TestRoc.o \
              TestDoubleColumn.o \
              TestPixel.o \
              Analysis.o \
	      SCurveTestBeam.o \
	      Test.o \
	      PixelAlive.o \
	      PHCalibration.o \
	      TestRange.o \
	      TestRangeDict.o \
	      Trim.o \
	      BumpBonding.o \
	      AddressLevels.o \
	      ThresholdMap.o \
	      SCurveTest.o \
	      TrimBits.o \
	      DacDependency.o \
	      PHTest.o \
	      PHRange.o \
	      PhDacScan.o \
              DacOverview.o \
              OffsetOptimization.o \
	      ChipVariation.o \
              FigureOfMerit.o \
              PhDacOverview.o \
	      AddressDecoding.o \
	      FullTest.o \
	      ThresholdTest.o \
	      IVCurve.o \
	      TemperatureTest.o \
              UbCheck.o \
              VhldDelOptimization.o \
	      TBMTest.o \
	      AnalogReadout.o \
	      CalDelay.o \
	      ThrComp.o \
	      TemperatureCalibration.o \
	      TBMUbCheck.o \
	      TrimVcal.o \
        VsfScan.o \
	      VsfOptimization.o \
	      PhNoise.o \
              TrimLow.o \
              TimeWalkStudy.o \
              Xray.o 

DAQFILES    = UsbDaq.o \
              daqLoggingManager.o \
              histogrammer.o \
              daqFrame.o daqFrameDict.o \
              $(PSI46EXPERT)

READDATA    = UsbDaq.o \
              histogrammer.o

ANAFILES     = SCurve.o SCurveDict.o CalibrationTable.o PHCalibrationFit.o PHCalibrationFitDict.o



# ----------------------------------------------------------------------
psi46expert: lib
	$(CC) $(CFLAGS) -I../ $(LDFLAGS) psi46expert.cpp -o ../bin/psi46expert ../lib/libpsi46expert.so $(ROOTGLIBS) ../lib/libinterface.so ../lib/libBasePixel.so

lib: $(addprefix ../obj/,$(PSI46EXPERT))
	$(CC) $(SOFLAGS) -o ../lib/libpsi46expert.so $(addprefix ../obj/,$(PSI46EXPERT))

MainFrameDict.cc: MainFrame.h MainFrameLinkDef.h
	$(ROOTSYS)/bin/rootcint  -f MainFrameDict.cc -c -I../ MainFrame.h MainFrameLinkDef.h


# ----------------------------------------------------------------------
takeData: daqlib
	$(CC) $(CFLAGS) -I../ $(LDFLAGS) takeData.cpp -o ../bin/takeData ../lib/libdaq.so $(ROOTGLIBS) ../lib/libinterface.so ../lib/libBasePixel.so

debugData: daqlib
	$(CC) $(CFLAGS) -I../ $(LDFLAGS) debugData.cpp -o ../bin/debugData ../lib/libdaq.so $(ROOTGLIBS) ../lib/libinterface.so ../lib/libBasePixel.so

daqlib: $(addprefix ../obj/,$(DAQFILES))
	$(CC) $(SOFLAGS) -o ../lib/libdaq.so $(addprefix ../obj/,$(DAQFILES))

daqFrameDict.cc: daqFrame.hh daqFrameLinkDef.h
	$(ROOTSYS)/bin/rootcint  -f daqFrameDict.cc -c -I../ daqFrame.hh daqFrameLinkDef.h

daqDACSettingsDict.cc: daqDACSettings.hh daqDACSettings.cc daqDACSettingsLinkDef.h
	$(ROOTSYS)/bin/rootcint -f daqDACSettingsDict.cc -c -I../ daqDACSettings.hh daqDACSettingsLinkDef.h

daqMultiDACSettingsDict.cc: daqMultiDACSettings.hh daqMultiDACSettings.cc daqMultiDACSettingsLinkDef.h
	$(ROOTSYS)/bin/rootcint -f daqMultiDACSettingsDict.cc -c -I../ daqMultiDACSettings.hh daqMultiDACSettingsLinkDef.h

daqTBSettingsDict.cc: daqTBSettings.hh daqTBSettings.cc daqTBSettingsLinkDef.h
	$(ROOTSYS)/bin/rootcint -f daqTBSettingsDict.cc -c -I../ daqTBSettings.hh daqTBSettingsLinkDef.h


# ----------------------------------------------------------------------
readData: readlib
	$(CC) $(CFLAGS) -I../ $(LDFLAGS) readData.cpp -o ../bin/readData ../lib/libreaddata.so $(ROOTGLIBS) ../lib/libinterface.so ../lib/libBasePixel.so

readData.o: readData.cpp
	$(CC) $(CFLAGS) -I../ -c readData.cpp -o ../obj/readData.o


readlib: $(addprefix ../obj/,$(READDATA))
	echo target readlib
	$(CC) $(SOFLAGS) -o ../lib/libreaddata.so $(addprefix ../obj/,$(READDATA))


# ----------------------------------------------------------------------
hvOff: lib
	$(CC) $(CFLAGS) -I../ $(LDFLAGS) hvOff.cpp -o ../bin/hvOff ../lib/libpsi46expert.so $(ROOTGLIBS) ../lib/libinterface.so ../lib/libBasePixel.so

hvOn: lib
	$(CC) $(CFLAGS) -I../ $(LDFLAGS) hvOn.cpp -o ../bin/hvOn ../lib/libpsi46expert.so $(ROOTGLIBS) ../lib/libinterface.so ../lib/libBasePixel.so

hvRead: lib
	$(CC) $(CFLAGS) -I../ $(LDFLAGS) hvRead.cpp -o ../bin/hvRead ../lib/libpsi46expert.so $(ROOTGLIBS) ../lib/libinterface.so ../lib/libBasePixel.so


ana:  $(addprefix ../obj/,$(ANAFILES))
	$(CXX) $(SOFLAGS) -o ../lib/libana.so $(addprefix ../obj/,$(ANAFILES))

SCurveDict.cc: SCurve.h SCurveLinkDef.h
	$(ROOTSYS)/bin/rootcint -f SCurveDict.cc -c SCurve.h SCurveLinkDef.h

PHCalibrationFitDict.cc: PHCalibrationFit.h PHCalibrationFitLinkDef.h
	$(ROOTSYS)/bin/rootcint -f PHCalibrationFitDict.cc -c PHCalibrationFit.h PHCalibrationFitLinkDef.h

TestRangeDict.cc: TestRange.h 
	$(ROOTSYS)/bin/rootcint  -f TestRangeDict.cc -c -I../ TestRange.h


psi46:
	cd ../interface; make;
	cd ../BasePixel; make;
	cd ../psi46expert; make;

all:
	make psi46
	make hvOff
	make hvOn
	make hvRead
	make ana

clean:
	rm -f $(addprefix ../obj/,$(PSI46EXPERT))
	rm -f $(addprefix ../obj/,$(READDATA))
	rm -f $(addprefix ../obj/,$(DAQFILES))
	rm -f core.*
	rm -f *~

cleanall:
	rm -f *Dict*
	rm -f core.*
	rm -f ../obj/*.o
	rm -f ../bin/*
	rm -f ../lib/lib*.so


links:
	cd ../BasePixel; make links
	cd ../psi46expert

	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C0.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C1.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C2.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C3.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C4.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C5.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C6.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C7.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C8.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C9.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C10.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C11.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C12.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C13.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C14.dat
	cp ../BasePixel/defaultParametersRoc/dacParameters.dat testModule/dacParameters_C15.dat

	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C0.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C1.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C2.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C3.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C4.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C5.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C6.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C7.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C8.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C9.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C10.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C11.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C12.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C13.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C14.dat
	cp ../BasePixel/defaultParametersRoc/trimParameters.dat testModule/trimParameters_C15.dat

	cp ../BasePixel/defaultParametersModule/configParameters.dat testModule/configParameters.dat

	cp ../BasePixel/defaultParametersRoc/testParameters.dat testModule/testParameters.dat

	cp ../BasePixel/defaultParametersRoc/tbParameters.dat testModule/tbParameters.dat

	cp ../BasePixel/defaultParametersRoc/tbmParameters.dat testModule/tbmParameters.dat
