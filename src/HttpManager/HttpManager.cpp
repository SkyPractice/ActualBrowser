#include "HttpManager.h"
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <format>
#include <openssl/ssl.h>
#include <openssl/tls1.h>
#include <boost/beast/http.hpp>
#include <string>
#include <iostream>
#include <filesystem>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>

namespace fs = std::filesystem;

HttpManager* HttpExposer::current_http_manager = nullptr;
std::vector<std::string> HttpExposer::memory_resources_paths = {};

HttpManager::HttpManager(net::io_context& io_context, ssl::context& ssl_context,
        tcp::resolver& our_resolver): io_ctx(io_context), ssl_ctx(ssl_context),
        resolver(our_resolver) {
};

UrlInfo HttpManager::getUrlInfoByUrl(std::string url){
    std::string schema;
    std::string host_name;
    std::string path;

    if(url.starts_with("https://")){
        schema = "https";
        url = url.substr(8);
    } else if (url.starts_with("http://")){
        schema = "http";
        url = url.substr(7);
    } else {
        schema = "https";
    }

    auto pos = url.find('/');
    if(pos != std::string::npos){
        path = url.substr(pos);
        url = url.substr(0, pos);
        host_name = url;
    } else {
        path = "/";
        host_name = url;
    }

    return {schema, host_name, path};
};

std::string HttpManager::getRequest(std::string url){
    if(fs::exists(url)){
        std::ifstream fstrea(url, std::ios::ate);

        size_t size = fstrea.tellg();
        std::string str(size, 0);

        fstrea.seekg(0, std::ios::beg);
        fstrea.read(&str[0], size);
        
        fstrea.close();

        return str;
    };

    UrlInfo url_info = getUrlInfoByUrl(url);
    std::cout << url_info.host_name << url_info.path << url_info.schem;
    ssl::stream<tcp::socket> sock(io_ctx, ssl_ctx);

    SSL_set_tlsext_host_name(sock.native_handle(), url_info.host_name.c_str());
    auto result = resolver.resolve(url_info.host_name, url_info.schem);
    net::connect(sock.lowest_layer(), result);
    sock.handshake(ssl::stream<tcp::socket>::client);

    http::request<http::string_body> req(http::verb::get, url_info.path, 11);
    req.set(beast::http::field::host, url_info.host_name);
    req.set(http::field::user_agent, "MyBrowser (Linux)");

    http::write(sock, req);

    beast::flat_buffer flat_buff;
    http::response<http::string_body> res;

    http::read(sock,flat_buff, res);
    sock.lowest_layer().close();
    return res.body();

};

std::string HttpManager::getImage(std::string url) {

    if(fs::exists(url)){
        return url;
    };

    UrlInfo url_info = getUrlInfoByUrl(url);
    std::cout << url_info.host_name << url_info.path << url_info.schem;
    ssl::stream<tcp::socket> sock(io_ctx, ssl_ctx);

    SSL_set_tlsext_host_name(sock.native_handle(), url_info.host_name.c_str());
    auto result = resolver.resolve(url_info.host_name, url_info.schem);
    net::connect(sock.lowest_layer(), result);
    sock.handshake(ssl::stream<tcp::socket>::client);

    http::request<http::string_body> req(http::verb::get, url_info.path, 11);
    req.set(beast::http::field::host, url_info.host_name);
    req.set(http::field::user_agent, "MyBrowser (Linux)");

    http::write(sock, req);

    beast::flat_buffer flat_buff;
    http::response<http::dynamic_body> res;

    http::read(sock,flat_buff, res);
    
    std::string content_type = res.at(http::field::content_type);
    std::string ext(response_formats.at(content_type));

    boost::uuids::random_generator guid_gen;
    boost::uuids::uuid file_uuid = guid_gen();
    std::string file_name = boost::uuids::to_string(file_uuid);

    std::ofstream strea(std::format("/tmp/{}.{}", file_name, ext), std::ios::binary);

    for(const auto& buf : res.body().data()){
        strea.write(reinterpret_cast<const char*>(buf.data()), buf.size());
    }

    strea.close();

    sock.lowest_layer().close();
    std::string path_name = std::format("/tmp/{}.{}", file_name, ext);
    return path_name;
};

std::string HttpManager::postRequest(std::string url, std::string_view body, std::string content_type){
    UrlInfo url_info = getUrlInfoByUrl(url);

    ssl::stream<tcp::socket> sock(io_ctx, ssl_ctx);

    SSL_set_tlsext_host_name(sock.native_handle(), url_info.host_name.c_str());
    auto result = resolver.resolve(url_info.host_name, url_info.schem);
    net::connect(sock.lowest_layer(), result);
    sock.handshake(ssl::stream<tcp::socket>::client);

    http::request<http::string_body> req(http::verb::post, url_info.path, 11);
    req.set(beast::http::field::host, url_info.host_name);
    req.set(http::field::user_agent, "MyBrowser (Linux)");
    req.set(http::field::content_type, content_type);
    req.body() = body;

    http::write(sock, req);

    beast::flat_buffer flat_buff;
    http::response<http::string_body> res;

    http::read(sock,flat_buff, res);
    sock.lowest_layer().close();
    return res.body();
};
std::string HttpManager::putRequest(std::string url, std::string_view body, std::string content_type){
    UrlInfo url_info = getUrlInfoByUrl(url);

    ssl::stream<tcp::socket> sock(io_ctx, ssl_ctx);

    SSL_set_tlsext_host_name(sock.native_handle(), url_info.host_name.c_str());
    auto result = resolver.resolve(url_info.host_name, url_info.schem);
    net::connect(sock.lowest_layer(), result);
    sock.handshake(ssl::stream<tcp::socket>::client);

    http::request<http::string_body> req(http::verb::put, url_info.path, 11);
    req.set(beast::http::field::host, url_info.host_name);
    req.set(http::field::user_agent, "MyBrowser (Linux)");
    req.set(http::field::content_type, content_type);
    req.body() = body;

    http::write(sock, req);

    beast::flat_buffer flat_buff;
    http::response<http::string_body> res;

    http::read(sock,flat_buff, res);
    sock.lowest_layer().close();
    return res.body();
};
std::string HttpManager::deleteRequest(std::string url){
    UrlInfo url_info = getUrlInfoByUrl(url);

    ssl::stream<tcp::socket> sock(io_ctx, ssl_ctx);

    SSL_set_tlsext_host_name(sock.native_handle(), url_info.host_name.c_str());
    auto result = resolver.resolve(url_info.host_name, url_info.schem);
    net::connect(sock.lowest_layer(), result);
    sock.handshake(ssl::stream<tcp::socket>::client);

    http::request<http::string_body> req(http::verb::delete_, url_info.path, 11);
    req.set(beast::http::field::host, url_info.host_name);
    req.set(http::field::user_agent, "MyBrowser (Linux)");

    http::write(sock, req);

    beast::flat_buffer flat_buff;
    http::response<http::string_body> res;

    http::read(sock,flat_buff, res);
    sock.lowest_layer().close();
    return res.body();
};
