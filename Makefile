#
# Makefile for FaCT++ project
#

# -- DO NOT CHANGE THE REST OF FILE --
SUBDIRS = Kernel FaCT++ FaCT++.JNI FaCT++.C
#SUBDIRS = Kernel Tests

include Makefile.include

# Additional targets to build parts of the FaCT++

.PHONY: kernel
kernel:
	make -C Kernel

.PHONY: fpp_lisp
fpp_lisp: kernel
	make -C FaCT++

.PHONY: fpp_jni
fpp_jni: kernel
	make -C FaCT++.JNI

