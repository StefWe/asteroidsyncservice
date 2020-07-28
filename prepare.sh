#!/bin/bash

cd .. 
wget https://github.com/google/googletest/archive/release-1.8.1.tar.gz
tar -xvzf release-1.8.1.tar.gz
cd asteroidsyncservice/tests
mkdir build 
cd build 
cmake .. 
make 
./test
