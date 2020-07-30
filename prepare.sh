#!/bin/bash

cd .. 
wget https://github.com/google/googletest/archive/release-1.8.1.tar.gz
tar -xvzf release-1.8.1.tar.gz
export GMOCK_HOME=/home/travis/build/StefWe/gmock-1.8.1/googlemock
cd asteroidsyncservice/tests
mkdir build 
cd build 
cmake .. 
make 
./test
