import os
import json
from pprint import pprint
import re
import numpy as np
from scipy import stats

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
    """
    skip_list = ['.DS_Store']
    all_files_data_list = []
    all_files = os.listdir(dir_name)
    all_files.sort(key=natural_keys)
    for f in all_files:
        if f not in skip_list:
            with open(dir_name + f) as data_file:
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
        total += blocks_object[b]['avg_val']  # Get the block and add the average value
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


def get_list_of_block_averages(blocks):
    return [blocks[x]['avg_val'] for x in blocks]


def write_percentage_removed(outFilePrefix, avg_list):
    minmin = np.min(avg_list)
    maxmax = np.max(avg_list)
    tmin_values = np.arange(minmin, maxmax, 0.01)

    nb = len(blocks)
    percentage_removed = []
    # Vary t_min between min and max
    for t_min in tmin_values:
        l = len([x for x in avg_list if x < t_min])
        p = (l/nb) * 100
        percentage_removed.append(p)

    import csv
    fileName = '{}-percentage_removed.csv'.format(outFilePrefix)
    with open(fileName, 'w') as csvfile:
        fieldnames = ['tmin', 'percent removed']
        writer = csv.writer(csvfile, delimiter=',')
        writer.writerow(fieldnames)
        writer.writerows(zip(tmin_values, percentage_removed))

    return


def main(args):
    dir_name = args[0] #'../json/hop_flower-4096/'
    outfile_prefix = args[1] #'hop-4096'
    all_files_data_list, keys_sorted = all_files_data(dir_name)
    average_of_average_values = compute_average_of_all_block_averages_for_all_the_files(all_files_data_list)

    blockCount = 16
    countIdx = blockCount - 1
    blocks = all_files_data_list[countIdx]['blocks']
    avg_list = get_list_of_block_averages(blocks)
    write_percentage_removed('{0}_{1}x{1}x{1}'.format(outfile_prefix, blockCount), avg_list)


if __name__ == '__main__':
    import sys
    main(sys.args)
