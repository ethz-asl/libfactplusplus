#
# Makefile for FaCT++ JNI interface library
#

# -- DO NOT CHANGE THE REST OF FILE --
LIBRARY = FaCTPlusPlusJNI
LTYPE = shared

INCLUDES = -I/System/Library/Frameworks/JavaVM.framework/Headers -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux 
USE_IL = ../Kernel

SOURCES = \
	Axioms.cpp\
	Expressions.cpp\
	KnowledgeExploration.cpp\
	Modularity.cpp\
	Queries.cpp\
	SaveLoad.cpp\
	MemoryStat.cpp\
	FaCTPlusPlus.cpp

include ../Makefile.include
