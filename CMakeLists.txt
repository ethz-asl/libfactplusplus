cmake_minimum_required(VERSION 2.6)

include(ReMake)

remake_project(
  libfactplusplus
  VERSION 1.6.3
  RELEASE stable
  SUMMARY "An efficient Description Logic reasoner compatible with OWL DL and OWL 2."
  AUTHOR "Dmitry Tsarkov"
  CONTACT "tsarkov@cs.man.ac.uk"
  HOME "http://bitbucket.org/dtsarkov/factplusplus"
  LICENSE "GNU Lesser General Public License (LGPL)"
  INSTALL /usr
  PREFIX factplusplus
  EXTRA_CXX_FLAGS -std=c++0x
)

remake_doc(
  html
)

# remake_pack_deb(
# )
# remake_pack_deb(
#   COMPONENT addons
#   DEPENDS rbdl liblua5.1-0 liburdfdom-model0.2 liburdfdom-model-state0.2
#     liburdfdom-sensor0.2 liburdfdom-world0.2
#   DESCRIPTION "addons"
# )
# remake_pack_deb(
#   COMPONENT dev
#   DEPENDS rbdl libeigen3-dev
#   DESCRIPTION "development headers"
# )
# remake_pack_deb(
#   COMPONENT addons-dev
#   DEPENDS rbdl-dev liblua5.1-0-dev liburdfdom-dev
#   DESCRIPTION "addons development headers"
# )
# remake_pack_deb(
#   ARCH all
#   COMPONENT doc
#   DESCRIPTION "documentation"
# )
# 
# remake_distribute_deb(
#   DISTRIBUTION trusty
#   SECTION libs
#   UPLOAD ppa:kralf/asl
#   DEPENDS libeigen3-dev liblua5.1-0-dev liburdfdom-dev libunittest++-dev
#     remake doxygen pkg-config
#   PASS CMAKE_BUILD_TYPE RDBL_REVISION
# )
