#ifndef subvol_globalstatscollector_h__
#define subvol_globalstatscollector_h__

namespace preproc{
  enum class MessageType : int {
    EMPTY_MESSAGE,
    DATA_READ_MESSAGE,
    DATA_WRITTEN_MESSAGE,
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



} // namespace preproc



#endif  // !subvol_globalstatscollector_h__
