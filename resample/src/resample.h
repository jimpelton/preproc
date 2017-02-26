//
// Created by jim on 2/19/17.
//

#ifndef resample_resample_h
#define resample_resample_h

#include <bd/datastructure/blockingqueue.h>
#include <bd/io/indexfile.h>
#include <bd/io/reader.h>
#include <bd/io/writer.h>

#include <string>

namespace resample
{

//bool
//runFromIndexFile(std::string const &rawFile, bd::IndexFile const &indexFile);

static void
go(bd::DataType t);



template<class Ty>
class Resample
{
public:

  Resample();
  ~Resample();


  bool
  operator()(std::string const &rawFile,
             glm::u64vec3 const &src,
             glm::u64vec3 const &dest,
             size_t memSize);


private:

  char *
  allocateBuffers(char *mem,
                  bd::BlockingQueue<bd::Buffer<Ty> *> &empty,
                  size_t nBuff,
                  size_t lenBuff);

  bd::Reader<Ty> m_reader;
  bd::Writer<Ty> m_writer;

  bd::BlockingQueue<Ty> m_rawEmpty;
  bd::BlockingQueue<Ty> m_rawFull;
  bd::BlockingQueue<Ty> m_resampEmpty;
  bd::BlockingQueue<Ty> m_resampFull;

  char *m_mem;
};

template<class Ty>
Resample<Ty>::Resample()
    : m_reader{ }
    , m_writer{ }
    , m_rawEmpty{ }
    , m_rawFull{ }
    , m_resampEmpty{ }
    , m_resampFull{ }
    , m_mem{ nullptr }
{
}

template<class Ty>
Resample<Ty>::~Resample()
{
  if (m_mem) {
    delete m_mem;
  }
}

namespace {
bool
areUpsampling(glm::u64vec3 const &src, glm::u64vec3 const &dest)
{
  uint64_t s{ src.x * src.y * src.z };
  uint64_t d{ dest.x * dest.y * dest.z };
  return  s < d;
}

double
ratio(glm::u64vec3 const &first, glm::u64vec3 const &second)
{
  uint64_t f{ first.x * first.y * first.z };
  uint64_t s{ second.x * second.y * second.z };
  return  f / double(s);
}

} // namespace


template<class Ty>
bool
Resample<Ty>::operator()(std::string const &rawFile,
                         glm::u64vec3 const &src,
                         glm::u64vec3 const &dest,
                         size_t memSize)
{

  m_mem = new char[memSize];
  {
    size_t const num_b{ 20 };
    double r{ 0.0 };
    if (areUpsampling(src, dest)) {
      // up sampling, more memory to dest buffers
      r = ratio(src, dest);

    } else {
      //down sampling, more memory to src buffers
      r = ratio(dest, src);
    }


    // total bytes dedicated to raw buffers.
    size_t const sz_total_small{ size_t(memSize * r) };
    // total smapce dedicated to rmap buffers.
    size_t const sz_total_big{ memSize - sz_total_small };
    // how long is each buffer? It is based off of the number of rmap buffers
    // and the space allocated to rmap buffers.
    size_t const len_buffers{ size_t( (sz_total_big / num_b ) / sizeof(Ty) ) };
    size_t const sz_small{ len_buffers * sizeof(Ty) };
    // how many raw buffs can we make of same length.
    size_t num_small{ sz_total_small / sz_small };

    if (areUpsampling(src,dest))
    char *mem = allocateBuffers(m_mem, m_rawEmpty, num_small, len_buffers);
    allocateBuffers(mem, m_resampEmpty, num_rmap, len_buffers);

    bd::Info() << "Allocated: " << num_small << " raw buffers of length " << len_buffers
               << ", and " << num_rmap << " rmap buffers of length "
               << len_buffers << ".";

  }
  return 0;
}

template<class Ty>
char *
Resample<Ty>::allocateBuffers(char *mem,
                              bd::BlockingQueue<bd::Buffer<Ty> *> &empty,
                              size_t nBuff,
                              size_t lenBuff)
{

  //    size_t const sz_buf{ szMem / nBuff };
  //    size_t const count{ sz_buf / sizeof(Ty) };
  Ty *p{ reinterpret_cast<Ty *>(mem) };

  for (size_t i{ 0 }; i < nBuff; ++i) {
    bd::Buffer<Ty> *buf{ new bd::Buffer<Ty>(p, lenBuff) };
    empty.push(buf);
    p += lenBuff;
  }

  return reinterpret_cast<char *>(p);

} // allocateEmptyBuffers()

} // namespace resample

#endif // ! resample_resample_h
