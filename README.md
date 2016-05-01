# preproc
A datapreprocessor for analyzing volume data sets for empty space removal.
```
USAGE: 

   ./preproc  [--print-blocks] [--tmax <float>] [--tmin <float>] [-b
              <uint>] [--nbz <uint>] [--nby <uint>] [--nbx <uint>] [--volz
              <uint>] [--voly <uint>] [--volx <uint>] [-g] [-c] [-t <float
              |ushort|uchar>] [-d <string>] [-u <string>] [--output-format
              <ascii|binary>] [--outfile-prefix <string>] [-o <string>] [-f
              <string>] [--] [--version] [-h]


Where: 

   --print-blocks
     Print blocks into to stdout.

   --tmax <float>
     Thresh max

   --tmin <float>
     Thresh min

   -b <uint>,  --buffer-size <uint>
     Buffer size bytes

   --nbz <uint>
     Num blocks z dim

   --nby <uint>
     Num blocks y dim

   --nbx <uint>
     Num blocks x dim

   --volz <uint>
     Volume z dim.

   --voly <uint>
     Volume y dim.

   --volx <uint>
     Volume x dim.

   -g,  --generate
     Write new index file from raw file

   -c,  --convert
     Read existing index file

   -t <float|ushort|uchar>,  --type <float|ushort|uchar>
     Data type (float, ushort, uchar).

   -d <string>,  --dat-file <string>
     Path to .dat file

   -u <string>,  --tfunc <string>
     Path to transfer function file.

   --output-format <ascii|binary>
     Output file type (ascii, binary)

   --outfile-prefix <string>
     Output file name prefix.

   -o <string>,  --outfile-path <string>
     Path to output file (default is '.')

   -f <string>,  --in-file <string>
     Path to data file.

   --,  --ignore_rest
     Ignores the rest of the labeled arguments following this flag.

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.
```

