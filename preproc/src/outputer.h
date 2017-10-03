#ifndef preproc_outputer_h__
#define preproc_outputer_h__

#include "messages/recipient.h"

namespace preproc{

class Outputer : public Recipient {

public:

  Outputer(): Recipient(){
    Broker::subscribeRecipient(this);
  }

  virtual ~Outputer(){ }


///////////////////////////////////////////////////////////////////////////////
void 
handle_DataReadMessage(DataReadMessage const &m) override
{
  

}




}


} // namespace preproc


#endif // ! preproc_outputer_h__
