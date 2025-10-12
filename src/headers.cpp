#include "headers.h"

#include <ranges>
#include <string_view>

using namespace std::string_view_literals;

using Callback = std::function<void(std::string_view, std::string_view)>;

void iterHeaders(std::string_view req, Callback &&callback) {
    // code here
}

std::pair<std::string, std::string> findHostPort(std::string_view req) {
    const std::string host_string_caption = "Host: ";
    std::string host;
    std::string port;

    size_t index_start = 0;
    size_t index_stop = 0;
    if ((index_start = req.find(host_string_caption)) == std::string::npos) {
        return {};
    }
    index_stop = req.find_first_of("\r", index_start);

    if (index_stop != std::string::npos) {
        index_start += host_string_caption.size();  // we need position after "Host: "
        std::string_view host_port = req.substr(index_start, index_stop - index_start);
        size_t port_separ_pos = host_port.find(":");
        host = host_port.substr(0, port_separ_pos);
        if (port_separ_pos != std::string::npos) {
            port = host_port.substr(port_separ_pos, host_port.size() - port_separ_pos);

            return {host, port};
        }

        return {host, ""};
    }

    return {};
}

std::optional<size_t> findContentLength(std::string_view rsp) {
    const std::string content_string_caption = "Content-Length: ";

    size_t content_size = 0;
    size_t content_string_pos = 0;
    size_t index_start = 0;
    size_t index_stop = 0;

    if ((content_string_pos = rsp.find(content_string_caption)) != std::string::npos) {
        index_stop = rsp.find_first_of("\r", content_string_pos);
        index_start =
            content_string_pos +
            content_string_caption
                .size();  // make position shift to start with the content size value itself
        content_size =
            std::strtoul(rsp.substr(index_start, index_stop - index_start).data(), nullptr, 10);

        return content_size;
    }

    return std::nullopt;
}
