lib_LTLIBRARIES = libpsi46interface.la

if HAVE_LIBFTDI
libpsi46interface_la_SOURCES = Delay.cc Log.cc USBInterface.libftdi.cc
else
libpsi46interface_la_SOURCES = Delay.cc Log.cc USBInterface.libftd2xx.cc
endif
libpsi46interface_la_CPPFLAGS = -I$(srcdir)/..

noinst_HEADERS = Delay.h Log.h USBInterface.h
