#pragma once
#include <boost/asio/thread_pool.hpp>

class Concurrency {
public:
    static boost::asio::thread_pool pool;
};