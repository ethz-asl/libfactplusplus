#
# Makefile for FaCT++ project
#

# -- DO NOT CHANGE THE REST OF FILE --
SUBDIRS = Bdd Kernel FaCT++ DIGParser FaCT++.DIG FaCT++.Server FaCT++.JNI

bdd:
	cd Bdd && make && cd ..

kernel: bdd
	cd Kernel && make && cd ..

digparser: kernel
	cd DIGParser && make && cd ..

fpp_lisp: kernel
	cd FaCT++ && make && cd ..

fpp_jni: kernel
	cd FaCT++.JNI && make && cd ..

fpp_dig: digparser
	cd FaCT++.DIG && make && cd ..

fpp_server: digparser
	cd FaCT++.Server && make && cd ..

include Makefile.include
