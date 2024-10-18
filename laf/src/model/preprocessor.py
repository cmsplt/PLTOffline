from sklearn.preprocessing import MinMaxScaler
from typing import List, Tuple
import torch
import pandas as pd
import numpy as np
import warnings
import torch.nn.functional as F
import seaborn as sns
import matplotlib.pyplot as plt

warnings.filterwarnings("ignore")

class DifferencePreprocessor:
    def __init__(self) -> None:
        """
        Initializes the preprocessor for anomaly detection in fill data
        It is especially useful for the difference between two columns and to
        ignore fast dropouts in the data.
        """
        self.scaler = MinMaxScaler()

    def __call__(
        self,
        downloaded_df: pd.DataFrame,
        target_cols: List[str],
        *,
        scale=True,
        downsample_perc: float = None,
        **kwargs,
    ) -> Tuple[torch.Tensor]:
        """
        Prepares the data for the model, scaling the data and converting it
        to a tensor.
        Args:
            df (_type_): Readed data from the .pkl files... containing
            information about the files
            target_cols (List[str]): The columns to transform

        KeyArgs:
            scale (bool, optional): If True, the data will be scaled.
            Defaults to True.
            downsample_perc (float, optional): If not None, the data will
            be downsampled. Defaults to None.

        Returns:
            Tuple(torch.Tensor): time, feature_data
        """
        assert len(target_cols) == 2, "Only two target columns are supported"
        foo_df = downloaded_df.copy()
        if scale:
            scaled = self._scale_df(foo_df, target_cols)
        else:
            scaled = torch.FloatTensor(foo_df[target_cols].values)
        log_diff = self._delta_cols(scaled)
        convolved = self.roll_convolution(log_diff, **kwargs) #if applying a 'mean' kernel, this seems to me like a moving average over a period corresponding to 2.5% of the time series len
        if downsample_perc is not None:
            sampled = self.downsample(convolved, downsample_perc)
        else:
            sampled = convolved
        return sampled

    def _scale_df(self, df: pd.DataFrame, target_cols):
        ndf = df.copy()
        X_vals = self.scaler.fit_transform(df[target_cols])
        ndf[target_cols] = X_vals
        return torch.FloatTensor(ndf[target_cols].values)

    def _delta_cols(self, scaled_tensor: pd.DataFrame):
        diff = torch.diff(scaled_tensor, dim=1).abs() #here is where you pass from 2 cols to 1 
        log_diff = torch.log(diff + 1)
        return log_diff

    @staticmethod
    def roll_convolution(
        series: torch.Tensor,
        aggregation: callable = "mean",
        conv_size: float = 0.025,
    ):
        ts_tensor = series.reshape(1, 1, -1)
        if aggregation == "mean":
            kernel = (
                torch.ones(1, 1, int(conv_size * len(series)))
                * 1
                / int(conv_size * len(series))
            )
        elif aggregation == "random":
            kernel = torch.rand(1, 1, int(conv_size * len(series)))
        else:
            raise ValueError("Aggregation method not supported")
        kernel_tensor = torch.Tensor(kernel).reshape(1, 1, -1)
        return F.conv1d(ts_tensor, kernel_tensor).reshape(-1, 1)

    @staticmethod
    def downsample(tensor: torch.Tensor, frac: float = 0.01) -> pd.DataFrame:
        """
        Donwsample the df by frac
        Notice that this reduces the number of samples in the df
        but makes possible to speedup analysis
        """
        folding_size = int(len(tensor) * frac)
        indices = torch.linspace(0, len(tensor) - 1, steps=len(tensor)).int()
        mask = indices % folding_size == 0
        sampled = tensor[mask]
        return sampled

    @staticmethod
    def build_dataframe(
        original_df: pd.DataFrame, X: torch.Tensor, name="x"
    ) -> pd.DataFrame:
        dt_min = pd.Timestamp(original_df.index.min())
        dt_max = pd.Timestamp(original_df.index.max())
        number_s = len(X)
        step = (dt_max - dt_min) / number_s
        tdelta = [step * i + dt_min for i in range(number_s)]
        return pd.DataFrame({"index": tdelta,
                             name: X.squeeze().numpy()}).set_index("index")

    #@classmethod
    #def plot_process(cls, df, target_cols, ax=[], **kwargs):
    #    processor = cls()
    #    X = processor(df, target_cols=target_cols)
    #    if len(ax) == 0:
    #        fig, ax = plt.subplots(2, 1, figsize=(10, 10))
    #    sns.lineplot(
    #        data=df.reset_index(),
    #        x="dt",
    #        y=target_cols[0],
    #        ax=ax[0],
    #        label=target_cols[0],
    #    )
    #    ax[0].set(xlabel="")
    #    sns.lineplot(
    #        data=df.reset_index(),
    #        x="dt",
    #        y=target_cols[1],
    #        ax=ax[0],
    #        label=target_cols[1],
    #    )
    #    sns.lineplot(
    #        np.arange(0, len(X)),
    #        X.squeeze().numpy(),
    #        label="_".join([str(t) for t in target_cols]),
    #        ax=ax[1],
    #    )
    #    ax[0].set_title("Raw data")
    #    ax[1].set_title("Convolved Kernel")

    #def differentiate(self, series: torch.Tensor):
    #    diff = ((series[1:] - series[:-1]).abs() + 1).log()
    #    return self.roll_convolution(diff, aggregation="mean")
