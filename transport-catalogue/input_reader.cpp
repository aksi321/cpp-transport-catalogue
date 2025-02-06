#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cctype>

namespace input_pars {
using namespace geo_math;
namespace parsers {

Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");
    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');
    if (comma == std::string_view::npos)
        return {nan, nan};
    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    try {
        double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        double lng = std::stod(std::string(str.substr(not_space2)));
        return {lat, lng};
    } catch (...) {
        return {nan, nan};
    }
}

std::string_view Trim(std::string_view s) {
    const auto start = s.find_first_not_of(' ');
    if (start == std::string_view::npos)
        return {};
    const auto end = s.find_last_not_of(' ');
    return s.substr(start, end - start + 1);
}

std::vector<std::string_view> Split(std::string_view s, char delim) {
    std::vector<std::string_view> result;
    size_t pos = 0;
    while (pos < s.size()) {
        while (pos < s.size() && s[pos] == ' ')
            ++pos;
        if (pos == s.size())
            break;
        size_t pos2 = s.find(delim, pos);
        if (pos2 == std::string_view::npos)
            pos2 = s.size();
        auto token = Trim(s.substr(pos, pos2 - pos));
        if (!token.empty())
            result.push_back(token);
        pos = pos2 + 1;
    }
    return result;
}

std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != std::string_view::npos) {
        return Split(route, '>');
    }
    auto stops = Split(route, '-');
    std::vector<std::string_view> result(stops.begin(), stops.end());
    if (!stops.empty())
        result.insert(result.end(), std::next(stops.rbegin()), stops.rend());
    return result;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == std::string_view::npos)
        return {};
    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos)
        return {};
    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos)
        return {};
    return { std::string(line.substr(0, space_pos)),
             std::string(line.substr(not_space, colon_pos - not_space)),
             std::string(line.substr(colon_pos + 1)) };
}

bool IsNumber(std::string_view s) {
    if (s.empty())
        return false;
    for (char ch : s)
        if (!std::isdigit(ch))
            return false;
    return true;
}

} // namespace parsers

void InputReader::ParseLine(std::string_view line) {
    using namespace parsers;
    auto command_description = ParseCommandDescription(line);
    if (command_description)
        commands_.push_back(std::move(command_description));
}

void InputReader::ApplyCommands(TransportCatalogue& catalogue) const {
    using namespace parsers;
    for (auto& com : commands_) {
        if (com.command == "Stop") {
            auto tokens = Split(com.description, ',');
            if (tokens.size() < 2)
                continue;
            try {
                double lat = std::stod(std::string(Trim(tokens[0])));
                double lng = std::stod(std::string(Trim(tokens[1])));
                catalogue.AddStop(com.id, lat, lng);
            } catch (...) {
                continue;
            }
        }
    }
    for (auto& com : commands_) {
        if (com.command == "Bus") {
            auto route = ParseRoute(com.description);
            catalogue.AddBus(com.id, route);
        }
    }
    for (auto& com : commands_) {
        if (com.command == "Stop") {
            auto tokens = Split(com.description, ',');
            if (tokens.size() < 3)
                continue; 
            for (size_t i = 2; i < tokens.size(); ++i) {
                auto entry = Trim(tokens[i]);
                if (entry.empty())
                    continue;
                size_t m_pos = entry.find('m');
                if (m_pos == std::string_view::npos)
                    continue;
                auto number_part = Trim(entry.substr(0, m_pos));
                if (!IsNumber(number_part))
                    continue;
                int distance = 0;
                try {
                    distance = std::stoi(std::string(number_part));
                } catch (const std::invalid_argument&) {
                    continue;
                }
                size_t to_pos = entry.find("to", m_pos);
                if (to_pos == std::string_view::npos)
                    continue;
                auto neighbor = Trim(entry.substr(to_pos + 2));
                if (neighbor.empty())
                    continue;
                catalogue.AddDistance(com.id, std::string(neighbor), distance);
            }
        }
    }
}
}
