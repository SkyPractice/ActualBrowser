#include <gtkmm/window.h>
#include "HttpManager/HttpManager.h"
#include "GUI/Window.h"
#include <gtkmm/application.h>
#include "Concurrency/ThreadPool.h"

boost::asio::thread_pool Concurrency::pool = boost::asio::thread_pool(4);

int main(int argc, char** argv){
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    net::io_context io_ctx;
    ssl::context ssl_ctx(ssl::context::tls_client);
    tcp::resolver resolver(io_ctx);
    ssl_ctx.set_verify_mode(ssl::context::verify_peer);
    ssl_ctx.load_verify_file("cacert.pem");
    
    HttpManager man(io_ctx, ssl_ctx, resolver);
    HttpExposer::current_http_manager = &man;
    auto app = Gtk::Application::create("org.gtkmm.example");

    return app->make_window_and_run<BrowserWindow>(argc, argv, man);
}