from typing import Any
import pandas as pd
import itertools
from tqdm.auto import tqdm
import numpy as np
import warnings
import seaborn as sns
import matplotlib.pyplot as plt
from ..data.data_getter import LocalFileExplorer
from .preprocessor import DifferencePreprocessor
from .detectors import EnsambleDetector
import glob
import os
import json

class AnomalySearcher:
    def __init__(self, mount_path: str = None):
        """Constructor for the AnomalySearcher class

        Args:
            mount_path (str, optional): Path to where the data from BRILDATA is
            mounted per year.
        """
        if mount_path:
            self.data_getter = LocalFileExplorer(mount_path)
        self.preprocessor = DifferencePreprocessor()
        self.detecting_machine = EnsambleDetector()
        self.combs = list(itertools.combinations(range(16), 2))

    def get_raw_data(self,
                     fill_number: str,
                     subsample: int = 1,
                     verbose: bool = False) -> pd.DataFrame:
        """Returns the raw data for a given fill number

        Args:
            fill_number (int | str): Fill number to be searched
            subsample (int, optional): Sample the integrated data to
            speed-up anomaly searches. Defaults to 1.

        Returns:
            pd.DataFrame: The pivoted data per channel to generate
                for each one of the fills, the index is the timestamp.
        """
        available_fills = self.data_getter.get_available_files(fill_number)
        dfs = []
        for i in range(len(available_fills)):
            try:
                df = self.data_getter.get_single_dataframe(fill_number, i)
                df = df.pivot_table(
                    index=["dt"], columns="channelid", values="data"
                ).dropna()
                dfs.append(df)
            except Exception as e:
                if verbose:
                    print(f"Problem with fill {fill_number}, {i}")
                    print(e)
        if not subsample:
            subsample = 1
        complete_df = pd.concat(dfs).sort_index()
        sample_df = complete_df.iloc[::subsample]
        int_df = self._resample_and_interpolate(sample_df).sort_index()
        return int_df

    def _resample_and_interpolate(self, data: pd.DataFrame) -> pd.DataFrame:
        """Resamples the dataframe and interpolates the missing values

        Args:
            data (pd.DataFrame): Dataframe with the data

        Returns:
            pd.DataFrame: Resampled dataframe
        """
        return data.resample("1S").mean().interpolate(method="linear")

    def _is_constant(self, channel: int, data: pd.DataFrame) -> bool:
        """Checks if the channel is non-constant in the dataframe

        Args:
            channel (int): Channel to be checked
            data (pd.DataFrame): Dataframe with the data

        Returns:
            bool: True if the channel is non-constant, False otherwise
        """
        x = data[channel].values
        diffs = np.diff(x)
        if len(diffs[diffs == 0]) / len(x) > 0.9:
            return True
        return False

    def list_nonconstant_channels(
        self, data: pd.DataFrame, exclude: int = None
    ) -> list:
        """Lists the non-constant channels in the dataframe

        Args:
            data (pd.DataFrame): Dataframe with the data

        Returns:
            list: List of the non-constant channels
        """
        are_constant = [
            self._is_constant(c, data)
            for c in data.columns if str(c).isnumeric()
        ]
        if exclude:
            return [
                ch
                for ch, is_constant in zip(data.columns, are_constant)
                if ch != exclude and not is_constant
            ]
        return [ch for ch, c in zip(data.columns, are_constant) if not c]

    def study_shannel(
        self, data: pd.DataFrame, studied_channel: int, name="x"
    ) -> pd.DataFrame:
        """Studies the channel in the dataframe

        Args:
            data (pd.DataFrame): Dataframe with the data
            studied_channel (int): Channel to be studied
            plot (bool, optional): If True, plots the data. Defaults to True.
        """
        df = data.copy()
        df["m_agg"] = df[
            (
                c
                for c in self.list_nonconstant_channels(df, studied_channel)
                if c != studied_channel
            )
        ].mean(axis=1)
        X = self.preprocessor(df, ["m_agg", studied_channel])
        return self.preprocessor.build_dataframe(df, X, name=name)
    
    def generate_plots(self,
                       fill_number: int,
                       preprocessed: pd.DataFrame,
                       save_path: int = None):
        """Generates the plots for the fill number
        
        Args:
            fill_number (int): Fill number to be plotted
            preprocessed (pd.DataFrame): Preprocessed data
            save_path (str, optional): Path to save the plots
        """
        raw_df = self.get_raw_data(fill_number)
        fig, ax = plt.subplots(2, 1, figsize=(20, 7), sharex=True)
        for ch in range(16):
            sns.lineplot(data=raw_df[ch], ax=ax[0], label=None)
            sns.lineplot(data=preprocessed[ch], label=ch, ax=ax[1])
        ax[0].set_title("Fill {}".format(fill_number))
        ax[1].set(xlabel="Time", ylabel="Preprocessed diff.")
        if save_path:
            if not os.path.exists(save_path):
                os.makedirs(save_path)
            plt.savefig(os.path.join(save_path,
                                     f"fill_{fill_number}.png"))

    def preprocess_data(self, data: pd.DataFrame) -> pd.DataFrame:
        """Studies the fill in the dataframe

        Args:
            data (pd.DataFrame): Dataframe with the data
        """
        dfs = []
        for channel in range(16):
            dfs.append(self.study_shannel(data, channel, name=channel))
        return pd.concat(dfs, axis=1)

    def search_in_channel(self,
                          channel: int,
                          data: pd.DataFrame, *,
                          th=0.01) -> dict:
        """Searches for anomalies in a given channel
        Args:
            channel (int): Channel to be searched
            data (pd.DataFrame): Dataframe with the data
        """
        logging_dict = {}
        to_search = False
        values = data[channel].values
        passes_threshold = values > th
        other_cols = [c for c in data.columns if c != channel]
        if len(other_cols) > 0:
            if any(passes_threshold):
                to_search = True
                logging_dict["WARNING"] = f"Channel {channel} Anomalous"
            if to_search:
                anomaly_dict = self.detecting_machine.detect(data[channel])
                logging_dict["ANOMALIES"] = anomaly_dict
        return logging_dict

    def search_anomalies(self, x_processed, threshold=0.1):
        report_dict = {}
        for channel in range(16):
            report_dict[channel] = self.search_in_channel(channel,
                                                          x_processed,
                                                          th=threshold)
        return report_dict

    def _run_pipeline(self,
                      fill_number: int,
                      subsample: int = 5,
                      return_preprocessed: bool = False):
        data = self.get_raw_data(fill_number, subsample=subsample)
        prepared_data = self.preprocess_data(data)
        anomaly_dict = self.search_anomalies(prepared_data)
        if return_preprocessed:
            return prepared_data, anomaly_dict
        return anomaly_dict

    @staticmethod
    def save_output(output, path):
        """Saves the output to a json file

        Args:
            output (Tuple | dict): Result from anomaly detection
            path (str): Where to save the output

        Returns:
            output: (Tuple | dict)
        """
        if isinstance(output, dict):
            output_dict = output
        elif isinstance(output, tuple):
            for out in output:
                if isinstance(out, dict):
                    output_dict = out
        if not path.endswith(".json"):
            path = path + ".json"
        with open(path, "w") as f:
            json.dump(output_dict, f)
        return output

    def __call__(self, fill_number: int,
                 save_path: str = None,
                 subsample: int = 5,
                 return_preprocessed: bool = False,
                 verbose: bool = False,
                 generate_plots: bool = False
                 ) -> Any:
        """Runs the pipeline

        Args:
            fill_number (int): Fill number
            save_path (str, optional): Where to save the output.
                Defaults to None.
            subsample (int, optional): Subsample rate, in seconds for the data,
                A larger subsample rate will make the pipeline run faster,
                but the results will be less granular.
                Defaults to 5.
            return_preprocessed (bool, optional): If True, returns the
                preprocessed step for visualization. Defaults to False.
            verbose (bool, optional): If True, prints the output
                Defaults to False.
            generate_plots (bool, optional): If True, generates plots for each
                one of the channels and with preprocessing data.
                Defaults to False.

        Returns:
            Any: Output of the pipeline
        """
        if not save_path:
            warnings.warn("""No save path provided.
            Output will not be saved.""", DeprecationWarning, stacklevel=2)
        if generate_plots:
            return_preprocessed = True
        output = self._run_pipeline(fill_number=fill_number,
                                    subsample=subsample,
                                    return_preprocessed=return_preprocessed)
        if save_path:
            if "single_fill_reports" not in save_path:
                single_output_path = os.path.join(save_path,
                                                  "single_fill_reports",
                                                  str(fill_number))
            elif fill_number not in save_path:
                single_output_path = os.path.join(save_path,
                                                  str(fill_number))
            else:
                single_output_path = save_path
            warnings.warn("""Save path does not exist. Creating it.""")
            if not os.path.exists(single_output_path.replace(f"{fill_number}",
                                                             "")):
                os.makedirs(single_output_path.replace(f"{fill_number}", ""))
            self.save_output(output, single_output_path)
            if generate_plots:
                base_path = single_output_path.split("single_fill_reports")[0]
                plot_path = os.path.join(base_path,
                                         "plots")
                if not os.path.exists(plot_path):
                    os.makedirs(plot_path)
        if verbose:
            print(output)
        if isinstance(output, tuple):
            preprocessed_data = output[0]
            if generate_plots:
                self.generate_plots(fill_number, preprocessed_data, plot_path)
        return output

    def generate_fills_report(self,
                              report_paths: str,
                              output_path: str,
                              ) -> dict:
        """Generates a report with all the fills in a given path

        Args:
            report_paths (str): Path to the reports
            output_path (str): Where to save the report

        Returns:
            dict: Report per each one of the channels
        """
        available_fills = glob.glob(report_paths + "/*")
        entire_report = {"fill_n": [],
                         "Anomalous_channels": [],
                         "Normal_channels": []}
        for generated_report in available_fills:
            fill_n = generated_report.split("/")[-1].replace(".json", "")
            entire_report["fill_n"].append(fill_n)
            with open(generated_report, "r") as f:
                report = json.load(f)
            anomalous = []
            nomals = []
            for ch_id, ch_cont in report.items():
                if ch_cont:
                    anomalous.append(ch_id)
                else:
                    nomals.append(ch_id)
            entire_report["Anomalous_channels"].append(anomalous)
            entire_report["Normal_channels"].append(nomals)
            report_df = pd.DataFrame(entire_report)
            report_df.to_pickle(os.path.join(output_path, "report_df.pkl"))
            self.save_output(entire_report,
                             os.path.join(output_path, "report_json"))
        return entire_report

    @classmethod
    def run_scan(cls,
                 mount_path: str,
                 output_path: str,
                 *,
                 overwrite: bool = False,
                 make_anomalous_plots: bool = True,
                 make_normal_plots: bool = False,
                 verbose: bool = False,
                 progress_bar: bool = True
                 ) -> None:
        """Runs the scan of the anomaly detection pipeline on a given mount path
        (Note that the mount path must be a directory at the brildata machine).
        Each one of the available fill files will be processed.

        Args:
            mount_path (str): Path to mount containing the fill files that
            was mounted using the MountData class.
            output_path (str): Where to save the output of the analysis.
            Here, the logs are saved, the plots are saved in the `/plots`
            folder and, and the results json are saved for each one of the
            fill files studied.
            As the iteration occurs, the internal `generate_fills_report`
            function is called to generate the report for all the fill files,
            reporting the anomalous channels where anomalies were found.
            
        Keyword Arguments:
            overwrite (bool, optional): A boolean indicating if the fills
            already reported in the output_path should be studied again.
            Defaults to False, meaning that the fills already reported in the
            output_path will not be analyzed again.
            make_anomalous_plots (bool, optional): A boolean indicating if the
            plots for the anomalous channels should be generated.
            Defaults to True, meaning that the plots for the anomalous channels
            will be generated and saved by default in the plots folder.
            Turning this functionality off will save time when running the
            pipeline on a large number of fills.
            make_normal_plots (bool, optional): A boolean indicating if the
            plots for the normal channels should be generated.
            Defaults to False, meaning that the plots for the normal
            channels will not be generated by default.
            verbose (bool, optional): A boolean indicating if the output
            of the pipeline should be printed. Turn it on for debugging
            purposes and internal analysis. Also exceptions raised for
            incomplete fills will be printed. Defaults to False.
            progress_bar (bool, optional): A boolean indicating if a progress
            bar should be shown. Defaults to True, meaning that a progress bar
            generated by tqdm will be shown. It is recommended to keep this
            option on to have an estimate of the progress of the
            pipeline and the time to complete.
        
        """
        # Make sure the directories exist
        assert os.path.isdir(mount_path), f"{mount_path} is not a directory"
        # Create instance of the class
        # Reporting path generation per fill
        per_fill_path = os.path.join(output_path, "single_fill_reports")
        searcher = cls(mount_path=mount_path)
        # Get the list of fills
        available_fills = glob.glob(mount_path + "/*")
        assert len(available_fills) > 0, "No fills found"
        already_analyzed = glob.glob(os.path.join(output_path, 
                                                  "single_fill_reports",
                                                  "*.json"))
        already_analyzed = [x.split("/")[-1].replace(".json", "")
                            for x in already_analyzed]
        # Iterate over the fills
        if not overwrite:
            available_fills = [n for n in available_fills
                               if n.split("/")[-1] not in already_analyzed]
        it_fills = tqdm(available_fills) if progress_bar else available_fills
        log_info = {}
        for fill in it_fills:
            try:
                if verbose:
                    print(f"Scanning fill {fill}...")
                fill_number = fill.split("/")[-1]
                prepared_data, anomaly_dict = searcher(fill_number,
                                                    output_path,
                                                    return_preprocessed=True)
                log_info[fill_number] = "Analysis completed"
                is_anomalous = False
                for ch_id, ch_cont in anomaly_dict.items():
                    if ch_cont:
                        is_anomalous = True
                        break
                base_path = per_fill_path.split("single_fill_reports")[0]
                plot_path = os.path.join(base_path,
                                         "plots")
                if is_anomalous and make_anomalous_plots:
                    searcher.generate_plots(int(fill_number),
                                       prepared_data,
                                       plot_path)
                if not is_anomalous and make_normal_plots:
                    searcher.generate_plots(int(fill_number),
                                        prepared_data,
                                        plot_path)
                # Generate the report of all the fills
                _ = searcher.generate_fills_report(per_fill_path, output_path)
            except Exception as e:
                if verbose:
                    print(f"Problem with fill {fill_number}")
                    print(e)
                    print("*"*50)
                log_info[fill_number] = str(e)
            # Save the log
            fail_path = os.path.join(output_path, "logs.json")
            cls.save_output(log_info, fail_path)
        # Generate the report of all the fills
        _ = searcher.generate_fills_report(per_fill_path,
                                           output_path)
        return None

