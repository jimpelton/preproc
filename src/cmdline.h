#ifndef preproc_cmdline_h__
#define preproc_cmdline_h__

#include <string>

enum class ActionType {
  Convert,  ///< Convert binary to ascii
  Generate  ///< Generate a new binary or ascii index file
};

enum class OutputType {
  Ascii, Binary
};

struct CommandLineOptions {
  // raw file path
  std::string inFile;
  // output file path
  std::string outFilePath;
  // output file prefix
  std::string outFilePrefix;
  // output file type
  OutputType outputFileType;
  // transfer function file path
  std::string tfuncPath;
  // for .dat descriptor file (currently unimplemented)
  std::string datFilePath;
  // volume data type
  std::string dataType;
  // Action to perform (write/read index file)
  ActionType actionType;
  // true if block data should be dumped to file
  bool printBlocks;
  // number of blocks
  uint64_t num_blks[3];
  // volume dimensions
  uint64_t vol_dims[3];
  // buffer size
  uint64_t bufferSize;
  // threshold max
  float tmax;
  // threshold minimum
  float tmin;
};

size_t
convertToBytes(std::string s);

///////////////////////////////////////////////////////////////////////////////
/// \brief Parses command line args and populates \c opts.
///
/// If non-zero arg was returned, then the parse was successful, but it does 
/// not mean that valid or all of the required args were provided on the 
/// command line.
///
/// \returns 0 on parse failure, non-zero if the parse was successful.
///////////////////////////////////////////////////////////////////////////////
int 
parseThem(int argc, const char *argv[], CommandLineOptions &opts);

void 
printThem(const CommandLineOptions &);

std::ostream& 
operator<<(std::ostream&, const CommandLineOptions &);

#endif // preproc_cmdline_h__
