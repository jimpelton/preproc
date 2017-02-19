//
// Created by jim on 2/18/17.
//

#ifndef PREPROCESSOR_WRITER_H
#define PREPROCESSOR_WRITER_H

#include <bd/log/logger.h>
#include <bd/io/buffer.h>
#include <bd/datastructure/blockingqueue.h>

#include <fstream>
#include <future>

template<class Ty>
class Writer
{

public:

  using buffer_type = typename bd::Buffer<Ty>;
  using queue_type = typename bd::BlockingQueue<buffer_type *>;


  Writer()
      : Writer{ nullptr, nullptr }
  {
  }


  Writer(bd::BlockingQueue<buffer_type *> *full,
         bd::BlockingQueue<buffer_type *> *empty)
      : m_empty{ empty }
      , m_full{ full }
  {
  }


  virtual ~Writer()
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
  operator()(std::ofstream &os)
  {
    bd::Info() << "Starting writer loop.";
    size_t bytes_written{ 0 };
    while (true) {

      buffer_type *buf{ m_full->pop() };

      // Let worker thread exit if the "magical empty buffer" is encountered
      if (!buf->getPtr()) {
        bd::Dbg() << "Empty buffer encountered, exiting writer loop.";
        break;
      }

      os.write(reinterpret_cast<char *>(buf->getPtr()),
               buf->getNumElements() * sizeof(Ty));
      bytes_written += buf->getNumElements() * sizeof(Ty);

      bd::Dbg() << "Writer written " << bytes_written << " bytes";

      buf->setNumElements(0);

      m_empty->push(buf);
    }

    bd::Info() << "Writer loop finished.";

    return 0;
  }


  static void
  start(Writer<Ty> &w, std::ofstream &os)
  {
    w.writer_future =
        std::async([&w](std::ofstream &of) -> uint64_t {
                     return w(of);
                   },
                   std::ref(os));
  }


  uint64_t
  join()
  {
    writer_future.wait();
    return writer_future.get();
  }


private:
  queue_type *m_empty;
  queue_type *m_full;

  std::future<uint64_t> writer_future;

};

#endif //PREPROCESSOR_WRITER_H
