#ifndef rawhist_cmdline_h__
#define rawhist_cmdline_h__

#include <string>

namespace rawhist
{
struct CommandLineOptions
{
  // raw file path
  std::string rawFilePath;
  // for .dat descriptor file (currently unimplemented)
  std::string datFilePath;
  // volume data type
  std::string dataType;
  // buffer size
  uint64_t bufferSize;
};


///////////////////////////////////////////////////////////////////////////////
/// \brief Parses command line args and populates \c opts.
///
/// If non-zero arg was returned, then the parse was successful, but it does 
/// not mean that valid or all of the required args were provided on the 
/// command line.
///
/// \returns 0 on parse failure, non-zero if the parse was successful.
///////////////////////////////////////////////////////////////////////////////
int parseThem(int argc, const char* argv[], CommandLineOptions& opts);




void printThem(CommandLineOptions&);

uint64_t convertToBytes(std::string s);
} // namespace rawhist

#endif // rawhist_h__
