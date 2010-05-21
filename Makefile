#
# Makefile for FaCT++ project
#

# -- DO NOT CHANGE THE REST OF FILE --
SUBDIRS = Kernel FaCT++ FaCT++.JNI

include Makefile.include

# Additional targets to build parts of the FaCT++

.PHONY: kernel
kernel:
	make -C Kernel

.PHONY: digparser
digparser: kernel
	make -C DIGParser

.PHONY: fpp_lisp
fpp_lisp: kernel
	make -C FaCT++

.PHONY: fpp_jni
fpp_jni: kernel
	make -C FaCT++.JNI

.PHONY: fpp_dig
fpp_dig: digparser
	make -C FaCT++.DIG

.PHONY: fpp_server
fpp_server: digparser
	make -C FaCT++.Server
