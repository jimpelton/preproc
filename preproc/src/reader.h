//
// Created by jim on 2/18/17.
//


#ifndef PREPROCESSOR_READER_H
#define PREPROCESSOR_READER_H

#include <bd/io/buffer.h>
#include <bd/datastructure/blockingqueue.h>


template<class Ty>
class Reader
{

public:
  using buffer_type = typename bd::Buffer<Ty>;
  using queue_type = typename bd::BlockingQueue<buffer_type *>;


  Reader()
      : Reader{ nullptr, nullptr }
  {
  }


  Reader(bd::BlockingQueue<buffer_type *> *full,
         bd::BlockingQueue<buffer_type *> *empty)
      : m_empty{ empty }
      , m_full{ full }
  {
  }


  virtual ~Reader()
  {
  }


public:

  void
  setFull(bd::BlockingQueue<buffer_type *> *full)
  {
    m_full = full;
  }


  void
  setEmpty(bd::BlockingQueue<buffer_type *> *empty)
  {
    m_empty = empty;
  }


  uint64_t
  operator()(std::istream &is)
  {
    size_t bytes_read{ 0 };
    bd::Info() << "Starting reader loop.";

    while (true) {
      buffer_type *buf{ m_empty->pop() };
      if (!buf->getPtr()) {
        break;
      }

      is.read(reinterpret_cast<char *>(buf->getPtr()),
              buf->getMaxNumElements() * sizeof(Ty));

      std::streamsize amount{ is.gcount() };
      bytes_read += amount;
      buf->setNumElements(amount / sizeof(Ty));
      bd::Dbg() << "Reader read " << bytes_read << " bytes";
      m_full->push(buf);

      // entire file has been read.
      if (amount < static_cast<long long>(buf->getMaxNumElements())) {
        break;
      }

    }

    bd::Info() << "Reader loop finished.";

    m_full->push(new bd::Buffer<Ty>(nullptr, 0));

    return bytes_read;
  }


  static void
  start(Reader &r, std::ifstream &is)
  {
    r.reader_future =
        std::async([&r](std::ifstream &of) -> uint64_t {
                     return r(of);
                   },
                   std::ref(is));
  }


  uint64_t
  join()
  {
    reader_future.wait();
    return reader_future.get();
  }


private:
  queue_type *m_empty;
  queue_type *m_full;

  std::future<uint64_t> reader_future;

};





#endif //PREPROCESSOR_READER_H
