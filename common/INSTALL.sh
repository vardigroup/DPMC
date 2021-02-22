#!/bin/bash

################################################################################

function installGmp { # called from unzipped ./lib/
  cd gmp
  ./configure --quiet --prefix=$PWD/INSTALLED --enable-cxx --enable-shared=no
  make --silent
  make --silent install
  cd ..
}

function installSylvan { # called from unzipped ./lib/
  cd sylvan
  mkdir -p build
  cd build
  cmake -DCMAKE_INSTALL_PREFIX=../INSTALLED -DBUILD_SHARED_LIBS=OFF  ..
  make --silent
  make --silent install
  cd ../..
}

function installCudd { # called from unzipped ./lib/
  cd cudd
  ./configure --quiet --prefix=$PWD/INSTALLED --enable-obj
  make --silent
  make --silent install
  cd ..
}

function makeLib {
  unzip -qq -n lib.zip #NOTE never overwriting
  cd lib

  # installGmp
  installSylvan
  installCudd

  cd ..
}

################################################################################

makeLib
