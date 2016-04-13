#include "datatypes.h"
#include "logger.h"

#include <algorithm>
#include <iostream>

namespace preproc
{

namespace
{
  const std::map<std::string, DataType> DataTypesMap
  {
    { "int", DataType::Integer },
    { "uint", DataType::UnsignedInteger },
    { "unsigned integer", DataType::UnsignedInteger },

    { "char", DataType::Character },
    { "uchar", DataType::UnsignedCharacter },
    { "unsigned char", DataType::UnsignedCharacter },

    { "short", DataType::Short },
    { "ushort", DataType::UnsignedShort },
    { "unsigned short", DataType::UnsignedShort },

    { "float", DataType::Float },

    { "unknown", DataType::Unknown }
  };
} // namespace


DataType
to_dataType(const std::string &typeStr)
{
 
  try {
    return DataTypesMap.at(typeStr);
  } catch (std::out_of_range &e) {
    std::cerr << e.what() << std::endl;
    Err()<< typeStr << " is unknown datatype!";
    return DataType::Unknown;
  }
}

std::string
to_string(DataType type)
{
  try {
    // convert data type to string.
    auto cit = std::find_if(DataTypesMap.begin(), DataTypesMap.end(),
      [type](std::pair<std::string, DataType> p)
        {
          return p.second == type;
        });

    if (cit != DataTypesMap.end()) {
      return (cit->first).c_str();
    } 
    
    return "unknown";
    

  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return "unknown";
  }
}
} /* namespace bd */


