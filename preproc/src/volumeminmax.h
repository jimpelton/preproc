#ifndef preproc_volumeminmax
#define preproc_volumeminmax


#include <bd/io/buffer.h>
#include <bd/io/bufferedreader.h>
#include <bd/log/logger.h>
#include <bd/tbb/parallelreduce_minmax.h>

#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

#include <string>

template<typename Ty>
void
volumeMinMax(std::string const & path,
             size_t szbuf,
             int numThreads,
             double *volMin,
             double *volMax,
             double *volTotal)
{

  bd::BufferedReader<Ty> r{ szbuf };
  if (!r.open(path)) {
    bd::Err() << "File " << path << " was not opened.";
    return;
  }
  r.start();


  double max{ 0 };
  double min{ 0 };
  double total{ 0 };

  bd::Info() << "Begin min/max computation.";

  bd::Buffer<Ty> *buf{ nullptr };
  
  tbb::task_scheduler_init init(numThreads);

  while ((buf = r.waitNextFullUntilNone()) != nullptr) {

    tbb::blocked_range<size_t> range(0, buf->getNumElements());
    bd::ParallelReduceMinMax<Ty> mm(buf);

    tbb::parallel_reduce(range, mm);

    if (max < mm.max_value)
      max = mm.max_value;
    if (min > mm.min_value)
      min = mm.min_value;

    if (total > 0.0) {
      total += mm.tot_value;
    } else {
      total = mm.tot_value;
    }

    r.waitReturnEmpty(buf);

  }

  bd::Info() << "Finished min/max computation.";
  *volMin = min;
  *volMax = max;
  *volTotal = total;
}

#endif // ! preproc_volumeminmax

