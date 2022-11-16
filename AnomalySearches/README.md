# Anomaly Search Utilities

This directory contains utilities for searching for anomalies in the PLT data. Note that these utilities are not currently used in the production pipeline and it is a collection of several tool.

### Tools
- `plt-anomaly-detector` - An unsupervised anomaly detector that uses an unsupervised Gaussian Model to detect anomalies in the scan of the hd5 files of fills available. See project documentation for more details.
* `KmeansAnalyzer` - A tool that uses Kmeans clustering to detect anomalies in the scan of Slink files.

---
## Usage
The `src` contains the scripts necesary to setup and compile the tools.

In order to prepare everything, you should run the following commands:
```bash
cd src
./setup.sh
```

This will create a `build` directory and download the necessary dependencies. After that, you should run the kmeans via:
```bash
python src/run_scan.py
```

You can also provide as an argument the path to the  folder  with slink files or a single slink file. If no argument is provided, the script will ask for the path to the folder. The output is saved for default in the `output` file.