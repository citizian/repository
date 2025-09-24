// src/HttpResponse.cpp
#include "HttpResponse.h"
#include <sstream>

void HttpResponse::setStatus(int code, const std::string &message) {
    statusCode = code;
    statusMessage = message;
}

void HttpResponse::setHeader(const std::string &key, const std::string &value) {
    headers[key] = value;
}

void HttpResponse::setBody(const std::string &bodyContent) {
    body = bodyContent;
    headers["Content-Length"] = std::to_string(body.size());
}

std::string HttpResponse::toString() const {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    for (auto &h : headers) {
        response << h.first << ": " << h.second << "\r\n";
    }
    response << "\r\n";
    response << body;
    return response.str();
}
