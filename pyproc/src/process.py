import dask.array as da
import numpy as np



class Process(object):
    def __init__(self, infile, dtype=np.byte, vol_dims=(1,1,1)):
        self.infile = infile
        self.vol_dims = vol_dims
        self.dtype = dtype
        self.first_run = True

    def _start(self, data):
        if self.first_run:
            volsum = data.sum()
            print(volsum)

    def process(self, blocks=(1,1,1)):
        mm = np.memmap(self.infile, dtype=self.dtype, mode='r', shape=self.vol_dims)
        chunk_shape = (self.vol_dims[0] / blocks[0],
                       self.vol_dims[1] / blocks[1],
                       self.vol_dims[2] / blocks[2])

        data = da.from_array(mm, chunks=chunk_shape)

        self._start(data)




