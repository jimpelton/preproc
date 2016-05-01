#! /bin/bash

for sz in {1..32}; do
build/preproc \
--in-file "/mnt/4tb/VolumeData/INL/Josh_Kane/hop_flower/9-13-13 CT Volume Hop Flower-256.raw" \
--dat-file "/mnt/4tb/VolumeData/INL/Josh_Kane/hop_flower/9-13-13 CT Volume Hop Flower-256.dat" \
--outfile-prefix hop_flower-256 \
--output-format ascii \
--generate \
--nbx "$sz" \
--nby "$sz" \
--nbz "$sz" \
--buffer-size 64M 
done


