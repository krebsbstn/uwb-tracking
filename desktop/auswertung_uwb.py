# -*- coding: utf-8 -*-
"""
Spyder Editor

This is a temporary script file.
"""

import json
import math
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Ellipse
import matplotlib.transforms as transforms

base_path = "C:\\Users\\Tom Niclas Herter\\Desktop\\Hochschule\\Projektarbeit\\uwb-tracking\\documentation\\logdata\\grid_meas\\"  # Change this to the path of your text file

file_name = "2mx2mx0m_3min" + ".txt"

file_path = base_path + file_name

pathlist = []


for i in range(2, 7):
    for j in range(2, 9):
        filename = str(i) + "mx"
        filename = filename + str(j) + "mx0m_3min.txt" 
        filename = base_path + filename
        pathlist.append(filename)
    

measurements = []

def open_text_file(file_path):
    try:
        coordinates_list = []
        with open(file_path, 'r') as file:
            for line in file:
                data = json.loads(line)
                coordinates_list.append([data['x'], data['y'], data['z']])
        return coordinates_list
    except FileNotFoundError:
        print("File not found.")
        return None
    except json.JSONDecodeError as e:
        print("Error decoding JSON:", e)
        return None
    except Exception as e:
        print("An error occurred:", e)
        return None


def calculate_meas(file_path, x_set, y_set):

    coordinates = open_text_file(file_path)
    
    measurements.append(coordinates)
    
    #if coordinates:
    #    print("List of lists with x, y, z coordinates:")
    #    for sublist in coordinates:
    #        print(sublist)
    
    distances = []
    
    for i in range(0, len(coordinates)):
        distance = math.sqrt((x_set - coordinates[i][0])**2 + (y_set - coordinates[i][1])**2)
        distances.append(distance)
    
    
    mittelwert = np.mean(distances)
    
    print("X soll:" + str(x_set))
    print("Y soll:" + str(y_set))

    print("Mittelwert:" + str(round(mittelwert*100, 2)))
    
    
    sigma = 0
    
    for i in range(0, len(distances)):
        sigma = sigma + (distances[i] - mittelwert)**2
        
    sigma = sigma/len(distances)
    sigma = math.sqrt(sigma)
    
    print("Stabw:" + str(round(sigma*100, 2)))
    print()



def confidence_ellipse(x, y, ax, n_std=2.0, facecolor='none', **kwargs):
    """
    Create a plot of the covariance confidence ellipse of *x* and *y*.

    Parameters
    ----------
    x, y : array-like, shape (n, )
        Input data.

    ax : matplotlib.axes.Axes
        The axes object to draw the ellipse into.

    n_std : float
        The number of standard deviations to determine the ellipse's radiuses.

    **kwargs
        Forwarded to `~matplotlib.patches.Ellipse`

    Returns
    -------
    matplotlib.patches.Ellipse
    """
    if x.size != y.size:
        raise ValueError("x and y must be the same size")

    cov = np.cov(x, y)
    pearson = cov[0, 1]/np.sqrt(cov[0, 0] * cov[1, 1])
    # Using a special case to obtain the eigenvalues of this
    # two-dimensional dataset.
    ell_radius_x = np.sqrt(1 + pearson)
    ell_radius_y = np.sqrt(1 - pearson)
    ellipse = Ellipse((0, 0), width=ell_radius_x * 2, height=ell_radius_y * 2,
                      facecolor=facecolor, **kwargs)

    # Calculating the standard deviation of x from
    # the squareroot of the variance and multiplying
    # with the given number of standard deviations.
    scale_x = np.sqrt(cov[0, 0]) * n_std
    mean_x = np.mean(x)

    # calculating the standard deviation of y ...
    scale_y = np.sqrt(cov[1, 1]) * n_std
    mean_y = np.mean(y)

    transf = transforms.Affine2D() \
        .rotate_deg(45) \
        .scale(scale_x, scale_y) \
        .translate(mean_x, mean_y)

    ellipse.set_transform(transf + ax.transData)
    return ax.add_patch(ellipse)


counter = 0
for x_set in range(2, 7):
    for y_set in range(2, 9):
        calculate_meas(pathlist[counter], x_set, y_set)
        if counter > len(pathlist):
            break
        counter = counter + 1




fig, ax = plt.subplots()
ax.set_xlim(0, 7)  # Example range for x-axis
ax.set_ylim(0, 9)  # Example range for y-axis

for meas_num in range(0, len(measurements)):
    
    x_vec = []
    y_vec = []
    
    for i in range(0, len(measurements[meas_num])):
        x_vec.append(measurements[meas_num][i][0])
        y_vec.append(measurements[meas_num][i][1])


    x = x_vec
    y = y_vec
    
    x_grid = np.linspace(2, 6, 5)
    y_grid = np.linspace(2, 8, 7)
    
    #ax.scatter(x, y, s=20, color='blue')  # Scatter plot of your data
    
    confidence_ellipse(np.array(x), np.array(y), ax, facecolor='blue', alpha=0.6) 
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.title('Confidence Ellipses')
    plt.grid(True)

for i in range(0, len(y_grid)):
    for j in range(0, len(x_grid)):
        ax.scatter(x_grid[j], y_grid[i], s=20, color='red', marker="x")


ax.scatter(0.81, 3.62, s=20, color='green', marker="o")
ax.scatter(0.81, 6.36, s=20, color='green', marker="o")
ax.scatter(6.31, 7.66, s=20, color='green', marker="o")
ax.scatter(6.72, 3.65, s=20, color='green', marker="o")
ax.scatter(2.77, 0.07, s=20, color='green', marker="o")

#ax.scatter(x_grid[j], y_grid[i], s=20, color='black', marker="x")

plt.savefig(r"C:\Users\Tom Niclas Herter\Desktop\Hochschule\Projektarbeit\uwb-tracking\documentation\UWB_System_Paper\pic\position_plot.pdf", format='pdf')
plt.show()










