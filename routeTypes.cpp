#include "routeTypes.h"

#include <fstream>

std::unordered_map<std::string, std::function<http::message_generator(Request req)>> staticGets;
std::vector<std::pair<boost::regex, std::function<http::message_generator(Request req, Match match)>>> dynamicGets;
std::unordered_map<std::string, std::function<http::message_generator(Request req)>> staticPosts;

// Static GET routes

void get(const std::string& path, const std::function<std::string()>& lambda) {
    get(path, [lambda](Request req, Response res) { return lambda(); });
}

void get(const std::string& path, const std::function<std::string(Request req, Response res)>& lambda) {
    get(path, [lambda](Request req) {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.keep_alive(req.keep_alive());
        res.set(http::field::content_type, "text/html; charset=utf-8");
        res.body() = lambda(req, res);
        res.prepare_payload();
        return res;
    });
}

void get(const std::string& path, std::function<http::message_generator(Request req)> lambda) {
    staticGets[path] = std::move(lambda);
}

// Dynamic GET routes

void get(const boost::regex& path, const std::function<std::string()>& lambda) {
    get(path, [lambda](Request req, Response res, Match match) { return lambda(); });
}

void get(const boost::regex& path, const std::function<std::string(Match match)>& lambda) {
    get(path, [lambda](Request req, Response res, Match match) { return lambda(match); });
}

void get(const boost::regex& path, const std::function<std::string(Request req, Response res)>& lambda) {
    get(path, [lambda](Request req, Response res, Match match) { return lambda(req, res); });
}

void get(const boost::regex& path, const std::function<std::string(Request req, Response res, Match match)>& lambda) {
    get(path, [lambda](Request req, Match match) {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.keep_alive(req.keep_alive());
        res.set(http::field::content_type, "text/html; charset=utf-8");
        res.body() = lambda(req, res, match);
        res.prepare_payload();
        return res;
    });
}

void get(const boost::regex& path, const std::function<http::message_generator(Request req)>& lambda) {
    get(path, [lambda](Request req, Match match) { return lambda(req); });
}

void get(const boost::regex& path, std::function<http::message_generator(Request req, Match match)> lambda) {
    dynamicGets.emplace_back(path, std::move(lambda));
}

// Static POST routes

void post(const std::string& path, const std::function<std::string(Request req, Response res)>& lambda) {
    post(path, [lambda](Request req) {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.keep_alive(req.keep_alive());
        res.set(http::field::content_type, "text/html; charset=utf-8");
        res.body() = lambda(req, res);
        res.prepare_payload();
        return res;
    });
}

void post(const std::string& path, std::function<http::message_generator(Request req)> lambda) {
    staticPosts[path] = std::move(lambda);
}
