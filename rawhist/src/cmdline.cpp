#include "cmdline.h"

#include <tclap/CmdLine.h>

#include <iostream>
#include <string>

namespace rawhist
{

int
parseThem(int argc, const char *argv[], CommandLineOptions &opts)
try
{
  if (argc==1) {
    return 0;
  }

  TCLAP::CmdLine cmd("Rawhist histogram generator.", ' ');

  // volume data file
  TCLAP::ValueArg<std::string>
      fileArg("f", "file", "Path to raw data file.", false, "", "string");
  cmd.add(fileArg);

  // dat file
  TCLAP::ValueArg<std::string>
      datFilePath("d", "dat-file", "Path to .dat file.", false, "", "string");
  cmd.add(datFilePath);

  // output file path
  TCLAP::ValueArg<std::string> outputFilePath
      ("o", "output-path", "Path to save hist file to.", false, "", "string");
  cmd.add(outputFilePath);

  // volume data type
  std::vector<std::string> dataTypes{ "float", "ushort", "uchar" };
  TCLAP::ValuesConstraint<std::string> dataTypeAllowValues(dataTypes);
  TCLAP::ValueArg<std::string>
      dataTypeArg("t", "type", "Data type (float, ushort, uchar).", false, "",
                  &dataTypeAllowValues);;
  cmd.add(dataTypeArg);


  // buffer size
  const std::string sixty_four_megs = "64M";
  TCLAP::ValueArg<std::string>
      bufferSizeArg("b", "buffer-size", "Buffer size bytes", false, sixty_four_megs,
                    "uint");
  cmd.add(bufferSizeArg);

  cmd.parse(argc, argv);

  opts.rawFilePath = fileArg.getValue();
  opts.datFilePath = datFilePath.getValue();
  opts.outputFilePath = outputFilePath.getValue();
  opts.dataType = dataTypeArg.getValue();
  opts.bufferSize = convertToBytes(bufferSizeArg.getValue());

  return static_cast<int>(cmd.getArgList().size());

} catch (TCLAP::ArgException &e) {
  std::cout << "Error parsing command line args: " << e.error() << " for argument "
            << e.argId() << std::endl;
  return 0;
}


void
printThem(CommandLineOptions &opts)
{
  std::cout
      << "File path: " << opts.rawFilePath
      << "\nDat file: " << opts.datFilePath
      << "\nData Type: " << opts.dataType
      << "\nBuffer Size: " << opts.bufferSize
      << std::endl;
}


uint64_t
convertToBytes(std::string s)
{
  size_t multiplier{ 1 };
  std::string last{ *( s.end()-1 ) };

  if (last=="K") {
    multiplier = 1024;
  } else if (last=="M") {
    multiplier = 1024*1024;
  } else if (last=="G") {
    multiplier = 1024*1024*1024;
  }

  std::string numPart(s.begin(), s.end()-1);
  auto num = stoull(numPart);

  return num*multiplier;
}
} // namespace rawhist
