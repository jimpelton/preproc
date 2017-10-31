
#include <iostream>
#include <fstream>

#include <glm/glm.hpp>

template<typename Ty>
void
go(glm::vec3 const & dims)
{
  double const min_grad_perc{ 0.25 };
  double const max_grad_perc{ 0.75 };
  
  auto const max_val = std::numeric_limits<Ty>::max();

  double const delta_per_col{ (max_grad_perc - min_grad_perc) / (dims.x - 2) };

  std::ofstream fout("outfile.raw");
  if (! fout.is_open())
  {
    std::cerr << "Oh no! Could not open the file!\n";
    return;
  }

  for (int s{ 0 }; s < dims.z; ++s) {
    std::cout << "\n Slab " << s << "\n";
    for (int r{ 0 }; r < dims.y; ++r) {
      std::cout << "\n Row " << r << ": ";
      for (int c{ 0 }; c < dims.x; ++c) {

        Ty val{ Ty(0) };
        if (c == dims.x - 1) {
          val = Ty(1) * max_val;
        }
        else if (c > 0) {
          val = Ty(c * delta_per_col * max_val);
        }

        std::cout << val << " ";
        fout << val;
      }
    }
  }
  fout.flush();
  fout.close();
  
}

int
main(int argc, char* argv[])
{
  std::cout << "Volume gradient maker.\n";
  
  go<short>({ 16, 16, 16 });


  return 0;
}
