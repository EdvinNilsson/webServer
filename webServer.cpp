#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "routeTypes.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

std::string mimeType(const std::string& path) {
    static const std::unordered_map<std::string, std::string> mimeMap = {
        {".7z", "application/x-7z-compressed"},
        {".aac", "audio/aac"},
        {".abw", "application/x-abiword"},
        {".apng", "image/apng"},
        {".arc", "application/x-freearc"},
        {".avi", "video/x-msvideo"},
        {".avif", "image/avif"},
        {".azw", "application/vnd.amazon.ebook"},
        {".bin", "application/octet-stream"},
        {".bmp", "image/bmp"},
        {".bz", "application/x-bzip"},
        {".bz2", "application/x-bzip2"},
        {".cda", "application/x-cdf"},
        {".csh", "application/x-csh"},
        {".css", "text/css"},
        {".csv", "text/csv"},
        {".doc", "application/msword"},
        {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
        {".eot", "application/vnd.ms-fontobject"},
        {".epub", "application/epub+zip"},
        {".gif", "image/gif"},
        {".gz", "application/gzip"},
        {".htm", "text/html"},
        {".html", "text/html"},
        {".ico", "image/vnd.microsoft.icon"},
        {".ics", "text/calendar"},
        {".jar", "application/java-archive"},
        {".jpeg", "image/jpeg"},
        {".jpg", "image/jpeg"},
        {".js", "text/javascript"},
        {".json", "application/json"},
        {".jsonld", "application/ld+json"},
        {".mid", "audio/midi"},
        {".midi", "audio/midi"},
        {".mjs", "text/javascript"},
        {".mkv", "video/x-matroska"},
        {".mp3", "audio/mpeg"},
        {".mp4", "video/mp4"},
        {".mpeg", "video/mpeg"},
        {".mpkg", "application/vnd.apple.installer+xml"},
        {".odp", "application/vnd.oasis.opendocument.presentation"},
        {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
        {".odt", "application/vnd.oasis.opendocument.text"},
        {".oga", "audio/ogg"},
        {".ogv", "video/ogg"},
        {".ogx", "application/ogg"},
        {".opus", "audio/ogg"},
        {".otf", "font/otf"},
        {".pdf", "application/pdf"},
        {".php", "application/x-httpd-php"},
        {".png", "image/png"},
        {".ppt", "application/vnd.ms-powerpoint"},
        {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
        {".rar", "application/vnd.rar"},
        {".rtf", "application/rtf"},
        {".sh", "application/x-sh"},
        {".svg", "image/svg+xml"},
        {".tar", "application/x-tar"},
        {".tif", "image/tiff"},
        {".tiff", "image/tiff"},
        {".ts", "video/mp2t"},
        {".ttf", "font/ttf"},
        {".txt", "text/plain"},
        {".vsd", "application/vnd.visio"},
        {".wav", "audio/wav"},
        {".weba", "audio/webm"},
        {".webm", "video/webm"},
        {".webp", "image/webp"},
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".xhtml", "application/xhtml+xml"},
        {".xls", "application/vnd.ms-excel"},
        {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
        {".xml", "application/xml"},
        {".xul", "application/vnd.mozilla.xul+xml"},
        {".zip", "application/zip"},
    };

    if (auto const pos = path.rfind('.'); pos != std::string::npos) {
        std::string ext(path.substr(pos));
        std::ranges::transform(ext, ext.begin(), tolower);

        if (const auto iter = mimeMap.find(ext); iter != mimeMap.end()) return iter->second;
    }

    return "application/text";
}

template <class Body, class Allocator>
http::message_generator handleRequest(http::request<Body, http::basic_fields<Allocator> >&& req) {
    auto const badRequest = [&req](const beast::string_view why, http::status status = http::status::bad_request) {
        http::response<http::string_body> res{status, req.version()};
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        std::cerr << req.method() << " " << res.result_int() << " " << req.target() << " " << why << std::endl;
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    auto const notFound = [&req](beast::string_view target) {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.keep_alive(req.keep_alive());
        std::cerr << req.method() << " " << res.result_int() << " " << target << std::endl;
        res.prepare_payload();
        return res;
    };

    auto const serverError = [&req](const beast::string_view what) {
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.keep_alive(req.keep_alive());
        std::cerr << req.method() << " " << res.result_int() << " " << req.target() << " " << what << std::endl;
        res.prepare_payload();
        return res;
    };

    if (req.method() != http::verb::get && req.method() != http::verb::head && req.method() != http::verb::post)
        return badRequest("Unknown HTTP-method", http::status::not_implemented);

    const size_t pos = req.target().find('?');
    const std::string path = pos != std::string::npos ? req.target().substr(0, pos) : req.target();

    if (path.empty() || path[0] != '/' || path.find("..") != beast::string_view::npos)
        return badRequest("Illegal request-target");

    std::cout << req.method() << " " << req.target() << std::endl;

    // Handle static POST requests
    if (req.method() == http::verb::post) {
        if (const auto iter = staticPosts.find(path); iter != staticPosts.end()) {
            try {
                return iter->second(req);
            } catch (const std::exception& e) {
                return serverError(e.what());
            }
        }
    }

    // Handle static GET requests
    if (const auto iter = staticGets.find(path); iter != staticGets.end()) {
        try {
            return iter->second(req);
        } catch (const std::exception& e) {
            return serverError(e.what());
        }
    }

    // Handle static file requests
    beast::error_code ec;
    const boost::filesystem::path filePath = boost::filesystem::weakly_canonical("public" + path, ec);

    if (!ec && is_regular_file(filePath)) {
        http::file_body::value_type body;
        body.open(filePath.c_str(), beast::file_mode::scan, ec);

        if (ec) return serverError(ec.message());

        auto const size = body.size();

        http::response<http::file_body> res{std::piecewise_construct, std::make_tuple(std::move(body)),
                                            std::make_tuple(http::status::ok, req.version())};
        res.set(http::field::content_type, mimeType(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return res;
    }

    // Handle dynamic GET requests
    for (const auto& [regex, lambda] : dynamicGets) {
        if (boost::smatch match; regex_match(path, match, regex)) {
            try {
                return lambda(req, match);
            } catch (const std::exception& e) {
                return serverError(e.what());
            }
        }
    }

    return notFound(path);
}

net::awaitable<void> session(beast::tcp_stream stream) {
    beast::flat_buffer buffer;

    while (true) {
        stream.expires_after(std::chrono::seconds(30));

        http::request<http::string_body> req;
        co_await http::async_read(stream, buffer, req, boost::asio::deferred);

        http::message_generator msg = handleRequest(std::move(req));

        const bool keep_alive = msg.keep_alive();

        co_await beast::async_write(stream, std::move(msg), boost::asio::deferred);

        if (!keep_alive) {
            break;
        }
    }

    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send);
}

net::awaitable<void> listen(const net::ip::tcp::endpoint endpoint) {
    const auto executor = co_await net::this_coro::executor;
    auto acceptor = net::ip::tcp::acceptor{executor, endpoint};

    while (true) {
        co_spawn(executor, session(beast::tcp_stream{co_await acceptor.async_accept(boost::asio::deferred)}),
                 boost::asio::detached);
    }
}

void startServer(const net::ip::address& address, const unsigned short port, const int threads) {
    net::io_context ioc{threads};
    co_spawn(ioc, listen(net::ip::tcp::endpoint{address, port}), [](const std::exception_ptr& eptr) {
        try {
            if (eptr) std::rethrow_exception(eptr);
        } catch (std::exception const& e) {
            std::cerr << e.what() << std::endl;
        }
    });

    std::vector<std::thread> vec(threads - 1);
    for (auto i = 0; i < threads - 1; ++i) vec.emplace_back([&ioc] { ioc.run(); });
    ioc.run();
}

void startServer(const int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: <address> <port> <threads>\n"
                  << "Example:\n"
                  << "    ./example-server 0.0.0.0 8080 4\n";
        return;
    }

    auto const address = net::ip::make_address(argv[1]);
    const unsigned short port = atoi(argv[2]);
    const int threads = std::max(1, atoi(argv[3]));

    startServer(address, port, threads);
}
