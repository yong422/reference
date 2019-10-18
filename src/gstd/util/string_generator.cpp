#include <iostream>
#include <sstream>
#include <random>
#include <vector>
#include <numeric>    // std::itoa
#include <algorithm>  // for_each
#include <functional> // bind

#include "gstd/util/string_generator.h"

namespace gstd {
namespace util {
  // https://lowrey.me/guid-generation-in-c-11/
  std::string GenerateUUID(const uint32_t& length) {
    // 0, 255 범위내에서 정수형 난수를 생성
    std::uniform_int_distribution<> distribution(0, 255);
    // 내부 하드웨어의 리소스를 이용하여 난수를 생성.
    std::random_device rd;
    // 난수 추출에 메르센 트위스터 엔진 사용.
    std::mt19937 gen(rd());
    auto generate_randnum = std::bind(distribution, gen);
    std::stringstream uuid_stream;
    std::vector<uint32_t> length_vector(length);
    std::iota(length_vector.begin(), length_vector.end(), 0);
    for_each(length_vector.begin(), length_vector.end(), [&uuid_stream, &generate_randnum](uint32_t i){
      std::stringstream hexstream;
      hexstream << std::hex << generate_randnum();
      auto hex = hexstream.str();
      uuid_stream << (hex.length() < 2 ? '0' + hex : hex);
    });
    return uuid_stream.str();
  }

} //  namespace util
} //  namespace gstd
