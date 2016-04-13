//
// Created by jim on 6/7/15.
//

#ifndef ordinal_h__
#define ordinal_h__

// Visual Studio does not support C++11 constexpr (sheesh!)
#ifdef _WIN32
#define PREPROC_CONSTEXPR
#else
#define PREPROC_CONSTEXPR constexpr
#endif 

namespace preproc
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Return the ordinal value from a strongly typed C++11 enum class.
/// The only template parameter is the enum class type.
///
/// \return The ordinal value as a std::size_t
///////////////////////////////////////////////////////////////////////////////
template <typename T>
PREPROC_CONSTEXPR
std::size_t
ordinal(T t)
{
  return static_cast<unsigned int>(t);
}
} // namespace preproc

#endif // !ordinal_h__


