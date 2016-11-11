#ifndef preproc_volumeminmax
#define preproc_volumeminmax

#include <bd/io/buffer.h>
#include <bd/io/bufferedreader.h>
#include <bd/log/logger.h>
#include <bd/tbb/parallelreduce_minmax.h>

#include <tbb/tbb.h>

#include <string>

template<typename Ty>
void
volumeMinMax(std::string const & path, size_t szbuf, double *volMin, double *volMax)
{
  bd::BufferedReader<Ty> r{ szbuf };
  if (!r.open(path)) {
    bd::Err() << "File " << path << " was not opened.";
    return;
  }


  r.start();
  Ty max{ 0 };
  Ty min{ 0 };

  bd::Info() << "Begin min/max computation.";

  bd::Buffer<Ty> *buf{ nullptr };

  while ((buf = r.waitNextFullUntilNone()) != nullptr) {

    tbb::blocked_range<size_t> range(0, buf->getNumElements());
    bd::ParallelReduceMinMax<Ty> mm(buf);

    tbb::parallel_reduce(range, mm);

    if (max < mm.max_value)
      max = mm.max_value;
    if (min > mm.min_value)
      min = mm.min_value;

    r.waitReturnEmpty(buf);

  }

  bd::Info() << "Finished min/max computation.";
  *volMin = min;
  *volMax = max;
}

#endif // ! preproc_volumeminmax

