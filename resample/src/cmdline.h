//
// Created by jim on 10/14/16.
//

#ifndef PREPROCESSOR_CMDLINE_H
#define PREPROCESSOR_CMDLINE_H

#include <bd/io/datatypes.h>

#include <cstdint>
#include <string>

namespace resample
{

  struct CommandLineOptions {

    // raw file path
    std::string inFilePath;
    // output file path
    std::string outFilePath;
    // dat file path
    std::string datFilePath;
    // index file path
    std::string indexFilePath;
    // volume dimensions
    uint64_t vol_dims[3];
    // volume target dimensions
    uint64_t new_vol_dims[3];
    // buffer size bytes
    uint64_t bufferSize;
    // data type
    bd::DataType dataType;

  };

size_t convertToBytes(std::string s);


///////////////////////////////////////////////////////////////////////////////
/// \brief Parses command line args and populates \c opts.
///
/// If non-zero arg was returned, then the parse was successful, but it does
/// not mean that valid or all of the required args were provided on the
/// command line.
///
/// \returns 0 on parse failure, non-zero if the parse was successful.
///////////////////////////////////////////////////////////////////////////////
int parseThem(int argc, const char * argv[], CommandLineOptions& opts);


void printThem(const CommandLineOptions&);


std::ostream& operator<<(std::ostream&, const CommandLineOptions&);

}
#endif //PREPROCESSOR_CMDLINE_H
