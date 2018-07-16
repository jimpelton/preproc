import argparse


def parse():
    parser = argparse.ArgumentParser(description='Process 3D volumetric data sets')

    parser.add_argument("filename", type=str, default='',
                        help='Input file')

    parser.add_argument('-o', '--ofile', type=str, default='',
                        help='Output file path')

    parser.add_argument('--blocks', type=lambda x: x.split(','), default='1,1,1',
                        help='Process file in x by y by z blocks')

    parser.add_argument('--dims', type=lambda x: x.split(','), default='1,1,1',
                        help='Input file voxel dimensions')

    parser.add_argument('-t', '--dtype', type=str, default='byte',
                        help='Data type')

    parser.add_argument('--debug', action='store_true', default=False,
                        help='Debug/verbose logging')

    return parser.parse_args()


