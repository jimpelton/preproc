#ifndef subvol_globalstatscollector_h__
#define subvol_globalstatscollector_h__

#include <iostream>

namespace preproc{
  enum class MessageType : int {
    EMPTY_MESSAGE,
    DATA_READ_MESSAGE,
  };
  
  class Recipient;
  class Message {
  public:
    Message(MessageType t) 
      : type{ t }
    {
    }
    virtual ~Message() { }

    virtual void
      operator()(Recipient &r) 
    {
    }

    MessageType type;
  };



} // namespace subvol



#endif  // !subvol_globalstatscollector_h__
