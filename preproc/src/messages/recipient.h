#ifndef RECIPIENT_H
#define RECIPIENT_H

#include "message.h"

#include <cstddef>

namespace preproc
{

class DataReadMessage;
class DataWrittenMessage;
class EmptyMessage;

class Recipient {
public:
  Recipient()
  {
  }


  virtual ~Recipient()
  {
  }

  virtual void
  deliver(Message *m)
  {
    (*m)(*this);
  }

  virtual void
  handle_EmptyMessage(EmptyMessage const &) { }

  virtual void
  handle_DataReadMessage(DataReadMessage const &) { }

  virtual void
  handle_DataWrittenMessage(DataWrittenMessage const &) { }

};

 /////////////////////////////////////////////////////////////////////////////// 
  class DataReadMessage : public Message
  {
  public:
    DataReadMessage ()
      : Message( MessageType::DATA_READ_MESSAGE )
      , Amount{ 0 }
    {
    }

    virtual ~DataReadMessage() { }

    void
    operator()(Recipient &r) override
    {
      r.handle_DataReadMessage(*this);
    }

    size_t Amount;
  };

///////////////////////////////////////////////////////////////////////////////
class DataWrittenMessage : public Message
{
public:
  DataWrittenMessage ()
      : Message( MessageType::DATA_WRITTEN_MESSAGE )
      , Amount{ 0 }
  {
  }

  virtual ~DataWrittenMessage() { }

  void
  operator()(Recipient &r) override
  {
    r.handle_DataWrittenMessage(*this);
  }

  size_t Amount;
};

///////////////////////////////////////////////////////////
class EmptyMessage : public Message
{
public:
  EmptyMessage ()
      : Message( MessageType::EMPTY_MESSAGE)
  {
  }

  virtual ~EmptyMessage() { }

  void
  operator()(Recipient &r) override
  {
    r.handle_EmptyMessage(*this);
  }

};

} // namespace preproc
#endif // RECIPIENT_H
