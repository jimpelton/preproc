//
// Created by jim on 4/2/17.
//

#ifndef SUBVOL_SEMATHING_H
#define SUBVOL_SEMATHING_H

#include <mutex>

namespace preproc
{

class Semathing
{

public:
  Semathing(unsigned max)
      : max{ max }
      ,count{ max }
  {
  }


  ~Semathing()
  {
  }


  void
  signal()
  {
    std::unique_lock <std::mutex> lck(mutex);
    count--;
    if (count <= 0) {
      cv.notify_all();
    }
  }


  void
  wait()
  {
    std::unique_lock <std::mutex> lck(mutex);
    while (count > 0) {
      cv.wait(mutex);
    }
    count = max;
  }

  /// \brief Reset the semaphore. This function is not thread safe!!!
  void
  reset()
  {
    count = max;
  }

private:
  unsigned int const max;
  unsigned int count;
  std::mutex mutex;
  std::condition_variable_any cv;
};

} // namespace subvol

#endif // ! SUBVOL_SEMATHING_H
