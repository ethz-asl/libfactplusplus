cmake_minimum_required(VERSION 2.6)

include(ReMake)

remake_project(
  libfact++
  VERSION 1.6.4
  RELEASE stable
  SUMMARY "An efficient Description Logic reasoner compatible with OWL DL and OWL 2."
  AUTHOR "Dmitry Tsarkov"
  CONTACT "tsarkov@cs.man.ac.uk"
  HOME "http://bitbucket.org/dtsarkov/factplusplus"
  LICENSE "GNU Lesser General Public License (LGPL)"
  README README.md
  INSTALL /usr
  FILENAME libfact++
  HEADER_DESTINATION include/fact++
  PREFIX fact++
  EXTRA_CXX_FLAGS -std=c++0x
)

remake_doc(
  html
)

remake_pack_deb(
)
remake_pack_deb(
  COMPONENT dev
  DEPENDS libfact++
  DESCRIPTION "development headers"
)
remake_pack_deb(
  ARCH all
  COMPONENT doc
  DESCRIPTION "documentation"
)

remake_distribute_deb(
  DISTRIBUTION trusty
  SECTION libs
  UPLOAD ppa:kralf/asl
  DEPENDS remake doxygen pkg-config
  PASS CMAKE_BUILD_TYPE LIBFACTPLUSPLUS_GIT_REVISION
)
