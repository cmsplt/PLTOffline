import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from adtk.detector import LevelShiftAD, PersistAD

# Create a time series with both a level shift and persistent anomalies
time_index = pd.date_range(start='2022-01-01', periods=100, freq='T')
data = np.random.normal(loc=10, scale=1, size=100)  # Normal noise
data[50:] += 5  # Introduce a level shift
data[20:30] += 5  # Introduce a persistent anomaly
data[70:80] -= 4  # Introduce another persistent anomaly

# Create a DataFrame
df = pd.DataFrame(data, index=time_index, columns=['value'])

# Create the LevelShiftAD detector
level_shift_detector = LevelShiftAD(c=3, side='both', window=10)
level_shift_detector.fit(df['value'])  # Train the detector
level_shift_anomalies = level_shift_detector.detect(df['value'])

# Create the PersistAD detector
persist_detector = PersistAD(c=2, side='both', window=5)
persist_detector.fit(df['value'])  # Train the detector
persistent_anomalies = persist_detector.detect(df['value'])

# Check for NaN values and convert to boolean
if level_shift_anomalies.isnull().all():
    print("No level shift anomalies detected.")
else:
    level_shift_anomalies = level_shift_anomalies.fillna(False)  # Convert NaNs to False
    level_shift_anomalies = level_shift_anomalies.astype(bool)    # Ensure it's boolean

if persistent_anomalies.isnull().all():
    print("No persistent anomalies detected.")
else:
    persistent_anomalies = persistent_anomalies.fillna(False)  # Convert NaNs to False
    persistent_anomalies = persistent_anomalies.astype(bool)    # Ensure it's boolean

# Plotting
plt.figure(figsize=(12, 6))
plt.plot(df['value'], label='Value', color='blue')

# Highlight Level Shift Anomalies
if level_shift_anomalies.any():
    plt.scatter(df.index[level_shift_anomalies], df['value'][level_shift_anomalies],
                color='red', label='Detected Level Shifts', zorder=5)

# Highlight Persistent Anomalies
if persistent_anomalies.any():
    plt.scatter(df.index[persistent_anomalies], df['value'][persistent_anomalies],
                color='orange', label='Detected Persistent Anomalies', zorder=5)

# True regions of anomalies
plt.axvline(x=time_index[50], color='green', linestyle='--', label='True Level Shift Point')
plt.axvspan(time_index[20], time_index[30], color='red', alpha=0.5, label='True Persistent Anomaly Region')
plt.axvspan(time_index[70], time_index[80], color='purple', alpha=0.5, label='True Persistent Anomaly Region')

plt.title('Anomaly Detection with LevelShiftAD (Red) and PersistAD (Orange)')
plt.legend()
plt.show()

