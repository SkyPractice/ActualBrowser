#pragma once
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ssl.hpp>
#include <string>
#include <string_view>
#include <string>
#include <unordered_map>

using namespace std::literals::string_view_literals;

namespace beast = boost::beast;
namespace net = boost::asio;
namespace ssl = net::ssl;
namespace http = beast::http;
using tcp = net::ip::tcp;


struct UrlInfo {
    std::string schem;
    std::string host_name;
    std::string path;
};

class HttpManager {
public:
    net::io_context& io_ctx;
    ssl::context& ssl_ctx;
    tcp::resolver& resolver;

    std::unordered_map<std::string_view, std::string_view> response_formats = {
        {"image/png"sv, "png"sv}, {"image/jpeg"sv, "jpg"sv}, {"image/webp"sv, "webp"sv}
    };

    HttpManager(net::io_context& io_context, ssl::context& ssl_context,
        tcp::resolver& our_resolver);

    // all the methods are synchronous intentionally
    std::string getRequest(std::string url);
    std::string postRequest(std::string url, std::string_view body, std::string content_type);
    std::string putRequest(std::string url, std::string_view body, std::string content_type);
    std::string deleteRequest(std::string url);
    std::string getImage(std::string url);

    UrlInfo getUrlInfoByUrl(std::string url);
};

class HttpExposer {
public:
    static HttpManager* current_http_manager;
    static std::vector<std::string> memory_resources_paths;
};

