#!/usr/bin/env python3

""" Plot the perftests. """

import argparse
import glob
import os.path

import matplotlib.pylab
import numpy

# Constants for libcperciva's perftests
_COLUMN_NAMES = ["blocksizes", "speed MB/s"]


def get_left_right(num_dots_bundle, num, num_bundles, width):
    """ Return the start and end positions for plotting data. """
    num_pos = (num_dots_bundle + 1) * num_bundles - 1

    # Center if there's only one position in this bundle.
    if num_pos == 1:
        return 0.5, 0.5

    # Normal bounds.
    span = width / (num_pos - 1)
    left = num * (num_dots_bundle + 1) * span
    right = left + (num_dots_bundle - 1) * span
    return left, right


def _plot_boxplot(data, pos, widths, num, num_bundles, color):
    """ Plot the data as box plot. """
    # Assume that all data bundles have the same number of items.
    num_dots_bundle = len(data[next(iter(data))])

    left, right = get_left_right(num_dots_bundle, num, num_bundles, widths)
    centers = (left + right) / 2

    # Convert to a list, sorted by key.
    data_list = []
    for key in sorted(data):
        data_list.append(data[key])

    # Actual plot command.
    boxplot = matplotlib.pylab.boxplot(data_list,
                                       positions=pos + centers,
                                       widths=widths / (num_bundles + 1))

    # Set the colour.
    for elem in ["boxes", "caps", "whiskers", "fliers", "means", "medians"]:
        matplotlib.pylab.setp(boxplot[elem], color=color)


def _plot_dots(data, pos, widths, num, num_bundles, color):
    """ Plot the data as dots. """
    # Assume that all data bundles have the same number of items.
    num_dots_bundle = len(data[next(iter(data))])
    left, right = get_left_right(num_dots_bundle, num, num_bundles, widths)

    # Actual plot command.
    for band_num, key in enumerate(sorted(data)):
        spread = numpy.linspace(left, right, num_dots_bundle)
        matplotlib.pylab.plot(pos[band_num] + spread, data[key],
                              ".", color=color)


def plot(args, data, num, fig, key):
    """ Plot the perftest output.

        The calling function is responsible for initializing 'fig',
        and calling .show() or .savefig().
    """
    sizes = sorted(data.keys())
    total = len(args.filenames)

    # Grey bands
    band_pos = numpy.array(range(len(sizes) + 1)) * 2 + 1
    band_widths = band_pos[1:] - band_pos[:-1] - 1
    band_centers = band_pos[:-1] + band_widths / 2

    for i in range(int(len(band_pos) - 1)):
        fig.axes[0].axvspan(band_pos[i] - 0.25,
                            band_pos[i] + band_widths[i] + 0.25,
                            color="grey", alpha=0.1)

    color = matplotlib.rcParams['axes.prop_cycle'].by_key()['color'][num]

    if args.boxplot:
        _plot_boxplot(data, band_pos[:-1], band_widths, num, total, color)
    else:
        _plot_dots(data, band_pos, band_widths, num, total, color)

    patch = matplotlib.patches.Patch(color=color, label=key)

    matplotlib.pylab.xlim(0, max(band_pos))

    matplotlib.pylab.xticks(band_centers, sizes)

    matplotlib.pylab.xlabel(_COLUMN_NAMES[0])
    matplotlib.pylab.ylabel(_COLUMN_NAMES[1])
    return patch


def load_file(filename):
    """ Load data from a file. """
    full = numpy.loadtxt(filename, ndmin=2)

    # Extract the columns, and consolidate data
    data_set = {}
    for row in full:
        key = int(row[0])
        value = row[1]
        if key not in data_set:
            data_set[key] = []
        data_set[key].append(value)

    return data_set


def parse_cmdline():
    """Parses the command line arguments."""
    parser = argparse.ArgumentParser(description="Plot the results of a " +
                                     "performance test")
    parser.add_argument("-b", "--boxplot", action="store_true",
                        help="Draw results in a box plot")
    parser.add_argument("--log-y", action="store_true",
                        help="Use a log-y axis")
    parser.add_argument("--sort-tags", action="store_true",
                        help="Sort filenames to put tags adjacent")
    parser.add_argument("filenames", metavar="FILENAME",
                        nargs="*",
                        help="Filenames which contain the results")

    return parser.parse_args()


def main():
    """ Plot the perftests. """
    args = parse_cmdline()

    # Find filenames (if not specified).
    if not args.filenames:
        args.filenames = sorted(glob.glob("perftest-*.txt"))

    # Find the longest common prefix of filenames.
    prefix = os.path.commonprefix([os.path.splitext(s)[0]
                                  for s in args.filenames])

    # Sort filenames to put tags adjacent (if requested).
    if args.sort_tags:
        def sortfunc(x):
            filename_no_leading = os.path.splitext(x[len(prefix):])[0]
            filename_split = filename_no_leading.split("-")
            # Put the tag at the end, for sorting.  Assume that the tags
            # don't contain any hyphens.
            key = "-".join(filename_split[1:] + [filename_split[0]])
            return key
        args.filenames = sorted(args.filenames, key=sortfunc)

    # Create the figure and basic initialization.
    fig, ax = matplotlib.pylab.subplots()
    ax.ticklabel_format(useOffset=False, axis="y")
    if args.log_y:
        matplotlib.pylab.yscale('log')

    # Add title.
    if prefix.endswith("-"):
        title = prefix[:-1]
    else:
        title = prefix
    matplotlib.pylab.title(title)

    # Plot perftest data.
    handles = []
    for i, filename in enumerate(args.filenames):
        data = load_file(filename)

        # Adjust the legend key.
        key = os.path.basename(filename)[len(prefix):-4]
        if not key:
            key = title

        # Actual plot.
        handle = plot(args, data, i, fig, key)
        handles.append(handle)

    # Add the legend.
    matplotlib.pylab.legend(handles=handles, loc="best")

    # Show.
    matplotlib.pylab.show()


if __name__ == "__main__":
    main()
