// include/HttpResponse.h
#pragma once
#include <string>
#include <unordered_map>

class HttpResponse {
public:
    HttpResponse() = default;
    ~HttpResponse() = default;

    void setStatus(int code, const std::string &message);
    void setHeader(const std::string &key, const std::string &value);
    void setBody(const std::string &bodyContent);

    std::string toString() const;   // 转换为 HTTP 响应字符串

private:
    int statusCode = 200;
    std::string statusMessage = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};
