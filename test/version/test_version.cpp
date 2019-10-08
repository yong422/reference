#include "gtest/gtest.h"
#include "gstd/check/version.h"
// ref/version 테스트

// upgrade 버전 테스트
TEST(TestVersion, CheckVersionUpgrade) {
  EXPECT_EQ(gstd::check::version::kHigher,
            gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "0.2.1", "0.2.5"));
  EXPECT_EQ(gstd::check::version::kHigher,
    gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "0.3.0", "0.4.5"));
  EXPECT_NE(gstd::check::version::kHigher,
    gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "0.3.0", "0.3.0"));
  EXPECT_NE(gstd::check::version::kHigher,
    gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "1.3.0", "0.7.0"));
}

// low 버전 테스트
TEST(TestVersion, CheckVersionLow) {
  EXPECT_EQ(gstd::check::version::kLower,
    gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "0.2.55", "0.2.54"));
  EXPECT_EQ(gstd::check::version::kLower,
    gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "0.3.55", "0.2.55"));
  EXPECT_EQ(gstd::check::version::kLower,
    gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "1.2.55", "0.6.54"));
}

// 동일 버전 또는 해석 실패 테스트
TEST(TestVersion, CheckVersionSameAndFailed) {
  EXPECT_EQ(gstd::check::version::kSame,
    gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "0.2.1", "0.2.1"));
  EXPECT_EQ(gstd::check::version::kSame,
    gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "0.3.0", "0.3.0"));
  EXPECT_NE(gstd::check::version::kSame,
    gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "1.0.1", "1.0.2"));
  EXPECT_NE(gstd::check::version::kSame,
    gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "0.3.1", "0.3.2"));

  EXPECT_EQ(gstd::check::version::kFailedParsing,
    gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "1.2.55.4", "0.6.54"));
  EXPECT_EQ(gstd::check::version::kFailedParsing,
    gstd::check::Version::CheckChanged(gstd::check::version::kSemantic, "1.2-55", "0.6.54"));
  EXPECT_EQ(gstd::check::version::kNotSupportedType,
    gstd::check::Version::CheckChanged(static_cast<gstd::check::version::Type>(5), "1.2-55", "0.6.54"));
}