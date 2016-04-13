#ifndef parsedat_h__
#define parsedat_h__

#include "datatypes.h"
#include "ordinal.h"

#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace preproc
{

struct DatFileData
{
  DatFileData() 
    : rX{ 0 }
    , rY{ 0 }
    , rZ{ 0 }
    , volumeFileName("")
    , dataType{ DataType::Float }
  {
  }

  // resolution
  unsigned int rX;
  unsigned int rY;
  unsigned int rZ;

  // file name of raw file
  std::string volumeFileName;

  // data type of raw file
  DataType dataType;

  std::string 
  to_string() const
  {
    std::stringstream ss;
 
    ss << "{ \"res\":[" << rX << "," << rY << "," << rZ << "],\n"
      "\"filename\":" << volumeFileName << ",\n"
      "\"dataType\":\"" << preproc::to_string(this->dataType) << "\"\n}";
    
    return ss.str();
  }
};

std::ostream& 
operator<<(std::ostream&, const DatFileData&);

bool
parseDat(const std::string& datfile, DatFileData& data);

} // namespace preproc


#endif // !parsedat_h__


