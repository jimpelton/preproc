#ifndef subvol_messagebroker_h__
#define subvol_messagebroker_h__

#include "message.h"
#include "recipient.h"

#include <bd/log/logger.h>

#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>
#include <iostream>


namespace preproc {

  class Broker {

  public:
    
    static void
    send(Message *m) 
    {
      std::unique_lock<std::mutex> lock(m_myself->m_waitMessageMutex);
      m_myself->m_messages.push(m);
      lock.unlock();
      m_myself->m_waitMessage.notify_all();
    }

    static void
    subscribeRecipient(Recipient *r)
    {
      m_myself->m_recipients.push_back(r);
    }

    static void
    start()
    {
      if (m_myself == nullptr) {
        m_myself = new Broker();
      }

      m_myself->m_workFuture = std::async(std::launch::async,
        [] { m_myself->work(); });
    }



  private:
    void
    work()
    {
      while(true) {
        Message *m{ getNextMessage() };

        for (auto &r : m_recipients) {
          r->deliver(m);
        }

        delete m;

        if (m->type == MessageType::EMPTY_MESSAGE) {
          break;
        }

      } // while
      bd::Info() << "Message worker exiting.";
    }
    
    Message *
    getNextMessage()
    {
      std::unique_lock<std::mutex> lock(m_waitMessageMutex);
      while(m_messages.empty()) {
        m_waitMessage.wait(m_waitMessageMutex);
      }

      Message *m{ m_messages.front() };
      m_messages.pop();
      return m;
    }


    std::vector<Recipient*> m_recipients;

//    std::mutex m_messagesMutex;
    std::queue<Message*> m_messages;

    std::mutex m_waitMessageMutex;
    std::condition_variable_any m_waitMessage;

    std::future<void> m_workFuture;

    static Broker *m_myself;
    //    std::map<Recipient, std::list<MessageType>> m_receipients;


  };
}
#endif // ! subvol_messagebroker_h__
