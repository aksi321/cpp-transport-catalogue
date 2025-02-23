#include "request_handler.h"
#include <sstream>

namespace request_handler {

RequestHandler::RequestHandler(const catalogue::TransportCatalogue& catalogue)
    : catalogue_(catalogue) {}

std::string RequestHandler::GetBusInfo(const std::string& bus_name) const {
    std::ostringstream os;
    if (catalogue_.GetBus(bus_name) == nullptr) {
        os << "{ \"request_id\": 0, \"error_message\": \"not found\" }";
    } else {
    auto stats = catalogue_.GetBusStatistics(bus_name);
    double curvature = (stats.geo_length > 0) ? (stats.length / stats.geo_length) : 0.0;
        os << "{ \"request_id\": 0, "
           << "\"stop_count\": " << stats.amount << ", "
           << "\"unique_stop_count\": " << stats.unique << ", "
           << "\"route_length\": " << stats.length << ", "
           << "\"curvature\": " << curvature << " }";
    }
    return os.str();
}

std::string RequestHandler::GetStopInfo(const std::string& stop_name) const {
    auto buses_info = catalogue_.GetBusesForStop(stop_name);
    std::ostringstream os;
   if (buses_info.request_status == "not found") {
        os << "{ \"request_id\": 0, \"error_message\": \"not found\" }";
    } else {
        os << "{ \"request_id\": 0, \"buses\": [";
        bool first = true;
        for (const auto& bus : buses_info.buses) {
            if (!first)
                os << ", ";
            os << "\"" << bus << "\"";
            first = false;
        }
        os << "] }";
    }
    return os.str();
}

}  // namespace request_handler
