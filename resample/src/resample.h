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

template<class Ty>
class Resample
{
public:
  Resample();
  ~Resample();


  bool
  operator()(std::string const &rawFile, bd::IndexFile const &indexFile);


private:
  char *
  allocateMemAndStuff();

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

}



template<class T>
bool
Resample<T>::operator()(std::string const &rawFile, bd::IndexFile const &indexFile)
{


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
