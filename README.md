# FaCT++

## Summary

**FaCT++** is a DL reasoner. It supports OWL DL and (partially) OWL 2 DL

## FaCT++ is moved to BitBucket

All further development will be performed at

[https://bitbucket.org/dtsarkov/factplusplus](https://bitbucket.org/dtsarkov/factplusplus)

## Versions

The latest version available form here is **1.6.3**, released 30 May 2014.

FaCT++ is implemented in C++ and uses optimised tableaux algorithms.

## Download

**Important**

Since January 2014 it is not possible to use Download option in Google Code
sites. You can download FaCT++ from Google Drive site instead.

## Detailed description and future plans

FaCT++ is partially supporting OWL 2 DL. The missing bits are:

- No support for keys

- Partial datatype support. At the moment, the only supported datatypes
  are Literal, string, anyURI, boolean, float, double, integer, int,
  nonNegativeInteger, dateTime.

The 1.4.0 release **is** the last one that support **DIG** interface.

The 1.4.0 release **is** the last one that supports **OWL API v2** interface.

The 1.4.1 release **is** the last one that supports **OWL API v3.0** interface

If you are lost in all the FaCT++ and/or APIs versions supported please
check the WhichVersionDoINeed page.

# Related Projects

* **JFact** is a java port of FaCT++. It is available from
  [http://sourceforge.net/projects/jfact](https://bitbucket.org/dtsarkov/factplusplus)
  under LGPL.
    
* **owlcpp** is a C++ library for parsing, querying, and reasoning with OWL 2
  ontologies. It is available from
  [http://owl-cpp.sourceforge.net](http://owl-cpp.sourceforge.net) under
  Boost Software License. 