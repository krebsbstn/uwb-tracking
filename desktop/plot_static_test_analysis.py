from ast import arg
import csv
import os
import argparse
import math
import json
import matplotlib.pyplot as plt
from scipy.stats import multivariate_normal
from matplotlib.patches import Ellipse
import matplotlib.transforms as transforms
from matplotlib import cm
import statistics
import numpy as np
import pandas as pd
from typing import List

# Data Structure holding the positions-Information
class PositionData:
    x = []
    y = []
    time = []

#Plot-Function, visualizes Latitude and Longitude in single subplots
def plot_x_y(positions : PositionData, path : str):
    plt.figure("x-Y-Plot")
    plt.subplot(2, 1, 1)
    plt.xlabel("Zeit [s]")
    plt.ylabel("X [m]")
    plt.plot(positions.time, positions.x)
    plt.plot(positions.time, [statistics.mean(positions.x)]*len(positions.x))
    plt.subplot(2, 1, 2)
    plt.xlabel("Zeit [s]")
    plt.ylabel("Y [m]")
    plt.plot(positions.time, positions.y)
    plt.plot(positions.time, [statistics.mean(positions.y)]*len(positions.y))
    plt.suptitle('X-Y-Plot', fontsize=16)
    plt.subplots_adjust(hspace=0.3)
    plt.savefig(path.replace(".txt", "") + "_xy_plot.png", bbox_inches='tight')

#Plot-Function, visualizes histogram for latitude and longitude
def plot_histogram(positions : PositionData, path : str):
    plt.figure("Histogram für X-Achse und Y-Achse")
    plt.subplot(2, 1, 1)
    plt.xlabel("X [m]")
    plt.ylabel("Anzahl der Positionen")
    pd.Series(positions.y).hist(bins=25)
    plt.subplot(2, 1, 2)
    plt.xlabel("Y [m]")
    plt.ylabel("Anzahl der Positionen")
    pd.Series(positions.x).hist(bins=25)
    plt.suptitle('Histogram für den Verlauf der X und Y-Koordinaten', fontsize=16)
    plt.subplots_adjust(hspace=0.3)
    plt.savefig(path.replace(".txt", "") + "_histogram_plot.png", bbox_inches='tight')

#Plot Function, allowes to plot probability distribution
def plot_probability(positions : PositionData, path : str,  real_pos : tuple):
    plt.figure("probability distribution of measurement")
    ax = plt.axes(projection ='3d')

    rel_x, rel_y = get_vectors_kartesian(zip(positions.x, positions.y), real_pos)
    rel_x = np.array(rel_x)
    rel_y = np.array(rel_y)

    #Parameters to set
    mu_x = statistics.mean(rel_x)
    mu_y = statistics.mean(rel_y)

    #Create grid and multivariate normal
    x = np.linspace(min(rel_x),max(rel_x),500)
    y = np.linspace(min(rel_y),max(rel_y),500)
    X, Y = np.meshgrid(x,y)
    pos = np.empty(X.shape + (2,))
    pos[:, :, 0] = X
    pos[:, :, 1] = Y
    rv = multivariate_normal([mu_x, mu_y], np.cov(rel_x,rel_y))

    my_col = cm.jet(rv.pdf(pos)/np.amax(rv.pdf(pos)))
    ax.plot_surface(X, Y, rv.pdf(pos),cmap='viridis',facecolors=my_col,linewidth=0)
    ax.set_xlabel('X-Koordinate')
    ax.set_ylabel('Y-Koordinate')
    ax.set_zlabel('Wahrscheinlichkeit')
    ax.set_title('Wahrscheinlichkeitsverteilung')
    plt.savefig(path.replace(".txt", "") + "_probability_distribution.png", bbox_inches='tight')

#Plot Function, plots the sigma Ellipses
def plot_sigma_ellipses(positions : PositionData, path : str, real_pos : tuple):
    plt.figure("Sigma-Ellipsen")
    ax_nstd = plt.axes()
    x,y = get_vectors_kartesian(zip(positions.x, positions.y), real_pos)
    x =  np.array(x)
    y =  np.array(y)
    ax_nstd.scatter(x, y, s=0.25)

    confidence_ellipse(x, y, ax_nstd, n_std=1,
                   label=r'$1\sigma=68\%$', edgecolor='firebrick')
    confidence_ellipse(x, y, ax_nstd, n_std=2,
                   label=r'$2\sigma=95.5\%$', edgecolor='fuchsia', linestyle='--')
    confidence_ellipse(x, y, ax_nstd, n_std=3,
                   label=r'$3\sigma=99.7\%$', edgecolor='blue', linestyle=':')

    ax_nstd.scatter(statistics.mean(x), statistics.mean(y), c='red', s=5)
    ax_nstd.set_title('Sigma-Ellipsen')
    ax_nstd.axhline(y=0, linewidth=1, color='k')
    ax_nstd.axvline(x=0, linewidth=1, color='k')
    ax_nstd.legend()
    ax_nstd.set_xlabel("X [m]")
    ax_nstd.set_ylabel("Y [m]")
    plt.savefig(path.replace(".txt", "") + "_ellipses.png", bbox_inches='tight')

