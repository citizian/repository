// include/HttpRequest.h
#pragma once
#include <string>
#include <unordered_map>

class HttpRequest {
public:
    HttpRequest() = default;
    ~HttpRequest() = default;

    bool parse(const std::string &raw);    // 解析原始请求

    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

private:
    std::string trim(const std::string &s);
};
