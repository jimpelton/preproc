#include "parsedat.h"
#include "ordinal.h"

#include <algorithm>
#include <functional>

#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>

#include <ctype.h>

namespace preproc
{

namespace
{
using SplitPair = std::pair<std::string, std::string>;

SplitPair splitAroundColon(const std::string& line);

std::string trim(const std::string& s, const std::string& delims);

std::string toLowerCase(const std::string& s);

void match(const SplitPair&, DatFileData& d);

void parseObjectFileName(const std::string& s, DatFileData& d);

void parseResolution(const std::string& s, DatFileData& d);

void parseFormat(const std::string& s, DatFileData& d);


///////////////////////////////////////////////////////////////////////////////
SplitPair
splitAroundColon(const std::string& line)
{
  std::string::size_type colonIdx{ line.find_first_of(':', 0) };
  if (colonIdx != std::string::npos) {
    return std::make_pair(line.substr(0, colonIdx), line.substr(colonIdx + 1));
  }

  return std::make_pair("", "");
}


///////////////////////////////////////////////////////////////////////////////
std::string
trim(const std::string& s,
     const std::string& delims = " \f\n\r\t\v")
{
  if (s.empty()) return "";

  std::string ss = s;
  //        try {
  std::string::size_type c{ s.find_first_not_of(delims) };
  if (c == std::string::npos) {
    return ss;
  }
  ss.erase(0, ss.find_first_not_of(delims));

  //if (c+1 < s.length())
  ss.erase(ss.find_last_not_of(delims) + 1);
  //        std::cout << "Trimmed: " << ss << std::endl;
  //        } catch (std::out_of_range &e) {
  //            std::cerr << "out of range: " << e.what() << std::endl;
  //            throw e;
  //        } catch (...) {
  //            throw std::exception();
  //        }
  return ss;
}

///////////////////////////////////////////////////////////////////////////////
void
match(const SplitPair& f, DatFileData& d)
{
  if (f.first.empty() || f.second.empty()) return;
  //        std::cout << "match(): first: " << f.first << ", second: " << f.second << std::endl;

  std::string ff = toLowerCase(trim(f.first));
  std::string ss = trim(f.second);
  //        std::cout << "match(): ff: " << ff << ", ss: " << ss << std::endl;

  if (ff == "objectfilename") {
    parseObjectFileName(ss, d);
  } else if (ff == "resolution") {
    parseResolution(ss, d);
  } else if (ff == "format") {
    parseFormat(ss, d);
  }
}


///////////////////////////////////////////////////////////////////////////////
void
parseObjectFileName(const std::string& s, DatFileData& d)
{
  d.volumeFileName = s;
}


///////////////////////////////////////////////////////////////////////////////
void
parseResolution(const std::string& s, DatFileData& d)
{
  std::string::size_type space{ s.find_first_of(" ") };
  if (space == std::string::npos) {
    return;
  }

  d.rX = static_cast<unsigned int>(stoi(s.substr(0, space)));

  std::string::size_type oldSpace = space;
  space = s.find_first_of(" ", oldSpace + 1);
  d.rY = static_cast<unsigned int>(stoi(s.substr(oldSpace + 1, space)));

  oldSpace = space;
  space = s.find_first_of(" ", oldSpace + 1);
  d.rZ = static_cast<unsigned int>(stoi(s.substr(oldSpace + 1, space)));
}


///////////////////////////////////////////////////////////////////////////////
void
parseFormat(const std::string& s, DatFileData& d)
{
  const std::string ss{ toLowerCase(s) };
  d.dataType = to_dataType(ss);
}

///////////////////////////////////////////////////////////////////////////////
std::string
toLowerCase(const std::string& s)
{
  if (s.empty()) return "";
  std::string ss;
  ss.resize(s.length());

  std::transform(s.begin(), s.end(), ss.begin(), ::tolower);
  return ss;
}
} // namespace


///////////////////////////////////////////////////////////////////////////////
bool
parseDat(const std::string& datfile, DatFileData& data)
{
  std::ifstream f; // (datfile, std::ios::in);
  f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  std::stringstream ss;

  try {
    f.open(datfile, std::ios::in);
    ss << f.rdbuf();
    f.close();
  } catch (std::ifstream::failure e) {
    std::cerr << "Opening/reading/closing the dat file failed: " << e.what() << std::endl;
    return false;
  }

  std::string line;
  while (std::getline(ss, line)) {
    if (line[0] == '#') continue;
    SplitPair split{ splitAroundColon(line) };
    match(split, data);
  }

  return true;
}


///////////////////////////////////////////////////////////////////////////////
std::ostream&
operator<<(std::ostream& os, const DatFileData& d)
{
  os << d.to_string();
  return os;
}
} // namespace bd


