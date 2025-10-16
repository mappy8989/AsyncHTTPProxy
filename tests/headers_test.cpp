#include "headers.h"
#include <gtest/gtest.h>

TEST(iterHeaders, Empty) {
    // code here
}

TEST(iterHeaders, SkipRequestLine) {
    // code here
}

TEST(iterHeaders, SingleHeader) {
    // code here
}

TEST(iterHeaders, MultipleHeaders) {
    // code here
}

TEST(iterHeaders, MultipleSameHeaders) {
    // code here
}

TEST(findHostPort, Simple) {
    std::string st = "GET /path HTTP/1.1\r\n\
Host: example.com:1234\r\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\n\
Accept: text/html,application/xhtml+xml\r\n\
Cookie: sessionId=abc123\r\n\
Content-Length: 4567\r\n\
Authorization : Bearer token123 ";
    // code here
    auto host_port = findHostPort(st);
    ASSERT_TRUE(host_port.first == "example.com");
    ASSERT_TRUE(host_port.second == "1234");
}

TEST(findHostPort, NoHost) {
    std::string st = "GET /path HTTP/1.1\r\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\n\
Accept: text/html,application/xhtml+xml\r\n\
Cookie: sessionId=abc123\r\n\
Content-Length: 4567\r\n\
Authorization : Bearer token123 ";
    // code here
    auto host_port = findHostPort(st);
    ASSERT_TRUE(host_port.first.empty());
    ASSERT_TRUE(host_port.second.empty());
}

TEST(findContentLength, Simple) {
    std::string st = "GET /path HTTP/1.1\r\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\n\
Accept: text/html,application/xhtml+xml\r\n\
Cookie: sessionId=abc123\r\n\
Content-Length: 4567\r\n\
Authorization : Bearer token123 ";

    auto len = findContentLength(st);
    ASSERT_TRUE(len.value() == 4567);
}

TEST(findContentLength, NoContentLength) {
    std::string st = "GET /path HTTP/1.1\r\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\n\
Accept: text/html,application/xhtml+xml\r\n\
Cookie: sessionId=abc123\r\n\
Authorization : Bearer token123 ";

    auto len = findContentLength(st);
    ASSERT_TRUE(!len.has_value());
}
