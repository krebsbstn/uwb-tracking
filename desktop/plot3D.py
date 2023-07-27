import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import serial
import re

# Verbindung zum Serial-Port herstellen
ser = serial.Serial('/dev/ttyUSB0', 115200)  # Anpassen des Portnamens und Baudraten

# Landmarkenpositionen
landmarkPositions = [
    [0.0, -4.0, 4.0],
    [0.0, 0.0, 4.0],
    [0.0, 4.0, 4.0],
    [4.0, 2.0, 4.0],
    [4.0, -2.0, 4.0]
]


# Leere Listen für die Datenpunkte
groundTruth = []
estimatedPosition = []

# Regex-Ausdruck zur Suche nach der Entfernung im String
pattern = re.compile(r"estimate: ([\d.-]+), ([\d.-]+), ([\d.-]+)&real: ([\d.-]+), ([\d.-]+), ([\d.-]+)")
eof = re.compile(r"end")

# Plot der Landmarkenpositionen
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.scatter3D(*zip(*landmarkPositions), c='r', marker='o')

# Achsenbeschriftungen
ax.set_xlabel('X-Achse')
ax.set_ylabel('Y-Achse')
ax.set_zlabel('Z-Achse')

# Titel und Legende
ax.set_title('Positionsschätzung')
ax.legend(['Landmarken', 'Zusätzliche Punkte'])

# Achsenverhältnis einstellen
ax.set_box_aspect([1, 1, 1])

# Plot anzeigen
plt.grid(True)
plt.tight_layout()

# Koordinaten für den Track speichern
track_x = []
track_y = []
track_z = []

# Schleife zum Lesen der seriellen Daten
while True:
    # Lese eine Zeile von der seriellen Schnittstelle
    serial_line = ser.readline().decode().strip()
    print(serial_line)
    
    # Versuche, die Entfernung aus der Zeile zu extrahieren
    match = pattern.search(serial_line)
    if match:
        # Koordinaten aus den Match-Gruppen extrahieren
        estimate_coordinates = [float(coord) for coord in match.group(1, 2, 3)]
        real_coordinates = [float(coord) for coord in match.group(4, 5, 6)]
        
        # Aktuelle Position zum Track hinzufügen
        # track_x.append(real_coordinates[0])
        # track_y.append(real_coordinates[1])
        # track_z.append(real_coordinates[2])
        
        # Plot des gesamten Tracks
        # ax.plot3D(track_x, track_y, track_z, 'g')
        
        ## Aktuelle Position plotten
        ax.scatter3D(*real_coordinates, c='b', marker='o')
        ax.scatter3D(*estimate_coordinates, c='g', marker='o')
        
        plt.draw()
        plt.pause(0.001)

    end = eof.search(serial_line)
    if end:
        plt.show()
        while 1:{}