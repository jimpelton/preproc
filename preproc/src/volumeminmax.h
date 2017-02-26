#ifndef preproc_volumeminmax
#define preproc_volumeminmax


#include <bd/io/buffer.h>
#include <bd/io/bufferedreader.h>
#include <bd/log/logger.h>
#include <bd/tbb/parallelreduce_minmax.h>
#include <bd/volume/volume.h>

#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

#include <string>





/// Open the raw data file at \c path and compute the min, max and average values.
/// \tparam Ty The data type of the voxel elements in the volume.
/// \param path The path to the file.
/// \param szbuf Size of buffer (in bytes) to allocate.
/// \param numThreads Number of tbb threads to use for processing.
/// \param volume The volume to use for storing the results in.
template<typename Ty>
void
volumeMinMax(std::string const & path,
             size_t szbuf,
             int numThreads,
             bd::Volume &volume)
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
  
  volume.min(min);
  volume.max(max);
  volume.total(total);
  glm::u64vec3 dims{ volume.voxelDims() };
  volume.avg(total / double(dims.x * dims.y * dims.z));

}

#endif // ! preproc_volumeminmax

