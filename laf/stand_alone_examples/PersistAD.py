#PersistAD
#
#Logic:
#PersistAD detects persistent anomalies, which are deviations from the expected value that persist over a specified duration.
#It is useful for identifying outliers that remain high or low for multiple consecutive time points, indicating a sustained anomaly rather than a transient spike.
#
#Parameters:
#c: The threshold for considering a value as anomalous. This can be based on standard deviations from the mean.
#side: Indicates whether to look for positive, negative, or both types of deviations.
#window: The number of periods that the anomaly must persist to be considered significant.

import numpy as np
import pandas as pd
from adtk.detector import PersistAD
import matplotlib.pyplot as plt

# Generate sample time series data with persistent anomalies
data = np.random.normal(loc=10, scale=1, size=100)  # Normal noise
data[20:30] += 5  # Introduce a persistent anomaly
data[70:80] -= 4  # Introduce another persistent anomaly

# Create a DataFrame
time_index = pd.date_range(start='2022-01-01', periods=100, freq='T')
df = pd.DataFrame(data, index=time_index, columns=['value'])

# Create the PersistAD detector
persist_detector = PersistAD(c=2, side='both', window=5)

# Train the detector on the data
persist_detector.fit(df['value'])  # This is the training step

# Detect persistent anomalies
persistent_anomalies = persist_detector.detect(df['value'])

# Print anomalies for debugging
print("Persistent Anomalies Detected:")
print(persistent_anomalies)

# Check for NaN values and convert to boolean
if persistent_anomalies.isnull().all():
    print("No anomalies detected.")
else:
    persistent_anomalies = persistent_anomalies.fillna(False)  # Convert NaNs to False
    persistent_anomalies = persistent_anomalies.astype(bool)    # Ensure it's boolean

    # Plotting
    plt.figure(figsize=(12, 6))
    plt.plot(df['value'], label='Value', color='blue')
    
    # Use boolean indexing to filter the anomalies
    plt.scatter(df.index[persistent_anomalies], df['value'][persistent_anomalies],
                color='orange', label='Detected Persistent Anomalies', zorder=5)

    plt.axvspan(time_index[20], time_index[30], color='red', alpha=0.5, label='True Persistent Anomaly Region')
    plt.axvspan(time_index[70], time_index[80], color='purple', alpha=0.5, label='True Persistent Anomaly Region')
    plt.title('Persistent Anomaly Detection with PersistAD')
    plt.legend()
    plt.show()
