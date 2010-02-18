#
# Makefile for FaCT++ DIG parser library
#

# link to Xerces-C
INCLUDES = -I$(XERCESCROOT)/include
LDFLAGS = -L$(XERCESCROOT)/lib/ -lxerces-c

# -- DO NOT CHANGE THE REST OF FILE --
LIBRARY = DIGParser
LTYPE = static

USE_IL = ../Kernel

SOURCES = \
	  DIGParserHandlers.cpp\
	  DIGimplInput.cpp\
	  DIGimplOutput.cpp\
	  DIGNamesManager.cpp\
	  DIGinterface.cpp\
	  KernelFactory.cpp

include ../Makefile.include
