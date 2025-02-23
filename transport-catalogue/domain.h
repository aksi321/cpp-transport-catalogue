#pragma once
#include "geo.h"
#include <string>
#include <vector>

namespace domain {
 struct Stop {
        std::string name;
        geo::Coordinates coordinates;
    };
   
struct Bus {
    std::string name;
    std::vector<std::string> stops;
    bool is_roundtrip = false;
};

}  // namespace domain
