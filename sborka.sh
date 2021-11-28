#!/bin/bash

cd ~/pathplanning/PathPlanningProject/Build/Debug
cmake ../../ -DCMAKE_BUILD_TYPE="Debug"
make
make install
cd ~/pathplanning/PathPlanningProject