#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>

namespace json_reader {

void ReadTransportCatalogue(const json::Document& doc, catalogue::TransportCatalogue& catalogue) {
    const auto& root = doc.GetRoot().AsMap();
    const auto& base_requests = root.at("base_requests").AsArray();

    for (const auto& node : base_requests) {
        const auto& obj = node.AsMap();
        if (obj.at("type").AsString() == "Stop") {
            std::string name = obj.at("name").AsString();
            double lat = obj.at("latitude").AsDouble();
            double lng = obj.at("longitude").AsDouble();
            catalogue.AddStop(name, lat, lng);
        }
    }

    for (const auto& node : base_requests) {
        const auto& obj = node.AsMap();
        if (obj.at("type").AsString() == "Stop") {
            std::string name = obj.at("name").AsString();
            if (obj.count("road_distances") > 0) {
                const auto& distances = obj.at("road_distances").AsMap();
                for (const auto& [neighbor, distance_node] : distances) {
                    int distance = distance_node.AsInt();
                    catalogue.AddDistance(name, neighbor, distance);
                }
            }
        } else if (obj.at("type").AsString() == "Bus") {
            std::string bus_name = obj.at("name").AsString();
            bool is_roundtrip = obj.at("is_roundtrip").AsBool();
            std::vector<std::string_view> bus_stops;
            for (const auto& stop_node : obj.at("stops").AsArray()) {
                bus_stops.push_back(stop_node.AsString());
            }
            catalogue.AddBus(bus_name, bus_stops, is_roundtrip);
        }
    }
}

json::Document ProcessRequests(const json::Document& doc, const catalogue::TransportCatalogue& catalogue) {
    using namespace json;
    Array responses;
    const auto& root = doc.GetRoot().AsMap();
    const auto& stat_requests = root.at("stat_requests").AsArray();
    
    renderer::RenderSettings render_settings = ParseRenderSettings(doc);

    for (const auto& request_node : stat_requests) {
        const auto& req = request_node.AsMap();
        int request_id = req.at("id").AsInt();
        std::string type = req.at("type").AsString();
        Dict response;
        response["request_id"] = Node(request_id);

        if (type == "Bus") {
            std::string bus_name = req.at("name").AsString();
            if (catalogue.GetBus(bus_name) == nullptr) {
                response["error_message"] = Node(std::string("not found"));
            } else {
                auto stats = catalogue.GetBusStatistics(bus_name);
                double curvature = (stats.geo_length > 0) ? (stats.length / stats.geo_length) : 0.0;
                response["stop_count"] = Node(static_cast<int>(stats.amount));
                response["unique_stop_count"] = Node(static_cast<int>(stats.unique));
                response["route_length"] = Node(static_cast<int>(stats.length));
                response["curvature"] = Node(curvature);
            }
        } else if (type == "Stop") {
            std::string stop_name = req.at("name").AsString();
            auto buses_info = catalogue.GetBusesForStop(stop_name);
            if (buses_info.request_status == "not found") {
                response["error_message"] = Node(std::string("not found"));
            } else {
                std::vector<std::string> sorted_buses;
                for (const auto& bus : buses_info.buses) {
                    sorted_buses.push_back(std::string(bus));
                }
                std::sort(sorted_buses.begin(), sorted_buses.end());
                Array buses;
                for (const auto& name : sorted_buses) {
                    buses.push_back(Node(name));
                }
                response["buses"] = Node(buses);
            }
        } else if (type == "Map") {
            std::ostringstream oss;
            renderer::RenderMap(catalogue, render_settings, oss);
            response["map"] = Node(oss.str());
        }
        responses.push_back(Node(response));
    }
    return Document(Node(responses));
}

renderer::RenderSettings ParseRenderSettings(const json::Document& doc) {
    renderer::RenderSettings settings;
    const auto& root = doc.GetRoot().AsMap();
    if (!root.count("render_settings")) {
        return settings;
    }
    const auto& render_settings = root.at("render_settings").AsMap();
    if (render_settings.count("width")) {
        settings.width = render_settings.at("width").AsDouble();
    }
    if (render_settings.count("height")) {
        settings.height = render_settings.at("height").AsDouble();
    }
    if (render_settings.count("padding")) {
        settings.padding = render_settings.at("padding").AsDouble();
    }
    if (render_settings.count("line_width")) {
        settings.line_width = render_settings.at("line_width").AsDouble();
    }
    if (render_settings.count("stop_radius")) {
        settings.stop_radius = render_settings.at("stop_radius").AsDouble();
    }
    if (render_settings.count("bus_label_font_size")) {
        settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
    }
    if (render_settings.count("bus_label_offset")) {
        const auto& offset = render_settings.at("bus_label_offset").AsArray();
        settings.bus_label_offset = {offset[0].AsDouble(), offset[1].AsDouble()};
    }
    if (render_settings.count("stop_label_font_size")) {
        settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
    }
    if (render_settings.count("stop_label_offset")) {
        const auto& offset = render_settings.at("stop_label_offset").AsArray();
        settings.stop_label_offset = {offset[0].AsDouble(), offset[1].AsDouble()};
    }
    if (render_settings.count("underlayer_color")) {
        const auto& color = render_settings.at("underlayer_color");
        if (color.IsString()) {
            settings.underlayer_color = color.AsString();
        } else if (color.IsArray()) {
            const auto& color_arr = color.AsArray();
            if (color_arr.size() == 3) {
                settings.underlayer_color = "rgb(" + std::to_string(color_arr[0].AsInt()) + "," +
                                              std::to_string(color_arr[1].AsInt()) + "," +
                                              std::to_string(color_arr[2].AsInt()) + ")";
            } else if (color_arr.size() == 4) {
                std::ostringstream oss;
                oss << "rgba(" 
                    << color_arr[0].AsInt() << ","
                    << color_arr[1].AsInt() << ","
                    << color_arr[2].AsInt() << ","
                    << std::defaultfloat << color_arr[3].AsDouble()
                    << ")";
                settings.underlayer_color = oss.str();
            }
        }
    }
    if (render_settings.count("underlayer_width")) {
        settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();
    }
    if (render_settings.count("color_palette")) {
        const auto& palette = render_settings.at("color_palette").AsArray();
        settings.color_palette.clear();
        for (const auto& color : palette) {
            if (color.IsString()) {
                settings.color_palette.push_back(color.AsString());
            } else if (color.IsArray()) {
                const auto& color_arr = color.AsArray();
                if (color_arr.size() == 3) {
                    settings.color_palette.push_back("rgb(" + std::to_string(color_arr[0].AsInt()) + "," +
                                                     std::to_string(color_arr[1].AsInt()) + "," +
                                                     std::to_string(color_arr[2].AsInt()) + ")");
                } else if (color_arr.size() == 4) {
                    std::ostringstream oss;
                    oss << "rgba(" 
                        << color_arr[0].AsInt() << ","
                        << color_arr[1].AsInt() << ","
                        << color_arr[2].AsInt() << ","
                        << std::defaultfloat << color_arr[3].AsDouble()
                        << ")";
                    settings.color_palette.push_back(oss.str());
                }
            }
        }
    }
    return settings;
}

} // namespace json_reader
