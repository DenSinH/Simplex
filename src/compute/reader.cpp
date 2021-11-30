#include "reader.h"

#include <stdexcept>
#include <sstream>


Reader::Reader(const std::string& filename, std::string separator) : separator(std::move(separator)) {
    file = std::ifstream(filename);

    if (!file.good()) {
        throw std::runtime_error("Failed to open file!");
    }
}

std::vector<point_max> Reader::Read() {
    std::vector<point_max> data;
    std::string line;
    const auto seplen = separator.length();
    std::vector<char> sep(seplen);
    sep.resize(seplen);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        point_max point{};
        int c = 0;
        while (ss && (c < point_max::dim)) {
            float x;
            ss >> x;
            if (ss) {
                ss.read(sep.data(), seplen);
                sep.push_back(0);
                if (separator != sep.data()) {
                    throw std::runtime_error("Bad separator");
                }
            }
            point[c++] = x;
        }

        data.push_back(point);
    }
    return std::move(data);
}
