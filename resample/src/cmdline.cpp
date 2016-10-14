//
// Created by jim on 10/14/16.
//

#include "cmdline.h"

#include <tclap/CmdLine.h>

#include <iostream>

namespace resample
{

int
parseThem(int argc, const char *argv[], CommandLineOptions &opts)
try
{
  if (argc == 1) {
    return 0;
  }

  TCLAP::CmdLine cmd("Resample volume data to target dimensions.", ' ');


  // volume data file
  TCLAP::ValueArg<std::string> inFilePAthArg("f",
                                       "infile-path",
                                       "Path to data file.",
                                       true,
                                       "",
                                       "string");
  cmd.add(inFilePAthArg);


  //output file
  TCLAP::ValueArg<std::string> outFilePath("o",
                                           "outfile-path",
                                           "Directory to write output file into (default is '.')",
                                           true,
                                           ".",
                                           "string");
  cmd.add(outFilePath);

  TCLAP::ValueArg<std::string> datFileArg("d",
                                          "dat-file",
                                          "Path to .dat file",
                                          false,
                                          "",
                                          "string");
  cmd.add(datFileArg);


  // original volume dims
  TCLAP::ValueArg<size_t> x_dimArg("", "vx", "Original x dim.", false, 1, "uint");
  cmd.add(x_dimArg);

  TCLAP::ValueArg<size_t> y_dimArg("", "vy", "Original y dim.", false, 1, "uint");
  cmd.add(y_dimArg);

  TCLAP::ValueArg<size_t> z_dimArg("", "vz", "Original z dim.", false, 1, "uint");
  cmd.add(z_dimArg);


  // target volume dims
  TCLAP::ValueArg<size_t> x_dimTargetArg("", "tx", "Target x dim.", false, 1, "uint");
  cmd.add(x_dimTargetArg);

  TCLAP::ValueArg<size_t> y_dimTargetArg("", "ty", "Target y dim.", false, 1, "uint");
  cmd.add(y_dimTargetArg);

  TCLAP::ValueArg<size_t> z_dimTargetArg("", "tz", "Target z dim.", false, 1, "uint");
  cmd.add(z_dimTargetArg);


  // buffer size
  std::string const sixty_four_megs = "64M";
  TCLAP::ValueArg<std::string>
      bufferSizeArg("b", "buffer-size",
                    "Buffer size bytes. Format is a numeric value followed by "
                        "K, M, or G.\n"
                        "Values: [0-9]+[KMG].\n"
                        "Default: 64M",
                    false,
                    sixty_four_megs, "uint");
  cmd.add(bufferSizeArg);


  opts.inFilePath = inFilePAthArg.getValue();
  opts.outFilePath = outFilePath.getValue();
  opts.datFilePath = datFileArg.getValue();
  opts.vol_dims[0] = x_dimArg.getValue();
  opts.vol_dims[1] = y_dimArg.getValue();
  opts.vol_dims[2] = z_dimArg.getValue();
  opts.new_vol_dims[0] = x_dimTargetArg.getValue();
  opts.new_vol_dims[1] = y_dimTargetArg.getValue();
  opts.new_vol_dims[2] = z_dimTargetArg.getValue();

  return static_cast<int>(cmd.getArgList().size());

} catch (TCLAP::ArgException e) {

  std::cerr << "Error parsing command line args: " << e.error() << " for argument "
            << e.argId() << std::endl;
  return 0;
}

size_t
convertToBytes(std::string s)
{
  size_t multiplier{ 1 };
  std::string last{ *( s.end() - 1 ) };

  if (last == "K") {
    multiplier = 1024;
  } else if (last == "M") {
    multiplier = 1024 * 1024;
  } else if (last == "G") {
    multiplier = 1024 * 1024 * 1024;
  }

  std::string numPart(s.begin(), s.end() - 1);
  auto num = stoull(numPart);

  return num * multiplier;
}

void
printThem(const CommandLineOptions &opts)
{
  std::cout << opts << std::endl;
}


std::ostream &
operator<<(std::ostream &os, const CommandLineOptions &opts)
{
  os << "\n" "Input file path: "
      << opts.inFilePath
      << "\n" "Output file path: "
      << opts.outFilePath
      << "\n" "Dat file: "
      << opts.datFilePath
      << "\n" "Vol dims (w X h X d): "
      << opts.vol_dims[0] << " X "
      << opts.vol_dims[1] << " X "
      << opts.vol_dims[2]
      << "\n" "Target Vol dims (w X h X d): "
      << opts.new_vol_dims[0] << " X "
      << opts.new_vol_dims[1] << " X "
      << opts.new_vol_dims[2]
      << "\n" "Buffer Size: "
      << opts.bufferSize << " bytes.";

  return os;
}
} // namespace resample