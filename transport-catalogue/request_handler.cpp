#include "request_handler.h"
#include <sstream>
#include <string>

namespace request_handler {

RequestHandler::RequestHandler(const catalogue::TransportCatalogue& catalogue)
    : catalogue_(catalogue) 
{}

std::string RequestHandler::GetBusInfo(const std::string& bus_name) const {
    // Просто запрашиваем информацию из транспортного справочника.
    const auto* bus = catalogue_.GetBus(bus_name);
    std::ostringstream os;
    if (!bus) {
        os << "Bus " << bus_name << ": not found";
    } else {
        auto stats = catalogue_.GetBusStatistics(bus_name);
        double curvature = (stats.geo_length > 0) ? (stats.length / stats.geo_length) : 0.0;
        os << "Bus " << bus_name << ": " 
           << stats.amount << " stops on route, " 
           << stats.unique << " unique stops, " 
           << stats.length << " route length, " 
           << curvature << " curvature";
    }
    return os.str();
}

std::string RequestHandler::GetStopInfo(const std::string& stop_name) const {
    // Просто запрашиваем информацию о остановке из транспортного справочника.
    auto buses_info = catalogue_.GetBusesForStop(stop_name);
    std::ostringstream os;
    if (buses_info.request_status == "not found") {
        os << "Stop " << stop_name << ": not found";
    } else if (buses_info.request_status == "empty") {
        os << "Stop " << stop_name << ": no buses";
    } else {
        os << "Stop " << stop_name << ": buses ";
        bool first = true;
        for (const auto& bus : buses_info.buses) {
            if (!first)
                os << " ";
            os << bus;
            first = false;
        }
    }
    return os.str();
}

} // namespace request_handler
