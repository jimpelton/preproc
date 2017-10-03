#ifndef RECIPIENT_H
#define RECIPIENT_H

#include "message.h"
#include <iostream>


namespace preproc
{

class DataReadMesage;

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
  handle_DataReadMessage(DataReadMessage &) { }
  

};

 /////////////////////////////////////////////////////////////////////////////// 
  class DataReadMessage : public Message
  {
  public:
    DataReadMessage ()
      : Message( MessageType::DATA_READ_MESSAGE )
      , IsChanging{ false }
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


} // namespace preproc
#endif // RECIPIENT_H
