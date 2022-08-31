# Installation Guide

Here we present the installation guide for the Anomaly Detection Tool.

### Automatic Installation

Provided that you are in a CERN lxplus environment, you can install the package with the following command runned from inside this directory:

```bash
source ./setup.sh
```
 
Notice that this will add a new environment to your path. If you wish to come bach to your previous lxsplus environment, you can run the following command:```source ~/.bashrc```. 

### Manually preparing the environment:
Ofcourse you can also perform each one of the steps manually by running the following commands:

```bash
source /cvmfs/sft.cern.ch/lcg/views/dev4/latest/x86_64-centos7-gcc11-opt/setup.sh
```
This is necessary to have a pipenv available, torch and a more updated python version. Now, we will install the packages we need:

```bash
pip install pipenv
pipenv shell
pip install -r requirements.txt
```

Finally, the package is ready to be installed (in order to apply the changes, you need to run the following command:

```bash
pip install .
```
And you are ready to go!

**Happy physics**

---
Further details can be found in the [source code](https://github.com/munozariasjm/plt-anomaly-detector).

If there is any question or problem, please contact [Jose M Munoz](https://munozariasjm.github.io/).
