#ifndef preproc_outputer_h__
#define preproc_outputer_h__

#include "semathing.h"
#include "messages/recipient.h"
#include "messages/messagebroker.h"

#include <cstddef>

namespace preproc
{

class Outputer : public Recipient
{

public:

  Outputer()
      : Recipient()
      , totalWritten{0}
      , totalRead{ 0 }
      , m_semathing{ 1 }
  {
    Broker::subscribeRecipient(this);
  }

virtual ~Outputer()
{
  Broker::removeRecipient(this);
  shouldExit=true;
  m_semathing.signal();
  m_future.wait();
}


  void
  start()
  {
    m_future = std::async(std::launch::async, [this](){ return this->print(); });
  }

///////////////////////////////////////////////////////////////////////////////
void 
handle_DataReadMessage(DataReadMessage const &m) override
{
  totalRead += m.Amount;
  m_semathing.signal();
}


///////////////////////////////////////////////////////////////////////////////
void
handle_DataWrittenMessage(DataWrittenMessage const &m) override
{
  totalWritten += m.Amount;
  m_semathing.signal();
}


///////////////////////////////////////////////////////////////////////////////
void
handle_EmptyMessage(EmptyMessage const &m) override
{
  shouldExit = true;
  m_semathing.signal();
}

private:
  void
  print()
  {
    shouldExit=false;
    while(true) {
      m_semathing.wait();
      if (shouldExit) {
        break;
      }
      std::cout << "\r";
      if (totalRead > 0) {
        std::cout << "Total read: " << totalRead << " ";
      }
      if (totalWritten > 0) {
        std::cout << "Total written: " << totalWritten;
      }
      std::cout << std::flush;
    }
    std::cout << std::flush;
    bd::Info() << "Exiting print loop in Outputer.";
  }

  size_t totalWritten;
  size_t totalRead;
  bool shouldExit;
  Semathing m_semathing;
  std::future<void> m_future;
};


} // namespace preproc


#endif // ! preproc_outputer_h__
