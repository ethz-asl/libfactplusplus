#
# Makefile for FaCT++ project
#

# -- DO NOT CHANGE THE REST OF FILE --
SUBDIRS = Kernel FaCT++ FaCT++.JNI DIGParser FaCT++.DIG FaCT++.Server

include Makefile.include

# Additional targets to build parts of the FaCT++

kernel:
	make -C Kernel

digparser: kernel
	make -C DIGParser

fpp_lisp: kernel
	make -C FaCT++

fpp_jni: kernel
	make -C FaCT++.JNI

fpp_dig: digparser
	make -C FaCT++.DIG

fpp_server: digparser
	make -C FaCT++.Server

