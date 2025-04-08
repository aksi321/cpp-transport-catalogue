#include "request_handler.h"
#include "json_builder.h"

namespace request_handler {

RequestHandler::RequestHandler(const catalogue::TransportCatalogue& catalogue)
    : catalogue_(catalogue) {}

json::Node RequestHandler::GetBusInfo(const std::string& bus_name, int request_id) const {
    json::Builder builder;
    
    if (bus_name.empty()) {
        return builder.StartDict()
            .Key("request_id").Value(request_id)
            .Key("error_message").Value("not found")
            .EndDict()
            .Build();
    }

    const auto* bus = catalogue_.GetBus(bus_name);
    if (!bus) {
        return builder.StartDict()
            .Key("request_id").Value(request_id)
            .Key("error_message").Value("not found")
            .EndDict()
            .Build();
    }

    auto stats = catalogue_.GetBusStatistics(bus_name);
    double curvature = (stats.geo_length > 0) ? (stats.length / stats.geo_length) : 0.0;

    return builder.StartDict()
        .Key("request_id").Value(request_id)
        .Key("route_length").Value(static_cast<int>(stats.length))
        .Key("curvature").Value(curvature)
        .Key("stop_count").Value(static_cast<int>(stats.amount))
        .Key("unique_stop_count").Value(static_cast<int>(stats.unique))
        .EndDict()
        .Build();
}

json::Node RequestHandler::GetStopInfo(const std::string& stop_name, int request_id) const {
    json::Builder builder;

    if (stop_name.empty() || !catalogue_.GetStop(stop_name)) {
        return builder.StartDict()
            .Key("request_id").Value(request_id)
            .Key("error_message").Value("not found")
            .EndDict()
            .Build();
    }

    const auto& buses = catalogue_.GetBusesForStop(stop_name);

    builder.StartDict()
        .Key("request_id").Value(request_id)
        .Key("buses").StartArray();

    for (const auto& bus : buses) {
        builder.Value(std::string(bus));
    }

    builder.EndArray();
    return builder.EndDict().Build();
}

}  // namespace request_handler
