#!/bin/bash

# Prepare the LXPLUS path 
source /cvmfs/sft.cern.ch/lcg/views/dev4/latest/x86_64-centos7-gcc11-opt/setup.sh

# Use pipenv to install dependencies
pip install -r requirements.txt

# Install the package
pip install .

echo "Done... Anomaly detection is ready to use"
echo "Please refer to the README file for further instructions"