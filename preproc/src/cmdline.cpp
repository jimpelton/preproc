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


  // transfer function file
  TCLAP::ValueArg<std::string> tfuncArg("u",
                                        "tfunc",
                                        "Path to transfer function file.",
                                        false,
                                        "",
                                        "string");
  cmd.add(tfuncArg);


  // .dat file
  TCLAP::ValueArg<std::string> datFileArg("d",
                                          "dat-file",
                                          "Path to .dat file",
                                          false,
                                          "",
                                          "string");
  cmd.add(datFileArg);


  TCLAP::MultiArg<std::string> numBlocksMultiArg("D",
                                                 "bdim",
                                                 "Number of blocks along each dim.",
                                                 false,
                                                 "string");
  cmd.add(numBlocksMultiArg);


  // convert bin to ascii flag
  TCLAP::SwitchArg readArg("c",
                           "convert",
                           "Read existing binary index file and convert to ascii");
  cmd.add(readArg);


  // volume dims
  TCLAP::ValueArg<size_t> xdimArg("", "volx", "Volume x dim.", false, 1, "uint");
  cmd.add(xdimArg);

  TCLAP::ValueArg<size_t> ydimArg("", "voly", "Volume y dim.", false, 1, "uint");
  cmd.add(ydimArg);

  TCLAP::ValueArg<size_t> zdimArg("", "volz", "Volume z dim.", false, 1, "uint");
  cmd.add(zdimArg);


//  // num blocks
//  TCLAP::ValueArg<size_t> xBlocksArg("",
//                                     "nbx",
//                                     "Num blocks x dim\n"
//                                         "Default: 1",
//                                     false,
//                                     1,
//                                     "uint");
//  cmd.add(xBlocksArg);
//
//  TCLAP::ValueArg<size_t> yBlocksArg("",
//                                     "nby",
//                                     "Num blocks y dim\n"
//                                         "Default: 1",
//                                     false,
//                                     1,
//                                     "uint");
//  cmd.add(yBlocksArg);
//
//  TCLAP::ValueArg<size_t> zBlocksArg("",
//                                     "nbz",
//                                     "Num blocks z dim\n"
//                                         "Default: 1",
//                                     false,
//                                     1,
//                                     "uint");
//  cmd.add(zBlocksArg);


  // buffer size
  std::string const sixty_four_megs = "64M";
  TCLAP::ValueArg<std::string>
      bufferSizeArg("b", "buffer-size",
                    "Buffer size bytes. Format is a numeric value followed by   "
                        "K, M, or G.\n"
                        "Values: [0-9]+[KMG].\n"
                        "Default: 64M",
                    false,
                    sixty_four_megs, "uint");
  cmd.add(bufferSizeArg);


  TCLAP::ValueArg<int>
    numThreadsArg("n",
                  "num-threads",
                  "Max number of threads to use.\n"
                  "Default: # cpus",
                  false,
                  0, "int");
  cmd.add(numThreadsArg);

  // print blocks
  TCLAP::SwitchArg
      printBlocksArg("", "print-blocks", "Print blocks into to stdout.", cmd, false);

  // skip rmap args
  TCLAP::SwitchArg
    skipRmapArg("", "skip-rmap", "Skip relevance mapping", cmd, false);

  cmd.parse(argc, argv);

  opts.actionType = readArg.getValue() ? ActionType::Convert : ActionType::Generate;
  opts.inFile = fileArg.getValue();
  opts.outFileDirLocation = outFileDirArg.getValue();
  opts.outFilePrefix = outFilePrefixArg.getValue();
  opts.rmapFilePath = rmapFilePathArg.getValue();
  opts.tfuncPath = tfuncArg.getValue();
  opts.datFilePath = datFileArg.getValue();
  opts.printBlocks = printBlocksArg.getValue();
  opts.skipRmapGeneration = skipRmapArg.getValue();
  opts.vol_dims[0] = xdimArg.getValue();
  opts.vol_dims[1] = ydimArg.getValue();
  opts.vol_dims[2] = zdimArg.getValue();
  opts.numBlocks = numBlocksMultiArg.getValue();
  opts.bufferSize = convertToBytes(bufferSizeArg.getValue());
  opts.numThreads = numThreadsArg.getValue();

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
     << "\n" "Input file path: "
     << opts.inFile
     << "\n" "Output file path: "
     << opts.outFileDirLocation
     << "\n" "Output file prefix: "
     << opts.outFilePrefix
     << "\n" "RMap output file path: "
     << opts.rmapFilePath
     << "\n" "Transfer function path: "
     << opts.tfuncPath
     << "\n" "Dat file: "
     << opts.datFilePath
     << "\n" "Data Type: "
     << opts.dataType
     << "\n" "Vol dims (w X h X d): "
     << opts.vol_dims[0] << " X "
     << opts.vol_dims[1] << " X "
     << opts.vol_dims[2]
     << "\n" "Num blocks (x X y X z): "
//     << opts.num_blks[0] << " X "
//     << opts.num_blks[1] << " X "
//     << opts.num_blks[2]
     << "\n" "Buffer Size: "
     << opts.bufferSize << " bytes."
//     << "\n" "Block ratio of vis. min/max: "
//     << opts.blockThreshold_Min << " - "
//     << opts.blockThreshold_Max
//     << "\n" "Voxel opacity rel. min/max: "
//     << opts.voxelOpacityRel_Min << " - "
//     << opts.voxelOpacityRel_Max
//     << "\n" "Volume min/max : "
//     << opts.volMin << " - "
//     << opts.volMax
     << "\n" "Print blocks: " << std::boolalpha
     << opts.printBlocks;

  return os;
}

} // namespace preproc
