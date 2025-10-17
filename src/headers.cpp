#include "headers.h"

#include <print>
#include <ranges>
#include <string_view>

using namespace std::string_view_literals;

using Callback = std::function<void(std::string_view, std::string_view)>;

void iterHeaders(std::string_view req, Callback &&callback) {
    int headers_counter = 0;
    for (auto pair : std::views::split(req, std::string_view("\r\n"))) {
        auto sv = std::string_view(&*pair.begin(), std::ranges::distance(pair));
        auto colon_pos = sv.find(":");
        if (colon_pos != std::string::npos && (headers_counter > 0)) {
            callback(sv.substr(0, colon_pos),
                     sv.substr(colon_pos + 2, sv.size() - colon_pos - 2));  // skip ": "
        } else {
            if (sv.find("HTTP/") != std::string::npos) {  // header found
                headers_counter++;
            }
        }
    }
}

std::pair<std::string, std::string> findHostPort(std::string_view req) {
    const std::string host_string_caption = "Host";
    std::string host;
    std::string port;

    auto get_host_port = [&](std::string_view key, std::string_view value) {
        if (key == host_string_caption) {
            size_t port_separator_pos = value.find(":");
            host = value.substr(0, port_separator_pos);

            if (port_separator_pos != std::string::npos) {
                port = value.substr(port_separator_pos + 1, value.size() - port_separator_pos);
            }
        }
    };

    iterHeaders(req, get_host_port);

    return {host, port};
}

std::optional<size_t> findContentLength(std::string_view rsp) {
    const std::string content_string_caption = "Content-Length";
    std::optional<size_t> content_len = std::nullopt;

    auto get_length = [&](std::string_view key, std::string_view value) {
        if (key == content_string_caption) {
            size_t len = 0;
            auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), len);
            if (ec == std::errc()) {
                content_len = len;
            }
        }
    };

    iterHeaders(rsp, get_length);
    return content_len;
}