#Plot-Function, visualizes Deviation Length and Angle in single subplots
def plot_deviation(positions : PositionData, path : str, real_pos : tuple):
    measured_pos = zip(positions.x, positions.y)
    plt.figure("Abweichung der Position")
    angle, length = get_vectors_euler(measured_pos, real_pos)
    print_values(length)
    #write_csv(path, length=length, angle=angle)
    plt.subplot(2, 1, 1)
    plt.xlabel("Zeit [s]")
    plt.ylabel("Betrag der Abweichung [m]")
    plt.plot(positions.time, length)
    plt.plot(positions.time, [statistics.mean(length)]*len(length))
    plt.subplot(2, 1, 2)
    plt.xlabel("Zeit [s]")
    plt.ylabel("Winkel der Abweichung [deg]")
    plt.plot(positions.time, angle)
    plt.plot(positions.time, [statistics.mean(angle)]*len(angle))
    plt.suptitle('Abweichung der Position', fontsize=16)
    plt.subplots_adjust(hspace=0.3)
    plt.savefig(path.replace(".txt", "") + "_deviation_plot.png", bbox_inches='tight')

#Function returning two lists for angle and length compared to fixpoint
def get_vectors_kartesian(measured_pos: List[tuple], real_pos: tuple):
    x = []
    y = []
    for x_coord, y_coord in measured_pos:
        if real_pos[0] is None and real_pos[1] is None:
            dx, dy = x_coord, y_coord
        elif real_pos[0] is not None and real_pos[1] is not None:
            dx, dy = x_coord - real_pos[0], y_coord - real_pos[1]
        else:
            raise ValueError("Problems with real_pos")
        x.append(dx)
        y.append(dy)
    return x, y


#Function returning two lists for angle and length compared to fixpoint
def get_vectors_euler(measured_pos: List[tuple], real_pos: tuple):
    angles = []
    lengths = []

    for x_coord, y_coord in measured_pos:
        if real_pos[0] is None and real_pos[1] is None:
            vector = (x_coord, y_coord)
        elif real_pos[0] is not None and real_pos[1] is not None:
            dx = x_coord - real_pos[0]
            dy = y_coord - real_pos[1]
            vector = (dx, dy)
        else:
            raise ValueError("Problems with real_pos")

        absolute_len = math.sqrt(vector[0] ** 2 + vector[1] ** 2)
        angle = math.atan2(vector[1], vector[0])
        angles.append((math.degrees(angle) + 360) % 360)
        lengths.append(absolute_len)

    return angles, lengths


#Writing Function, allows to store the length and angle data into a csv file
def write_csv(path: str, length: List, angle: List) -> None:
    """Write length and angle to path as csv."""
    angles = ["angle"] + angle
    lengths = ["length"] + length

    output_path = os.path.splitext(path)[0] + "_angle_length.csv"
    with open(output_path, 'w') as file_handle:
        for index, pair in enumerate(zip(angles, lengths)):
            if index == 0:
                line = ",".join(["iterations", str(pair[0]), str(pair[1])]) + "\n"

            else:
                line = ",".join([str(index-1), str(pair[0]), str(pair[1])]) + "\n"

            file_handle.write(line)

    return None

# Helper Function, allowing to calculate sigma ellipses
def confidence_ellipse(x, y, ax, n_std=3.0, facecolor='none', **kwargs):
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

#Helper Function, converting a String number to float number
def get_float(number_str : str):
    if number_str == '':
        return 0
    else:
        return float(number_str)

# Read Function, allows the use of a text file containing {"x":0,"y":0,"z":0}-jsons per line as input, returns positions Class
def read_txt(file):
    positions = PositionData()
    i=0
    for line in file:
        json_data = json.loads(line)
        positions.x.append(json_data["x"])
        positions.y.append(json_data["y"])
        positions.time.append(i)
        i += 1

    return positions

def print_values(lengths : List):
    st_dev = statistics.pstdev(lengths)
    print("Standard deviation of the length: " + str(st_dev) + " meters")
    print("Mean deviation of the length: " + str(statistics.mean(lengths)) + " meters")
    print("Maximum deviation of the length: " + str(max(lengths)) + " meters")
    print("Minimum deviation of the length: " + str(min(lengths)) + " meters")

#Entrence Point
def main(args):
    with open(os.path.abspath(args.path)) as txtdatei:
        positions = read_txt(txtdatei)
    plot_x_y(positions, args.path)
    plot_histogram(positions,args.path)
    plot_deviation(positions, args.path, (args.x, args.y))   
    plot_probability(positions, args.path, (args.x, args.y))
    plot_sigma_ellipses(positions, args.path, (args.x, args.y))
    if (args.show):
        plt.show()


if __name__ == '__main__':
    # Create the parser
    my_parser = argparse.ArgumentParser(description=main.__doc__)

    # Add the arguments
    my_parser.add_argument(action="store",
                           dest="path",
                           type=str,
                           help="path to txt files")

    my_parser.add_argument("-s", "--show",
                           action="store_true",
                           dest="show",
                           help="an optional argument")

    my_parser.add_argument("-x", "--xaxis",
                           action="store",
                           type=float,
                           default=None,
                           dest="x",
                           help="X-Coordinate to compare to.")

    my_parser.add_argument("-y", "--yaxis",
                           action="store",
                           type=float,
                           default=None,
                           dest="y",
                           help="Y-Coordinate to compare to.")

    my_parser.set_defaults(func=main)
    arguments = my_parser.parse_args()
    arguments.func(arguments)
