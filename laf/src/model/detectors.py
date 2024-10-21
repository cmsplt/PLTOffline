import numpy as np
import pandas as pd
from abc import ABC, abstractmethod
from adtk.detector import (PersistAD, LevelShiftAD)
import torch
from typing import Tuple

class Detector(ABC):
    def __init__(self, **kwargs):
        super().__init__()
        self.kwargs = kwargs

    @abstractmethod
    def detect(self, data):
        pass

class EnsambleDetector(Detector):
    def __init__(self, **kwargs):
        """
        Ensamble detector for anomaly detection, uses 
        part of ADTK library and a custom Gaussian Slider.
        """
        super().__init__(**kwargs)
        self.level_shift_detector = ExternalDetector(
            detector=LevelShiftAD(c=10.0, side='both', window=5))
        self.persisent_detector = ExternalDetector(
            detector=PersistAD(c=10.0, side='positive'))
        #more methods here https://arundo-adtk.readthedocs-hosted.com/en/stable/notebooks/demo.html
        self.bump_detector = GaussianAnomalyDetector()

    def detect(self, data: pd.Series) -> dict:
        """
        Detects anomalies in the data
        
        Args:
            data (pd.Series): Data to detect anomalies. Must contain index as
            timestamps.
        
        Returns:
            dict: Dictionary with the anomalies detected by each detector
            with characteristic anomaly type, for each one the values are the
            detecting starting timestamp of the occurrence.
        """       
        anomalies = {}
        try:
            shifts = self.level_shift_detector(data)
            if len(shifts) > 0:
                anomalies["shift"] = self.to_t_int(shifts)
            jumps = self.persisent_detector(data)
            if len(jumps) > 0:
                anomalies["jumps"] = self.to_t_int(jumps)
            bumps = self.bump_detector(data)
            if len(bumps) > 0:
                anomalies["bumps"] = self.to_t_int(bumps)
        except Exception as e:
            print("Error in detector", e)
            anomalies["bumps"] = []
        return anomalies

    @staticmethod
    def to_t_int(data: pd.Series) -> dict:
        """
        Searches for anomalies in a given channel

        Args:
            channel (int): Channel to be searched
            data (pd.DataFrame): Dataframe with the data

        """
        starts = []
        timestamps = data.index.values
        timedelta_threshold = timestamps[1] - timestamps[0]
        for index, timestamp in enumerate(timestamps[1:]):
            if timestamp - timestamps[index - 1] > 5 * timedelta_threshold:
                starts.append(str(timestamp))
        return starts

class ExternalDetector(torch.nn.Module):
    """Wrapper for the external detectors."""
    def __init__(self, detector: Detector, **kwargs):
        """
        Wrapper for the external detectors.

        Args:
            detector (Detector): Detection algorithm of the external library,
            namely ADTK
        """
        super().__init__(**kwargs)
        self.detector = detector

    def forward(self, data):
        anomalies = self.detector.fit_detect(data)
        a_tt = self._anomalies_to_arrays(data.values, anomalies)
        return data.iloc[a_tt]

    def _anomalies_to_arrays(self,
                             x: np.array,
                             anomalies: pd.Series) -> np.array:
        a_tt = np.arange(len(x))[(~anomalies.isna() & anomalies > 0)]
        return a_tt

class GaussianAnomalyDetector(torch.nn.Module):
    """Wrapper for the Gaussian Anomaly Detector."""
    def __init__(self, threshold: float = 0.01) -> None:
        super().__init__()
        self.detection_algorithm = AutoGaussianSlider(threshold=threshold)

    def forward(self, data: pd.Series) -> Tuple[np.array, np.array]:
        x = data.values
        if not isinstance(x, torch.Tensor):
            x = torch.FloatTensor(x)
        a_tt, a_X = self.detection_algorithm(x, torch.arange(len(x)))
        return data.iloc[a_tt]

class AutoGaussianSlider(torch.nn.Module):
    def __init__(self,
                 slider_perc: float = 0.05,
                 threshold: float = 0.01) -> None:
        """ 
        Gaussian Slider detector for efficient anomaly detection.

        Args:
            slider_perc (float, optional): Size of the window to slide in the
            data. Defaults to 0.05.
            threshold (float, optional): Threshold to detect anomalies.
            . Defaults to 0.01.
        """
        super().__init__()
        self.slider_perc = slider_perc
        self.threshold = threshold

    def forward(self, X: torch.Tensor, tts: torch.Tensor) -> torch.Tensor:
        """Passes the data through the detector.

        Args:
            X (torch.Tensor): Array with the preprocessed difference between
            two columns.
            tts (torch.Tensor): Time to time difference between the two columns

        Returns:
            torch.Tensor: Array with the differences between the two columns.
        """
        slider_stride = 1 # Sets the sliding window to move one element at a time
        window_length = int(X.size()[0] * self.slider_perc) # Determines the size of the sliding window

        # Lets slide
        patches = X.unfold(0, window_length, slider_stride)
        t_patches = tts.unfold(0, window_length, slider_stride)
        means = patches.mean(dim=1)[:-1]
        stds = patches.std(dim=1)[:-1].abs() + 1e-6
        points = patches[1:, 0]
        t_points = t_patches[1:, 0]
        results_ = self.p_scorer(means, stds, points)
        mask = results_.le(self.threshold)
        return t_points[mask], points[mask]

    def p_scorer(
        self, mean: torch.Tensor, std: torch.Tensor, point: torch.Tensor
    ) -> torch.Tensor:
        """Estimates the probability of a point being an anomaly.

        Args:
            mean (torch.Tensor): The mean of the gaussian distribution of the
            previuos window.
            std (torch.Tensor): The standard of the gaussian distribution of
            the previuos window.
            point (torch.Tensor): Description of parameter `point`.
        Returns:
            torch.Tensor: Probability of the point being an anomaly.
        """
        dist = torch.distributions.normal.Normal(mean, std)
        pbs = dist.cdf(point)
        return pbs

    #def fast_gaussian(
    #    self, mean: torch.Tensor, std: torch.Tensor, point: torch.Tensor
    #) -> torch.Tensor:
    #    power = -0.5 * ((mean - point) / (std + 1e-6)) ** 2
    #    return torch.exp(power) / (2.5 * std + 1e-6)
