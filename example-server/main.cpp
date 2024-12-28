#include <boost/filesystem.hpp>
#include <boost/url.hpp>

#include "webServer/webServer.h"

int main(const int argc, char* argv[]) {
    get("/", [] { return "<h1>Hello World!</h1>"; });

    get("/json", [](Request req, Response res) {
        res.set(http::field::content_type, "application/json");
        return R"({"Hello": "World"})";
    });

    get(boost::regex{"/user/(\\d+)"}, [](Match match) { return "User ID: " + match[1]; });

    get("/redirect", [](Request req) {
        http::response<http::string_body> res{http::status::moved_permanently, req.version()};
        res.set(http::field::location, "/");
        return res;
    });

    get("/params", [](Request req, Response res) -> std::string {
        const boost::urls::url_view url(req.target());
        if (const auto iter = url.params().find("name"); iter != url.params().end()) {
            const auto name(*iter);
            return "Hello " + name.value + "!";
        }
        return "Hello!";
    });

    get("/form", [] {
        return R"(<form action="/form" method="post">)"
               R"(<label for="name">Name:</label><br>)"
               R"(<input type="text" id="name" name="name">)"
               R"(<input type="submit" value="Submit">)"
               R"(</form>)";
    });

    post("/form", [](Request req, Response res) {
        if (auto view = boost::urls::parse_query(req.body())) {
            if (const auto iter = view->find("name"); iter != view->end()) {
                return "Thanks " + iter->value.decode() + " for submitting the form!";
            }
        }
        throw std::runtime_error("Invalid form");
    });

    get(boost::regex("/hello/(?<name>.*)"), [](Match match) -> std::string {
        const std::string name = match["name"].str();
        if (auto view = boost::urls::make_pct_string_view(name)) {
            std::string buf;
            buf.resize(view.value().decoded_size());
            view.value().decode({}, boost::urls::string_token::assign_to(buf));
            return "Hello " + buf + "!";
        }
        return "Hello!";
    });

    get("/file-body", [](Request req) -> http::message_generator {
        beast::error_code ec;
        const boost::filesystem::path filePath = boost::filesystem::canonical("public/hello_world.txt", ec);
        http::file_body::value_type body;
        body.open(filePath.c_str(), beast::file_mode::scan, ec);

        if (ec) {
            http::response<http::string_body> res{http::status::not_found, req.version()};
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "The file was not found!";
            res.prepare_payload();
            return res;
        }
        auto const size = body.size();
        http::response<http::file_body> res{std::piecewise_construct, std::make_tuple(std::move(body)),
                                            std::make_tuple(http::status::ok, req.version())};
        res.set(http::field::content_type, "text/plain");
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return res;
    });

    get(boost::regex(".*"), [](Request req, Response res) {
        res.result(http::status::not_found);
        return "<h1>404 Not Found!</h1>";
    });

    startServer(argc, argv);
    return 0;
}
