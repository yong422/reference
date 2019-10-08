#include "gtest/gtest.h"

#include "gstd/client/http/http-client.h"

#include <iostream>

// TODO: 네트워크에 상관없는 테스트로 구성
// TODO: docker 기본 http 테스트 환경 구축하여 docker 이용한 테스트 구성
class HttpClientTest : public ::testing::Test {
 protected:
  // 테스트에 필요한 설정을 설정
  // 각 테스트 케이스 시작전 호출
  virtual void SetUp() {
    //CurlGlobalInit();
  }

  // 테스트 자원 해제
  // 각 테스트 종료시 호출출
  virtual void TearDown() {
    //CurlGlobalExit();
  }
};

TEST_F(HttpClientTest, VersionCheck){
  std::cout << "curl version : " << gstd::client::http::HttpClient::GetCurlVersion() << std::endl;
  std::cout << "ssl of curl version : " << gstd::client::http::HttpClient::GetSSLVersion() << std::endl;
  // version string 결과가 있으면 성공
  EXPECT_STRNE("", gstd::client::http::HttpClient::GetCurlVersion().c_str());
  EXPECT_STRNE("", gstd::client::http::HttpClient::GetSSLVersion().c_str());
}

TEST_F(HttpClientTest, Post){
  gstd::client::http::HttpClient http_client;
  //http_client.set_is_debug(true);
  http_client.set_url("https://inspect.gabia.com/api/devices");
  http_client.set_is_verify(false);
  http_client.set_timeout(/*timeout*/ 5);
  http_client.set_retries(3);
  // post 요청테스트 서버가 문제아니면 true
  EXPECT_EQ(true, http_client.Post());
  EXPECT_EQ(gstd::client::http::kNotFound, http_client.response_code());
  EXPECT_STREQ("{\"message\":\"Not Found\"}", http_client.GetResponseData());
  std::cout << http_client.GetResponseData() << std::endl;
}

TEST_F(HttpClientTest, FileDownloadTest){

  gstd::client::http::HttpClient http_client;
  http_client.set_download_file_name("GabiaSMSAgent-0.68.76.exe");
  http_client.set_is_debug(true);
  http_client.set_url("http://monrepo.gabia.com/repo/windows/GabiaSMSAgent-0.68.76.exe ");
  http_client.Get();
  EXPECT_STREQ("", http_client.last_error_message().c_str());
  EXPECT_EQ(gstd::client::http::kSuccess, http_client.response_code());
  std::cout << "time : " << http_client.download_speed() << " sec" << std::endl;
  std::cout << "size : " << http_client.download_page_size() / 1024 << " kBytes" << std::endl;
}
