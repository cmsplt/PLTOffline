# API Documentation

Here we present the API documentation of the project of the main two API modules.

### Mounting Tool

This tool is used to mount the data from the PLT-BRIL detector. It is a wrapper around the `sshfs` command, and it is used to mount the data from the detector in a given directory.
The connection drop is automatically handled when the lxplus instance is closed.

----
##### MountData
```python
class MountData(user: str, password: str = None)
```
Utility class to mount the data from the PLT-BRIL detector. 

The entered user and password are used to authenticate the 
connection to the cmsusr and brildata machines.

**Args:**

-  <b>`user`</b>  (str): User to authenticate the connection.
-  <b>`password`</b>  (str, optional): Password to authenticate the connection. Defaults to None.

**Returns:**
    None
----
After the class is created, the user can use the `create_mount` method to mount the data in a given directory.

```python
def create_mount(mount_source: str, mount_target: str) -> None:
```
This function creates a mount between a source and a target by  by performung a double ssh to the cmusr and brilmachine. 



**Args:**
 
 - <b>`mount_source`</b> (str, optional):  Machine where data is stored. 
 - <b>`mount_target`</b> (str, optional):  Path to where the mount is going to be created. If it already exists, no exception is raised and if the path does not exist, it is created. 

**Returns:**

 - `None`:  Nothing is returned. 


**Example:**
```python
>>> mount_source = "brildev1:/brildata/22"
>>> mount_target = "./Files/22"
>>> m = MountData("YOUR USER", "YOUR PASSWORD")
>>> m.create_mount(mount_source, mount_target)
```
----



### Anomaly Detection Tool

This tool is used to detect anomalies in the data from the PLT-BRIL detector. It is a wrapper around the `python` command, and it is used to detect anomalies in the data from the detector in a given directory. The class used is:

---
##### AnomalySearcher
```python
class AnomalySearcher(mount_path: str)
```
Constructor for the AnomalySearcher class

**Args**:

- <b>`mount_path`</b> (str, optional): Path to where the data from BRILDATA is
    mounted per year.

**Returns:**

 - <b>`None`</b>:  Nothing is returned. 

**Example:**
```python
    >>> searcher = AnomalySearcher("./Files/22")
```
---
#### Per Fill Analysis

The tool allows the user to run the analysis of a single fill.


```python
def AnomalySearcher(fill_number: int,
                    save_path: str = None,
                    subsample: int = 5,
                    return_preprocessed: bool = False,
                    verbose: bool = False,
                    generate_plots: bool = False
                    ) -> Union[dict, Tuple[pd.DataFrame, dict]]:
```
Runs the pipeline for anomaly detection on a single fill

**Args**:

-   <b>`fill_number`</b> (int): Fill number to be analyzed. Must be a positive integer.
 contained in the mount path.
-    <b>`save_path`</b> (str, optional): Where to save the output.
        Defaults to None.
-    <b>`subsample`</b> (int, optional): Subsample rate, in seconds for the data,
        A larger subsample rate will make the pipeline run faster,
        but the results will be less granular.
        Defaults to 5.
-    <b>`return_preprocessed`</b> (bool, optional): If True, returns the
        preprocessed step for visualization. Defaults to False.
-    <b>`verbose`</b> (bool, optional): If True, prints the output
        Defaults to False.
-    <b>`generate_plots`</b> (bool, optional): If True, generates plots for each
        one of the channels and with preprocessing data.
        Defaults to False.

**Returns**:

-    <b>`Union[dict, Tuple[pd.DataFrame, dict]]`</b>: 
    Output of the pipeline.
     If return_preprocessed is True, returns a tuple with the preprocessed data and the output of the pipeline.
     Otherwise, returns the output of the pipeline, which is a dictionary with each channel as a key 
     and the anomalies information as a value for each one.

**Example:**
```python
    >>> searcher = AnomalySearcher("./Files/22")
    >>> searcher.AnomalySearcher(fill_number=8080,
                                save_path="./Results/22",
                                generate_plots=True)
```
#### Batched Analysis

```python
def run_scan(cls,
             mount_path: str,
             output_path: str,
             *, # Any keyword arguments:
             overwrite: bool = False,
             make_anomalous_plots: bool = True,
             make_normal_plots: bool = False,
             verbose: bool = False,
             progress_bar: bool = True
             ) -> None:
```
Runs the scan of the anomaly detection pipeline on a given mount path
(Note that the mount path must be a directory at the brildata machine).
Each one of the available fill files will be processed.

**Args**:

-   <b>`mount_path`</b> (str): Path to mount containing the fill files that was mounted 
    using the MountData class. 

-   <b>`output_path`</b> (str): Where to save the output of the analysis. Here, the logs are saved, 
    the plots are saved in the `/plots` folder and, and the results json are saved for each one of the fill files studied. As the iteration occurs, the internal `generate_fills_report` function is called to generate the report for all the fill files, reporting the anomalous channels where anomalies were found.

**Keyword Args**:

-   <b>`overwrite`</b> (bool, optional): A boolean indicating if the fills already reported in the output_path should be studied again. Defaults to False, meaning that the fills already reported in the output_path will not be analyzed again.

-   <b>`make_anomalous_plots`</b> (bool, optional): A boolean indicating if the plots for the anomalous channels should be generated. Defaults to True, meaning that the plots for the anomalous channels will be generated and saved by default in the plots folder. Turning this functionality off will save time when running the pipeline on a large number of fills.

- <b>`make_normal_plots`</b> (bool, optional): A boolean indicating if the plots for the normal channels should be generated. Defaults to False, meaning that the plots for the normal channels will not be generated by default.

-  <b>`verbose`</b> (bool, optional): A boolean indicating if the output of the pipeline should be printed. Turn it on for debugging purposes and internal analysis. Also exceptions raised for incomplete fills will be printed. Defaults to False.

- <b>`progress_bar`</b> (bool, optional): A boolean indicating if a progress bar should be shown. Defaults to True, meaning that a progress bar generated by tqdm will be shown. It is recommended to keep
    this option on to have an estimate of the progress of the pipeline and the time to complete. 


**Returns**:

-  <b>`None`</b>: Nothing is returned.  

**Example:**
```python
    >>> searcher = AnomalySearcher("./Files/22")
    >>> searcher.run_scan(mount_path="./Files/22",
                          output_path="./Results/22",
                          make_anomalous_plots=True,
                          make_normal_plots=True,
                          verbose=True,
                          progress_bar=True)
```

---
##### Further details

Further documentation can be found in the [source code](https://github.com/munozariasjm/plt-anomaly-detector).

If there is any question or problem, please contact [Jose M Munoz](https://munozariasjm.github.io/).