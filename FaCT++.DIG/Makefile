#
# Makefile for FaCT++
#

# link to Xerces-C
INCLUDES = -I$(XERCESCROOT)/include
LDFLAGS = -L$(XERCESCROOT)/lib/ -lxerces-c

# -- DO NOT CHANGE THE REST OF FILE --
EXECUTABLE = digFaCT++

USE_IL = ../DIGParser ../Kernel ../Bdd

SOURCES = digFileInterface.cpp\
          digFaCT.cpp

include ../Makefile.include
