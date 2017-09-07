//
// Created by jim on 8/23/16.
//

#include "cmdline.h"


#include <bd/io/datfile.h>
#include <glm/glm.hpp>

#include <iostream>
#include <array>
#include <string>
#include <fstream>
#include <bd/log/logger.h>


namespace concat
{
  template<class Ty>
  class RowReader
  {
  public:
    RowReader(std::unique_ptr<std::istream> in, glm::u64vec3 const &dims)
      : m_in{ std::move(in) }
      , m_rowBytes{ dims.x * sizeof(Ty) }
      , m_totalRows{ dims.y * dims.z }
      , m_currentRow{ 0 }
    {
    }

    ~RowReader()
    {
    }

    size_t
    nextRow(Ty *buf)
    {
      if (m_currentRow == m_totalRows || !m_in->good()) {
        return 0;
      }

      m_in->get(reinterpret_cast<char*>(buf), m_rowBytes);

      m_currentRow++;
      return m_rowBytes;
    }

  private:
    std::unique_ptr<std::istream> m_in;
    size_t const m_rowBytes;
    size_t const m_totalRows;
    size_t m_currentRow;
  };


  template<class Ty>
  class Concatenator
  {
  public:
    Concatenator(std::string const &inFile,
      std::string const &outFile,
      glm::u64vec3 const &inDims,
      glm::u64vec3 const &outDims)
      : m_inName{ inFile }
      , m_outName{ outFile }
      , m_inDims{ inDims }
      , m_outDims{ outDims }
    {
    }


    ~Concatenator()
    {
      if (m_inBuffer != nullptr) {
        delete m_inBuffer;
      }
      if (m_outBuffer != nullptr) {
        delete m_outBuffer;
      }
    }

    void
    concat()
    {
      m_inBuffer = new Ty[m_inDims.x];

      std::unique_ptr<std::ifstream> inFile{ new std::ifstream{ m_inName, std::ios::binary } };
      if (! inFile->is_open()) {
        return;
      }
      RowReader<Ty> rr(std::move(inFile), m_inDims);

      std::ofstream outFile{ m_outName, std::ios::binary };
      if (! outFile.is_open()) {
        return;
      }

      size_t concats{ m_outDims.x / m_inDims.x };
      while (rr.nextRow(m_inBuffer) > 0) {
        // write this row x number of times
        for (size_t i = 0; i < concats; ++i) {
          outFile.write(reinterpret_cast<char*>(m_inBuffer), m_inDims.x * sizeof(Ty));
        }
      }
    }

  private:

    std::string const m_inName;
    std::string const m_outName;
    glm::u64vec3 const m_inDims;
    glm::u64vec3 const m_outDims;
    
    Ty *m_inBuffer;
    Ty *m_outBuffer;

  }; // class Concatenator

  


  template<class Ty>
  void
  doConcat(CommandLineOptions const &clo, bd::DatFileData const &dat)
  {
    Concatenator<Ty> cc{
      clo.rawFilePath,
      clo.outputFilePath,
      {dat.rX, dat.rY, dat.rZ},
      {clo.targetXDim, clo.targetYDim, clo.targetZDim}
    };

    cc.concat();
    
  }


} // namespace concat

int
main(int argc, const char *argv[])
{
  concat::CommandLineOptions clo;

  if (concat::parseThem(argc, argv, clo) == 0) {

    std::cerr << "No arguments provided.\nPlease use -h for usage info."
      << std::endl;
    return 1;
  }

  bd::DatFileData datFile;
  bd::parseDat(clo.datFilePath, datFile);

  switch (datFile.dataType) {
  case bd::DataType::Character:
    concat::doConcat<int8_t>(clo, datFile);
    break;
  case bd::DataType::UnsignedCharacter:
    concat::doConcat<uint8_t>(clo, datFile);
    break;
  case bd::DataType::Short:
    concat::doConcat<int16_t>(clo, datFile);
    break;
  case bd::DataType::UnsignedShort:
    concat::doConcat<uint16_t>(clo, datFile);
    break;
  case bd::DataType::Float:
    concat::doConcat<float>(clo, datFile);
    break;
  case bd::DataType::Double:
    concat::doConcat<double>(clo, datFile);
    break;
  default:
    break;
  }
}
