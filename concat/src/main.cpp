//
// Created by jim on 8/23/16.
//

#include "cmdline.h"


#include <bd/log/logger.h>
#include <bd/io/datfile.h>
#include <glm/glm.hpp>

#include <iostream>
#include <array>
#include <string>
#include <fstream>


namespace concat
{
  template<class Ty>
  class RowReader
  {
  public:
    RowReader(std::unique_ptr<std::istream> in, glm::u64vec3 const &dims)
      : m_in{ std::move(in) }
      , m_dims{ dims }
      , m_rowBytes{ dims.x * sizeof(Ty) }
      , m_totalRows{ dims.y * dims.z }
      , m_currentRow{ 0 }
      , m_cpos{ 0 }
    {
      std::cout << "RowReader has " << m_totalRows << " to read.";
      std::cout << "Each row is " << m_rowBytes << " bytes.";
    }

    ~RowReader()
    {
    }

    size_t
    nextRow(Ty *buf)
    {
      if (m_currentRow == m_totalRows) {
        std::cout << "\nNo rows left." << std::endl;
        return 0;
      }
      
      if (m_in->read(reinterpret_cast<char*>(buf), m_rowBytes)) {
        m_currentRow++;
        auto amount = m_in->tellg() - m_cpos;
        m_cpos = m_in->tellg();
        return amount;
      }

      return 0;
    }

    void
    seekToSlab(size_t slab) const {
      m_in->seekg(slab * m_dims.x * m_dims.y * sizeof(Ty));
    }

  private:
    
    std::unique_ptr<std::istream> m_in;
    glm::u64vec3 const m_dims;
    size_t const m_rowBytes;
    size_t const m_totalRows;
    size_t m_currentRow;
    std::streamoff m_cpos;
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
      , m_inBuffer{ nullptr }
    {
    }


    ~Concatenator()
    {
      if (m_inBuffer != nullptr) {
        delete m_inBuffer;
      }
    /*  if (m_outBuffer != nullptr) {
        delete m_outBuffer;
      }*/
    }

    void
    concat()
    {
      m_inBuffer = new Ty[m_inDims.x];

      std::unique_ptr<std::ifstream> inFile{ 
        new std::ifstream{ m_inName, std::ios::binary } };

      if (! inFile->is_open()) {
        std::cerr << "Could not open input file: " << m_inName << std::endl;
        return;
      }
      RowReader<Ty> rr(std::move(inFile), m_inDims);

      std::ofstream outFile{ m_outName, std::ios::binary };
      if (! outFile.is_open()) {
        std::cerr << "Could not open output file: " << m_outName << std::endl;
        return;
      }

      glm::u64vec3 concats{ m_outDims / m_inDims };
      size_t totalRead{ 0 };
      size_t read{ 0 };

      for (size_t cz = 0; cz < concats.z; ++cz) {
        // seek to start of file
        //rr.seekToSlab(0);
        size_t s{ 0 };
        for (size_t cy = 0; cy < concats.y; ++cy) {
          // seek to start of slab cy
          rr.seekToSlab(s++);
          size_t r{ 0 };
          // write out slab cy, reading row by row.
          while ((read = rr.nextRow(m_inBuffer)) > 0 && r < m_inDims.y) {
            totalRead += read;
            // write this row x number of times
            for (size_t cx = 0; cx < concats.x; ++cx) {
              outFile.write(reinterpret_cast<char*>(m_inBuffer), read);
              std::cout << '\r' << "Bytes read: " << totalRead << ", bytes written: " << outFile.tellp() << " bytes.";
            }
            r++;
          } // while
        } // for cy
      }
    }

  private:

    std::string const m_inName;
    std::string const m_outName;
    glm::u64vec3 const m_inDims;
    glm::u64vec3 const m_outDims;
    
    Ty *m_inBuffer;

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
    std::cout << "Done concatenating!" << std::endl;
    
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
  if (! bd::parseDat(clo.datFilePath, datFile) ) {
    std::cerr << "Could not open .dat file " << clo.datFilePath << std::endl;
    return 1;
  }
  std::cout << datFile.to_string() << std::endl;

  switch (datFile.dataType) {
  case bd::DataType::Character:
    std::cout << "int8_t" << std::endl;
    concat::doConcat<int8_t>(clo, datFile);
    break;
  case bd::DataType::UnsignedCharacter:
    std::cout << "uint8_t" << std::endl;
    concat::doConcat<uint8_t>(clo, datFile);
    break;
  case bd::DataType::Short:
    std::cout << "int16_t" << std::endl;
    concat::doConcat<int16_t>(clo, datFile);
    break;
  case bd::DataType::UnsignedShort:
    std::cout << "uint16_t" << std::endl;
    concat::doConcat<uint16_t>(clo, datFile);
    break;
  case bd::DataType::Float:
    std::cout << "float" << std::endl;
    concat::doConcat<float>(clo, datFile);
    break;
  case bd::DataType::Double:
    std::cout << "double" << std::endl;
    concat::doConcat<double>(clo, datFile);
    break;
  default:
    std::cerr << "Unsupported data type: " << bd::to_string(datFile.dataType) << std::endl;
    return 1;
  }

  return 0;
}
