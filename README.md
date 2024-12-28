# Easy-to-use and Fast C++ HTTP Web Server built on Boost.Beast

## Features

- Static and Dynamic Routes
- Multi-threaded
- Support for GET, POST and HEAD requests
- Static files

## Defining routes

Static and dynamic routes can be defined using the `get` and `post` functions. Routes are resolved in the following order: static routes, static files, dynamic routes. Static files are served from the `public` directory in the current working directory, and the server handles MIME types for common file types.

### Static routes

Routes can be defined by returning a `std::string`. By default, the context type is `text/html` and the status is `200 OK`.

```c++
get("/", [] {
    return "Hello World!";
});
```

The request (`const http::request<http::string_body>&`) and the response (`http::response<http::string_body>&`) can be accessed as parameters in the lambda function.

```c++
get("/json", [](Request req, Response res) {
    res.set(http::field::content_type, "application/json");
    return R"({"Hello": "World"})";
});
```

Routes can also be defined by returning a `http::response` to handle responses without body or with non-string bodies.  

```c++
get("/redirect", [](Request req) {
    http::response<http::string_body> res{http::status::moved_permanently, req.version()};
    res.set(http::field::location, "/");
    return res;
});
```

### Dynamic routes

Dynamic routes allow you to define routes using regular expressions (`boost::regex`). Dynamic routes are resolved in the same order as they are defined.

```c++
get(boost::regex(".*"), [](Request req, Response res) {
    res.result(http::status::not_found);
    return "<h1>404 Not Found!</h1>";
});
```

The matched values can be accessed by adding `Match` (`const boost::smatch&`) as the last parameter.

```c++
get(boost::regex{"/user/(\\d+)"}, [](Match match) {
    return "User ID: " + match[1];
});
```

## Example

A complete example server can be found in [example-server](example-server).

```c++
#include "webServer/webServer.h"

int main(const int argc, char* argv[]) {
    get("/", [] {
        return "<h1>Hello World!</h1>";
    });
    
    get("/json", [](Request req, Response res) {
        res.set(http::field::content_type, "application/json");
        return R"({"Hello": "World"})";
    });
    
    get("/redirect", [](Request req) {
        http::response<http::string_body> res{http::status::moved_permanently, req.version()};
        res.set(http::field::location, "/");
        return res;
    });
    
    get(boost::regex(".*"), [](Request req, Response res) {
        res.result(http::status::not_found);
        return "<h1>404 Not Found!</h1>";
    });
    
    startServer(argc, argv);
    return 0;
}
```

## Requirements
- C++ 20 or later
- Boost 1.83 or later
