#pragma once

#include "point.h"

#include <fstream>
#include <string>
#include <vector>


struct Reader {
    Reader(const std::string& filename, std::string separator = ",");

    std::vector<point_max> Read();

private:
    std::ifstream file;
    std::string separator;
};