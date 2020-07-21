#!/bin/bash

################################################################################

function makeLib {
  unzip -n lib.zip #NOTE never overwriting

  cd lib/cudd-3.0.0

  ./configure --enable-obj
  make

  cd ../..
}

################################################################################

makeLib
