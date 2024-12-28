#pragma once

#include <boost/beast/http.hpp>
#include <boost/regex.hpp>
#include <string>
#include <unordered_map>

namespace beast = boost::beast;
namespace http = beast::http;

using Request = const http::request<http::string_body>&;
using Response = http::response<http::string_body>&;
using Match = const boost::smatch&;

extern std::unordered_map<std::string, std::function<http::message_generator(Request req)>> staticGets;
extern std::vector<std::pair<boost::regex, std::function<http::message_generator(Request req, Match match)>>>
    dynamicGets;
extern std::unordered_map<std::string, std::function<http::message_generator(Request req)>> staticPosts;

// Static GET routes

// Add a static GET route by returning a std::string, omitting request and response arguments.
void get(const std::string& path, const std::function<std::string()>& lambda);

// Add a static GET route by returning a std::string.
void get(const std::string& path, const std::function<std::string(Request req, Response res)>& lambda);

// Add a static GET route by returning a http::message_generator (e.g., http::response<http::string_body>).
void get(const std::string& path, std::function<http::message_generator(Request req)> lambda);

// Dynamic GET routes

// Add a dynamic GET route by returning a std::string, omitting request, response and match arguments.
void get(const boost::regex& path, const std::function<std::string()>& lambda);

// Add a dynamic GET route by returning a std::string, omitting request and response arguments.
void get(const boost::regex& path, const std::function<std::string(Match match)>& lambda);

// Add a dynamic GET route by returning a std::string, omitting match argument.
void get(const boost::regex& path, const std::function<std::string(Request req, Response res)>& lambda);

// Add a dynamic GET route by returning a std::string.
void get(const boost::regex& path, const std::function<std::string(Request req, Response res, Match match)>& lambda);

// Add a dynamic GET route by returning a http::message_generator (e.g., http::response<http::string_body>),
// omitting match argument.
void get(const boost::regex& path, const std::function<http::message_generator(Request req)>& lambda);

// Add a dynamic GET route by returning a http::message_generator (e.g., http::response<http::string_body>).
void get(const boost::regex& path, std::function<http::message_generator(Request req, Match match)> lambda);

// Static POST routes

// Add a static POST route by returning a std::string.
void post(const std::string& path, const std::function<std::string(Request req, Response res)>& lambda);

// Add a static POST route by returning a http::message_generator (e.g., http::response<http::string_body>).
void post(const std::string& path, std::function<http::message_generator(Request req)> lambda);
