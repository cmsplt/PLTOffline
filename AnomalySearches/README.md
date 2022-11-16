# Anomaly Search Utilities

This directory contains utilities for searching for anomalies in the PLT data. Note that these utilities are not currently used in the production pipeline and it is a collection of several tool.
---
### Tools
* `plt-anomaly-detector` - An unsupervised anomaly detector that uses an unsupervised Gaussian Model to detect anomalies in the scan of the hd5 files of fills available. See project documentation for more details.
* `KmeansAnalyzer` - A tool that uses Kmeans clustering to detect anomalies in the scan of Slink files.

---
### Usage
The `src` contains the scripts necesary to setup and compile the tools.
