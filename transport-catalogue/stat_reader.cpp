#include "stat_reader.h"
#include <iostream>

namespace utility {

void PrintBus(const TransportCatalogue& catalogue, std::string_view left, std::string_view right, std::ostream& output) {
    BusCounted rez = catalogue.GetBusStatistics(right);
    if (!rez.IsEmpty()) {
        double curvature = (rez.geo_length > 0) ? rez.length / rez.geo_length : 0.0;
        output << left << " " << right << ": " << rez.amount << " stops on route, " 
               << rez.unique << " unique stops, " << rez.length << " route length, " 
               << curvature << " curvature";
    } else {
        output << left << " " << right << ": not found";
    }
}

void PrintStop(const TransportCatalogue& catalogue, std::string_view left, std::string_view right, std::ostream& output) {
    BusesForStop answer = catalogue.GetBusesForStop(right);
    output << left << " " << right << ":";
    if (answer.request_status == "not found") {
        output << " not found";
    } else if (answer.request_status == "empty") {
        output << " no buses";
    } else if (answer.request_status == "found") {
        output << " buses";
        for (auto s : answer.buses) {
            output << " " << s;
        }
    }
}

void ParseAndPrintStat(const TransportCatalogue& catalogue, std::string_view request, std::ostream& output) {
    size_t pos = request.find(' ');
    std::string_view left = request.substr(0, pos);
    std::string_view right = request.substr(pos + 1);
    if (left == "Bus") {
        PrintBus(catalogue, left, right, output);
    } else if (left == "Stop") {
        PrintStop(catalogue, left, right, output);
    }
    output << std::endl;
}

} // namespace utility
