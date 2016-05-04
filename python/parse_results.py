"""
Convenience methods for parsing the json results produced by preproc.

Jim Pelton -- May 2016
"""

import os
import json
import re

import numpy as np
# from scipy import stats
import matplotlib.pyplot as plt


def atoi(text):
    return int(text) if text.isdigit() else text


def natural_keys(text):
    '''
    alist.sort(key=natural_keys) sorts in human order
    http://nedbatchelder.com/blog/200712/human_sorting.html
    (See Toothy's implementation in the comments)
    '''
    return [atoi(c) for c in re.split('(\d+)', text)]


def length_keys(li):
    return len(li)


def all_files_data(dir_name):
    """
    Return a list with json data for all files in dir_name, and a matching list
    with all of the keys for each block of the blocks object in sorted order.
    List looks like:
      {
          'header': {..stuff..},
          'blocks': {
              'block_0': { ..block stuff.. },
              'block_1': { ..block stuff.. },
                ...
                ...
          }
      }
    Keys list is just:
       [   [ 'block_0' ],
           [ 'block_0', 'block_1' ],
              ...
           [ 'block_0', ..., 'block_N' ]
       ]
       :param dir_name: a path object for the directory to scan.
       :return a list with json data for all files in dir_name, and a matching list or sorted keys for each block
    """
    all_files_data_list = []
    all_files = os.listdir(dir_name)
    all_files.sort(key=natural_keys)
    for f in all_files:
        with open(os.path.join(dir_name, f)) as data_file:
            all_files_data_list.append(json.load(data_file))

    keys_sorted = []
    for d in all_files_data_list:
        sorted_keys = [x for x in d['blocks']]
        sorted_keys.sort(key=natural_keys)
        keys_sorted.append(sorted_keys)

    return all_files_data_list, keys_sorted


def compute_average_of_block_averages(blocks_object):
    """
    Compute the average of all the block averages in blocks_ojbect
    :param blocks_object: JSON object with a bunch of block objects.
    :return: A single average for all the block averages.
    """
    total = 0
    num_blocks = 0
    for b in blocks_object:
        total += blocks_object[b]['avg_val']  # Get avg of block with key b.
        num_blocks += 1
    return total / num_blocks


def compute_average_of_all_block_averages_for_all_the_files(all_files_data_list):
    """
    For each file, get the blocks object from all_files_data_list and compute the average of each block.
    :param all_files_data_list:
    :return:
    """
    average_of_average_values = []
    for f in all_files_data_list:
        avg = 0
        blocks_object = f['blocks']  # JSON object for all the blocks in file f.
        avg = compute_average_of_block_averages(blocks_object)
        average_of_average_values.append(avg)

    return average_of_average_values


def compute_histogram_for_block_object(block_object, num_bins):
    averages = []
    for key in block_object:
        averages.append(block_object[key]['avg_val'])

    return np.histogram(averages, bins=num_bins)


def main(args):
    # dir_name = '../json/hop_flower-512/'
    dir_name = os.path.normpath(args[1])
    all_files_data_list, keys_sorted = all_files_data(dir_name)
    average_of_average_values = compute_average_of_all_block_averages_for_all_the_files(all_files_data_list)

    plt.plot(average_of_average_values)
    plt.title('Average of block averages.')
    plt.xlabel('Block count (x^3)')
    plt.ylabel('Block average')
    plt.show()
    print('Found {} files in {}'.format(len(all_files_data_list), dir_name))


if __name__ == '__main__':
    import sys

    main(sys.argv)

# print('Num Blocks: {}, Block Size: {} Avg: {}'
#           .format(num_blocks,
#                   [x for x in map(lambda v, b : v // b, num_vox, num_blocks )],
#                   avg))


print('Done')
