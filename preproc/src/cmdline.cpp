#include "cmdline.h"

#include <tclap/CmdLine.h>

#include <iostream>
#include <string>
#include <limits>

namespace preproc
{

int
parseThem(int argc, const char *argv[], CommandLineOptions &opts)
try
{
  if (argc == 1) {
    return 0;
  }

  TCLAP::CmdLine cmd("Preprocessor.", ' ');


  // volume data file
  TCLAP::ValueArg<std::string>
      fileArg("f", "in-file", "Path to data file.", false, "", "string");
  cmd.add(fileArg);


  //output file
  TCLAP::ValueArg<std::string>
      outFileDirArg("o",
                    "outfile-dir",
                    "Directory to write output file into (default is '.')",
                    false,
                    ".",
                    "string");
  cmd.add(outFileDirArg);


  // rmap output path
  TCLAP::ValueArg<std::string>
      rmapFilePathArg("r",
                      "rmap-outfile",
                      "File to write the rmap to (default is 'rmap-temp.bin'). Suggest that "
                          "the rmap is written to a separate physical disk from the "
                          "input raw file.",
                      false,
                      "rmap-temp.bin",
                      "string");
  cmd.add(rmapFilePathArg);



  //output file prefix
  TCLAP::ValueArg<std::string>
      outFilePrefixArg("",
                       "outfile-prefix",
                       "Output file name prefix.",
                       false,
                       "",
                       "string");
  cmd.add(outFilePrefixArg);

  // ouput file type
//  std::vector< std::string > outputTypes{ "ascii", "binary" };
//  TCLAP::ValuesConstraint< std::string > outputTypesAllowValues(outputTypes);
//  TCLAP::ValueArg< std::string > outputTypeArg
//      ("", "output-format", "Output file type (ascii, binary)", false, "",
//          &outputTypesAllowValues);
//  cmd.add(outputTypeArg);


  // transfer function file
  TCLAP::ValueArg<std::string>
      tfuncArg("u", "tfunc", "Path to transfer function file.", false, "", "string");
  cmd.add(tfuncArg);


  // .dat file
  TCLAP::ValueArg<std::string>
      datFileArg("d", "dat-file", "Path to .dat file", false, "", "string");
  cmd.add(datFileArg);

  // volume data type
  std::vector<std::string> dataTypes{ "float", "ushort", "uchar" };
  TCLAP::ValuesConstraint<std::string> dataTypeAllowValues(dataTypes);
  TCLAP::ValueArg<std::string>
      dataTypeArg("t", "type", "Data type (float, ushort, uchar).", false, "",
                  &dataTypeAllowValues);
  cmd.add(dataTypeArg);

  // convert bin to ascii flag
  TCLAP::SwitchArg readArg("c", "convert", "Read existing binary index file and "
      "convert to ascii");
  cmd.add(readArg);


  // volume dims
  TCLAP::ValueArg<size_t> xdimArg("", "volx", "Volume x dim.", false, 1, "uint");
  cmd.add(xdimArg);

  TCLAP::ValueArg<size_t> ydimArg("", "voly", "Volume y dim.", false, 1, "uint");
  cmd.add(ydimArg);

  TCLAP::ValueArg<size_t> zdimArg("", "volz", "Volume z dim.", false, 1, "uint");
  cmd.add(zdimArg);


  // num blocks
  TCLAP::ValueArg<size_t> xBlocksArg("", "nbx", "Num blocks x dim", false, 1, "uint");
  cmd.add(xBlocksArg);

  TCLAP::ValueArg<size_t> yBlocksArg("", "nby", "Num blocks y dim", false, 1, "uint");
  cmd.add(yBlocksArg);

  TCLAP::ValueArg<size_t> zBlocksArg("", "nbz", "Num blocks z dim", false, 1, "uint");
  cmd.add(zBlocksArg);


  // buffer size
  const std::string sixty_four_megs = "64M";
  TCLAP::ValueArg<std::string> bufferSizeArg
      ("b", "buffer-size", "Buffer size bytes", false, sixty_four_megs, "uint");
  cmd.add(bufferSizeArg);


  // block ratio of visibility threshold min/max
  TCLAP::ValueArg<float>
      blockROV_Min_Arg("", "block-rov-min", "Block ratio-of-visibility min", true,
                     std::numeric_limits<float>::lowest(), "float");
  cmd.add(blockROV_Min_Arg);

  TCLAP::ValueArg<float>
      blockROV_Max_Arg("", "block-rov-max", "Block ratio-of-visibility max", true,
                     std::numeric_limits<float>::max(), "float");
  cmd.add(blockROV_Max_Arg);


  // voxel opacity relevance threshold
  TCLAP::ValueArg<float>
    voxelOpacityRelevance_Min_Arg("", "voxel-opacity-min", "Voxel opacity relevance minimum threshold", true,
                                0.0, "float");
  cmd.add(voxelOpacityRelevance_Min_Arg);


  // voxel opacity relevance threshold
  TCLAP::ValueArg<float>
      voxelOpacityRelevance_Max_Arg("", "voxel-opacity-max", "Voxel opacity relevance maximum threshold", true,
                                  1.0, "float");
  cmd.add(voxelOpacityRelevance_Max_Arg);


  // volume minimum value
  TCLAP::ValueArg<double>
      volMinArg("", "vol-min", "Volume min. Default is MIN_DOUBLE", false,
                std::numeric_limits<double>::lowest(), "double");
  cmd.add(volMinArg);


  // volume maximum value
  TCLAP::ValueArg<double>
      volMaxArg("", "vol-max", "Volume max. Default is MAX_DOUBLE", false,
                std::numeric_limits<double>::max(), "double");
  cmd.add(volMaxArg);


  // print blocks
  TCLAP::SwitchArg
      printBlocksArg("", "print-blocks", "Print blocks into to stdout.", cmd, false);

  cmd.parse(argc, argv);

  opts.actionType = readArg.getValue() ? ActionType::Convert : ActionType::Generate;
  opts.inFile = fileArg.getValue();
  opts.outFilePath = outFileDirArg.getValue();
  opts.outFilePrefix = outFilePrefixArg.getValue();
  opts.rmapFilePath = rmapFilePathArg.getValue();
  opts.tfuncPath = tfuncArg.getValue();
  opts.datFilePath = datFileArg.getValue();
  opts.dataType = dataTypeArg.getValue();
  opts.printBlocks = printBlocksArg.getValue();
  opts.vol_dims[0] = xdimArg.getValue();
  opts.vol_dims[1] = ydimArg.getValue();
  opts.vol_dims[2] = zdimArg.getValue();
  opts.num_blks[0] = xBlocksArg.getValue();
  opts.num_blks[1] = yBlocksArg.getValue();
  opts.num_blks[2] = zBlocksArg.getValue();
  opts.bufferSize = convertToBytes(bufferSizeArg.getValue());
  opts.blockThreshold_Min = blockROV_Min_Arg.getValue();
  opts.blockThreshold_Max = blockROV_Max_Arg.getValue();
  opts.voxelOpacityRel_Max = voxelOpacityRelevance_Max_Arg.getValue();
  opts.voxelOpacityRel_Min = voxelOpacityRelevance_Min_Arg.getValue();
  opts.volMin = volMinArg.getValue();
  opts.volMax = volMaxArg.getValue();

  return static_cast<int>(cmd.getArgList().size());

} catch (TCLAP::ArgException &e) {
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
  os << "Action type: " << ( opts.actionType == ActionType::Convert ?
                             "Convert" : "Generate" )
     << "\n" "Input file path: " << opts.inFile
     << "\n" "Output file path: " << opts.outFilePath
     << "\n" "Output file prefix: " << opts.outFilePrefix
     << "\n" "RMap output file path: " << opts.rmapFilePath
     << "\n" "Transfer function path: " << opts.tfuncPath
     << "\n" "Dat file: " << opts.datFilePath
     << "\n" "Data Type: " << opts.dataType
     << "\n" "Vol dims (w X h X d): "
     << opts.vol_dims[0] << " X "
     << opts.vol_dims[1] << " X "
     << opts.vol_dims[2]
     << "\n" "Num blocks (x X y X z): "
     << opts.num_blks[0] << " X "
     << opts.num_blks[1] << " X "
     << opts.num_blks[2]
     << "\n" "Buffer Size: " << opts.bufferSize << " bytes."
     << "\n" "Block ratio of vis. min/max: " << opts.blockThreshold_Min << " - "
     << opts.blockThreshold_Max
     << "\n" "Voxel opacity rel. min/max: " << opts.voxelOpacityRel_Min << " - "
     << opts.voxelOpacityRel_Max
     << "\n" "Volume min/max : " << opts.volMin << " - " << opts.volMax
     << "\n" "Print blocks: " << std::boolalpha
     << opts.printBlocks;

  return os;
}

} // namespace preproc
