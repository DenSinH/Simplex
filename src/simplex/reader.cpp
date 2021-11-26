#include "reader.h"

#include <stdexcept>
#include <sstream>


Reader::Reader(const std::string& filename, std::string separator) : separator(std::move(separator)) {
    file = std::ifstream(filename);

    if (!file.good()) {
        throw std::runtime_error("Failed to open file!");
    }
}

std::vector<point3d> Reader::Read() {
    std::vector<point3d> data;
    std::string line;
    const auto seplen = separator.length();
    std::vector<char> sep(seplen);
    sep.resize(seplen);

    auto expect_sep = [&](std::stringstream& ss) {
        ss.read(sep.data(), seplen);
        sep.push_back(0);
        if (separator != sep.data()) {
            throw std::runtime_error("Bad separator");
        }
    };

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        float x, y, z;
        ss >> x;
        expect_sep(ss);
        ss >> y;
        expect_sep(ss);
        ss >> z;

        data.emplace_back(x, y, z);
    }
    return std::move(data);
}
