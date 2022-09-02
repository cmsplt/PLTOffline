# Plt Anomaly Detection Tool
### PLT-BRIL, CMS, CERN


[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://GitHub.com/Naereen/StrapDown.js/graphs/commit-activity)

[![Linux](https://svgshare.com/i/Zhy.svg)](https://svgshare.com/i/Zhy.svg)

[![Documentation Status](https://readthedocs.org/projects/ansicolortags/badge/?version=latest)](https://munozariasjm.github.io/plt-anomaly-detector/)

---
### Description

This repo is a efficient implementation of an anomaly detection algorithm suited for the data coming from the PLT-BRIL detector, is is intended to be robust to experimental noise and not highly tuneable.

For now, the implementation is a Proof-of-Concept, and looks forward to be generalized to other usecases and dockerized.

Please visit the [documentation](https://munozariasjm.github.io/plt-anomaly-detector/) for more information.

---

### Quick start

We recommend to use all the commands within a lxplus instance. To beguin, please be in the directory of this repository.

```bash
cd AnomalyDetection
```

From here, we should create a new pipenv environment to install the required packages. This can be easily done via the script we prepared:

```bash
source setup.sh
```
Now, we prepared a module to easily mount the data from the PLT-BRIL detector. To do so, we need to run the following command in a python shell:

```python
from src.data.mounting_tool import MountData
MOUNT_TARGET = "./Files/22"
mounter = MountData(user="YOUR_CMSUSR", password="YOUR_password")
mounter.create_mount(mount_source="brildev1:/brildata/22/", mount_target=MOUNT_TARGET)
```
This will mount all the files containing fills in 2022 in the directory `./Files/22`. And the data will be unmounted when the lxplus instance is closed.
Now, the data is ready to be used. 

There are two main ways of running the anomaly detection algortihm. One of them is to run a single fill file:

```python
from src.model.searcher import AnomalySearcher
searcher = AnomalySearcher(MOUNT_TARGET)

# Fill to analyze:
FILL_N = 1
result = searcher(FILL_N,
                      return_preprocessed=True,
                      generate_plots=True,
                      save_path = "./Results/22",
                      )

```
This function will return a dictionary with the results of the analysis. The plots will be saved in the directory `./Results/22`. The plots are saved in the following format: `FILL_N.png`. The preprocessed data will be saved in the following format: `./Results/22/single_fill_reports/FILL_N.pkl`.

The other way of running the algorithm is to run it over all the fills in a given directory:

```python
AnomalySearcher().run_scan(
    mount_path=MOUNT_TARGET,
    output_path="./Results/22",
    make_anomalous_plots=True,
    overwrite=False,
    progress_bar=True,
)
```

Running this function will create a directory `./Results/22` with the following structure:

```bash
-|Results/22 |
             |-plots
                |-FILL_0.png
                |-FILL_1.png
                |-FILL_2.png
                |   ...
             |-single_fill_reports
             |-report_df.pkl
             |-logs.json
```

The `report_df.pkl` file contains a pandas dataframe with the results of the analysis. The `logs.json` file contains the logs of the analysis. The `plots` directory contains the plots of the anomalous fills. The `single_fill_reports` directory contains the analysis data of the fills, reporting different types of anomalies and their respective timestamps.

To use with more details these endpoints, please refer to the [documentation]().

---

### Contributing

All the dependencies are managed via pipenv for reproducibility and ease of use. We lint all the code with flake8 and black and shall use precode to enforce the coding style.

Please add changes to a new branch, and considering test passing, the PR to the master branch.

--- 

**Contact**

Contact @munozariasjm for any issues or questions related to the code.
