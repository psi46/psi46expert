lib_LTLIBRARIES = libpsi46BasePixel.la libpsi46telescope.la

# Program source declarations
libpsi46BasePixel_la_SOURCES = CalibrationTable.cc \
			       ConfigParameters.cc \
			       ControlNetwork.cc \
			       DACParameters.cc \
			       DecoderCalibration.cc \
			       DoubleColumn.cc \
			       Keithley.cc \
			       Module.cc \
			       Pixel.cc \
			       psi46_tb.cc \
			       RawPacketDecoder.cc \
			       Roc.cc \
			       SysCommand.cc \
			       TBAnalogInterface.cc \
			       TBAnalogParameters.cc \
			       TBInterface.cc \
			       TBM.cc \
			       TBMParameters.cc \
			       TBParameters.cc \
			       remotecalls_chiptest.inc \
			       remotecalls.inc \
			       remotecalls_modultest.inc \
			       remotecalls_xraytest.inc

libpsi46telescope_la_SOURCES = psi46_tbts.cc \
			       ConfigParameters.cc\
			       remotecalls_ts.inc

libpsi46BasePixel_la_CPPFLAGS = -I$(srcdir)/..
libpsi46telescope_la_CPPFLAGS = -I$(srcdir)/.. $(ROOTFLAGS)

noinst_HEADERS = CalibrationTable.h \
		 ConfigParameters.h \
		 ControlNetwork.h \
		 DACParameters.h \
		 DecodedReadout.h \
		 DecoderCalibration.h \
		 DoubleColumn.h \
		 GlobalConstants.h \
		 Keithley.h \
		 Module.h \
		 Pixel.h \
		 psi46_tb.h \
		 psi46_tbts.h \
		 RawPacketDecoder.h \
		 Roc.h \
		 SysCommand.h \
		 TBAnalogInterface.h \
		 TBAnalogParameters.h \
		 TBInterface.h \
		 TBM.h \
		 TBMParameters.h \
		 TBParameters.h \
		 Getline.c
