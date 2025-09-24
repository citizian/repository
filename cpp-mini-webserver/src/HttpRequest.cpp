// src/HttpRequest.cpp
#include "HttpRequest.h"
#include <sstream>
#include <algorithm>

bool HttpRequest::parse(const std::string &raw) {
    std::istringstream stream(raw);
    std::string line;

    // 解析请求行
    if (!std::getline(stream, line)) return false;
    line = trim(line);
    std::istringstream lineStream(line);
    if (!(lineStream >> method >> path >> version)) return false;

    // 解析请求头
    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty()) break; // 空行分隔头和 body
        auto pos = line.find(':');
        if (pos == std::string::npos) continue;
        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));
        headers[key] = value;
    }

    // 解析 body
    std::string bodyContent;
    while (std::getline(stream, line)) {
        bodyContent += line + "\n";
    }
    if (!bodyContent.empty() && bodyContent.back() == '\n') bodyContent.pop_back();
    body = bodyContent;

    return true;
}

std::string HttpRequest::trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \r\n\t");
    size_t end = s.find_last_not_of(" \r\n\t");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}
