#include "GarbageCollector.h"
#include <boost/asio/thread_pool.hpp>
boost::asio::thread_pool GarbageCollector::collector_pool = boost::asio::thread_pool(4);
std::vector<RunTimeVal*> GarbageCollector::alive_vals = {};
int GarbageCollector::unchecked_allocs_count = 0;