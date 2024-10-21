#LevelShiftAD

#Logic:
#LevelShiftAD detects level shifts in time series data, which are abrupt changes in the mean level of the series.
#It is particularly useful for identifying shifts that indicate a change in the underlying process generating the data.
#This method works by comparing the observed values to the expected mean, considering a specified window size and a threshold for detecting the shifts.

#Parameters:
#c: The threshold for detecting shifts. A higher value means fewer shifts are detected.
#side: Specifies which direction to detect shifts ('both', 'up', or 'down').
#window: The size of the window used for the mean calculation.

import pandas as pd
import numpy as np
from adtk.detector import LevelShiftAD
import matplotlib.pyplot as plt

# Generate sample time series data with a level shift
time_index = pd.date_range(start='2022-01-01', periods=100, freq='T')
data = np.random.normal(loc=10, scale=1, size=100)  # Normal noise
data[50:] += 5  # Introduce a level shift

# Create a DataFrame
df = pd.DataFrame(data, index=time_index, columns=['value'])

# Create the LevelShiftAD detector
detector = LevelShiftAD(c=3, side='both', window=10)

# Train the detector on the data
detector.fit(df['value'])  # This is the training step

# Detect level shifts
anomalies = detector.detect(df['value'])

# Check for NaN values in anomalies
print("Anomalies detected:")
print(anomalies)

# Ensure anomalies is a boolean array without NaN values
if anomalies.isnull().all():
    print("No anomalies detected.")
else:
    anomalies = anomalies.fillna(False)  # Convert NaNs to False
    anomalies = anomalies.astype(bool)    # Ensure it is a boolean array

    # Plotting
    plt.figure(figsize=(12, 6))
    plt.plot(df['value'], label='Value', color='blue')

    # Use boolean indexing to filter the anomalies
    plt.scatter(df.index[anomalies], df['value'][anomalies], color='red', label='Detected Level Shifts', zorder=5)

    plt.axvline(x=time_index[50], color='green', linestyle='--', label='True Level Shift Point')
    plt.title('Level Shift Detection with LevelShiftAD')
    plt.legend()
    plt.show()

