
#include "gstd/net/ipchecker.h"
#include "gmock/gmock.h"
class MockIpChecker : public gstd::net::IpChecker {

  public:
    MOCK_METHOD1(Open, bool(const char* dbfile));
};