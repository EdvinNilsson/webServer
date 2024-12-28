#pragma once

#include <boost/asio/ip/address.hpp>

#include "routeTypes.h"

void startServer(const boost::asio::ip::address& address, unsigned short port, int threads);

void startServer(int argc, char* argv[]);
