#!/bin/bash

 
wget https://github.com/google/googletest/archive/release-1.8.1.tar.gz
tar -xvzf release-1.8.1.tar.gz
export GMOCK_HOME=~/gmock-1.8.1
pwd
cd asteroidsyncservice/tests
mkdir build 
cd build 
cmake .. 
make 
./test
