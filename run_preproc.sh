#! /bin/bash


#build/preproc \
#--in-file "/mnt/4tb/VolumeData/INL/Josh_Kane/hop_flower/9-13-13 CT Volume Hop Flower-512.raw" \
#--dat-file "/mnt/4tb/VolumeData/INL/Josh_Kane/hop_flower/9-13-13 CT Volume Hop Flower-512.dat" \
#--outfile-prefix hop_flower-512 \
#--output-format ascii \
#--generate \
#--tmax 5.0 \
#--tmin 7.0 \
#--nbx 8 \
#--nby 8 \
#--nbz 8 \
#--buffer-size 64M 

# build/Debug/preprocessor \
# --in-file "/Users/jim/Documents/thesis/volumes/OhioU_WitmerLab_Iguana_iguana_OUVC_10709_arterial_injection/raw/Iguana_arterial_injection_695x681x1460_short.raw" \
# --dat-file "/Users/jim/Documents/thesis/volumes/hop_flower/9-13-13 CT Volume Hop Flower-512.dat" \
# --outfile-prefix iguana- \
# --output-format ascii \
# --generate \
# --tmax 12.0 \
# --tmin 10.0 \
# --nbx 32 \
# --nby 32 \
# --nbz 32 \
# --buffer-size 32M

#build_clion/Debug/preproc \
#--in-file "/Volumes/Untitled/INL/Josh_Kane/hop_flower/Hop_Flower-Resampled-3509x3787x4096.raw" \
#--dat-file "/Volumes/Untitled/INL/Josh_Kane/hop_flower/Hop_Flower-Resampled-3509x3787x4096.dat" \
#--outfile-prefix hop_flower-4096 \
#--output-format ascii \
#--generate \
#--tmax 30.0 \
#--tmin 6.0 \
#--nbx 32 \
#--nby 32 \
#--nbz 32 \
#--buffer-size 8G




build/preproc \
--in-file "/home/jim/t/volumes/INL/Josh_Kane/hop_flower/9-13-13 CT Volume Hop Flower-512.raw" \
--dat-file "/home/jim/t/volumes/INL/Josh_Kane/hop_flower/9-13-13 CT Volume Hop Flower-512.dat" \
--outfile-prefix hop_flower-512 \
--output-format binary \
--generate \
--tmax 30.0 \
--tmin 6.0 \
--nbx 2 \
--nby 2 \
--nbz 2 \
--buffer-size 64M

#build/preproc \
#--in-file "/mnt/4tb/VolumeData/INL/Josh_Kane/hop_flower/Hop_Flower-Resampled-3509x3787x4096.raw" \
#--dat-file "/mnt/4tb/VolumeData/INL/Josh_Kane/hop_flower/Hop_Flower-Resampled-3509x3787x4096.dat" \
#--outfile-prefix hop_flower-4096 \
#--output-format binary \
#--generate \
#--tmax 158.0 \
#--tmin 6.8 \
#--nbx 2 \
#--nby 2 \
#--nbz 2 \
#--buffer-size 20G

#build/preproc \
#--in-file "/mnt/4tb/VolumeData/" \
#--dat-file "/mnt/4tb/VolumeData/INL/Josh_Kane/hop_flower/Hop_Flower-Resampled-3509x3787x4096.dat" \
#--outfile-prefix hop_flower-4096 \
#--output-format ascii \
#--generate \
#--tmax 12.0 \
#--tmin 10.0 \
#--nbx 32 \
#--nby 32 \
#--nbz 32 \
#--buffer-size 20G
