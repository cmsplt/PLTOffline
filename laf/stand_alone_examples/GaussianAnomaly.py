import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import torch

# Define the AutoGaussianSlider class
class AutoGaussianSlider(torch.nn.Module):
    def __init__(self, slider_perc: float = 0.05, threshold: float = 0.01) -> None:
        super().__init__()
        self.slider_perc = slider_perc
        self.threshold = threshold

    def forward(self, X: torch.Tensor, tts: torch.Tensor) -> torch.Tensor:
        slider_stride = 1
        window_length = int(X.size()[0] * self.slider_perc)

        # Create sliding patches
        patches = X.unfold(0, window_length, slider_stride)
        t_patches = tts.unfold(0, window_length, slider_stride)

        # Calculate means and standard deviations
        means = patches.mean(dim=1)[:-1]
        stds = patches.std(dim=1)[:-1].abs() + 1e-6
        points = patches[1:, 0]
        t_points = t_patches[1:, 0]

        # Get scores
        results_ = self.p_scorer(means, stds, points)
        mask = results_.le(self.threshold)
        return t_points[mask], points[mask]

    def p_scorer(self, mean: torch.Tensor, std: torch.Tensor, point: torch.Tensor) -> torch.Tensor:
        """Estimates the probability of a point being an anomaly."""
        dist = torch.distributions.normal.Normal(mean, std)
        pbs = dist.cdf(point)
        return pbs

# Create synthetic data with anomalies
np.random.seed(42)  # For reproducibility
time_index = pd.date_range(start='2022-07-18', periods=100, freq='T')
data = np.random.normal(loc=10, scale=1, size=100)  # Normal noise
data[30:40] += 10  # Introduce an anomaly (spike)
data[70:80] -= 8   # Introduce another anomaly (drop)

# Create a DataFrame
df = pd.DataFrame(data, index=time_index, columns=['value'])

# Convert data to torch tensors
X = torch.FloatTensor(df['value'].values)
tts = torch.arange(len(X))

# Initialize the anomaly detector
detector = AutoGaussianSlider(slider_perc=0.1, threshold=0.01)

# Detect anomalies
anomaly_time_points, anomaly_values = detector(X, tts)

# Plotting
plt.figure(figsize=(12, 6))
plt.plot(df.index, df['value'], label='Value', color='blue')

# Convert anomaly_time_points to numpy array of integers
anomaly_time_points = anomaly_time_points.numpy().astype(int)

# Plot detected anomalies
plt.scatter(df.index[anomaly_time_points], anomaly_values, color='red', label='Detected Anomalies', zorder=5)

# Vertical lines for the actual anomalies in the synthetic data
plt.axvline(x=df.index[30], color='green', linestyle='--', label='Anomaly Start')
plt.axvline(x=df.index[40], color='purple', linestyle='--', label='Anomaly End')
plt.axvline(x=df.index[70], color='orange', linestyle='--', label='Drop Start')
plt.axvline(x=df.index[80], color='cyan', linestyle='--', label='Drop End')

plt.title('Anomaly Detection using AutoGaussianSlider')
plt.xlabel('Time')
plt.ylabel('Value')
plt.legend()
plt.show()
