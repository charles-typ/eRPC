#include <string>

static inline std::string gen_random(const size_t len)
{
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  tmp_s.reserve(len);

  for (size_t i = 0; i < len; ++i)
  {
    tmp_s += alphanum[static_cast<size_t>(rand()) % (sizeof(alphanum) - 1)];
  }

  return tmp_s;
}