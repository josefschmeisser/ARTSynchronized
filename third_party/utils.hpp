#pragma once

#include <string>

std::string getFileName(const std::string& s) {
    char sep = '/';

#ifdef _WIN32
    sep = '\\';
#endif

    size_t i = s.rfind(sep, s.length());
    if (i != std::string::npos) {
        return(s.substr(i+1, s.length() - i));
    }

    return("");
}

template<class T>
auto splitVector(std::vector<T>& vec, size_t parts) {
    using iterator = typename std::remove_reference<decltype(vec)>::type::iterator;
    std::vector<std::pair<iterator, iterator>> output;

    size_t length = vec.size() / parts;
    size_t remainder = vec.size() % parts;

    size_t begin = 0, end = 0;
    for (size_t i = 0; i < std::min(parts, vec.size()); ++i) {
        end += (remainder > 0) ? (length + !!(remainder--)) : length;
        output.push_back({vec.begin() + begin, vec.begin() + end});
        begin = end;
    }

    return output;
}
