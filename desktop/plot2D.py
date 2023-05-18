import matplotlib
matplotlib.use('TkAgg')
import serial
import re
import matplotlib.pyplot as plt
import numpy as np
from filterpy.kalman import KalmanFilter

# Öffne die serielle Schnittstelle
ser = serial.Serial('/dev/ttyUSB0', 115200)

# Leere Listen für die Datenpunkte
distances = []
times = []
smoothed_distances = []

# Regex-Ausdruck zur Suche nach der Entfernung im String
pattern = re.compile(r'DIST:\s+(\d+\.\d+)\s+m')

# Initialisiere den Plot
plt.ion()
fig, (ax1, ax2) = plt.subplots(2, 1, sharex=False)
text = ax1.text(0.05, 0.90, '', transform=ax1.transAxes)

# Erstelle einen leeren Histogramm-Plot
bins = np.linspace(0, 5, 50)
hist, _, _ = ax2.hist([], bins=bins)

# Erstelle einen leeren Linienplot
line, = ax1.plot([], [])
smoothed_line, = ax1.plot([], [])
ax1.set_ylabel('Entfernung (m)')
ax1.set_xlabel('Sequenznummer')

# Kalman Filter initialisieren
kf = KalmanFilter(dim_x=1, dim_z=1)
kf.x = np.array([0.0])  # initial state
kf.F = np.array([[1.]])  # state transition matrix
kf.H = np.array([[1.]])  # measurement function
kf.P *= 10.  # covariance matrix
kf.R = 30  # measurement noise

# Schleife zum Lesen der seriellen Daten
while True:
    # Lese eine Zeile von der seriellen Schnittstelle
    serial_line = ser.readline().decode().strip()
    # Versuche, die Entfernung aus der Zeile zu extrahieren
    match = pattern.search(serial_line)
    if match:
        # Extrahiere den Float-Wert der Entfernung
        dist = float(match.group(1))
        # Füge den Datenpunkt hinzu
        distances.append(dist)
        times.append(len(distances))
        # Glätte die Entfernungsmessungen mit dem Kalman-Filter
        kf.predict()
        kf.update(dist)
        smoothed_dist = kf.x[0]
        smoothed_distances.append(smoothed_dist)
        # Aktualisiere den Linienplot
        line.set_data(times, distances)
        smoothed_line.set_data(times, smoothed_distances)
        ax1.relim()
        ax1.autoscale_view()
        # Aktualisiere das Textfeld mit dem letzten Messwert
        text = f'Letzter Messwert: {dist:.2f} m (geglättet: {smoothed_dist:.2f} m)'
        fig.suptitle(text)
        # Aktualisiere das Histogramm
        resolution = np.linspace(min(distances)-0.1, max(distances)+0.1, 50)
        hist, resolution = np.histogram(distances, bins=resolution)
        ax2.set_xlim(resolution[0], resolution[-1])
        ax2.set_ylim(0, max(hist) + 1)
        ax2.clear()
        ax2.bar(resolution[:-1], hist, width=resolution[1]-resolution[0], align='edge')
        ax2.set_xlabel('Entfernung (m)')
        ax2.set_ylabel('Anzahl Messungen')
        plt.draw()
        plt.pause(0.001)
