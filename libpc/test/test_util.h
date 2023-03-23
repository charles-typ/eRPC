#ifndef PC_LIBPC_SRC_TEST_TEST_UTIL_H_
#define PC_LIBPC_SRC_TEST_TEST_UTIL_H_

#include <string>

static inline std::string gen_random(const int len)
{
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i)
  {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return tmp_s;
}

#endif // PC_LIBPC_SRC_TEST_TEST_UTIL_H_