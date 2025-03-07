#include "json_reader.h"
#include "json_builder.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <set>
#include <optional>
#include <stdexcept>
#include <iomanip>

namespace json_reader {

InputData ParseInputData(const json::Document& doc) {
    InputData input;
    const auto& root = doc.GetRoot().AsDict();
    const auto& base_requests = root.at("base_requests").AsArray();
    
    for (const auto& node : base_requests) {
        const auto& obj = node.AsDict();
        std::string type = obj.at("type").AsString();
        if (type == "Stop") {
            StopData stop;
            stop.name = obj.at("name").AsString();
            stop.latitude = obj.at("latitude").AsDouble();
            stop.longitude = obj.at("longitude").AsDouble();
            if (obj.count("road_distances") > 0) {
                const auto& distances = obj.at("road_distances").AsDict();
                for (const auto& [neighbor, distance_node] : distances) {
                    stop.road_distances[neighbor] = distance_node.AsInt();
                }
            }
            input.stops.push_back(std::move(stop));
        } else if (type == "Bus") {
            BusData bus;
            bus.name = obj.at("name").AsString();
            bus.is_roundtrip = obj.at("is_roundtrip").AsBool();
            const auto& stops_array = obj.at("stops").AsArray();
            for (const auto& stop_node : stops_array) {
                bus.stops.push_back(stop_node.AsString());
            }
            input.buses.push_back(std::move(bus));
        }
    }
    if (root.count("render_settings")) {
        input.render_settings = ParseRenderSettings(doc);
    }
    return input;
}

void FillTransportCatalogue(const InputData& input, catalogue::TransportCatalogue& catalogue) {
    for (const auto& stop : input.stops) {
        catalogue.AddStop(stop.name, stop.latitude, stop.longitude);
    }
    for (const auto& stop : input.stops) {
        for (const auto& [neighbor, distance] : stop.road_distances) {
            catalogue.AddDistance(stop.name, neighbor, distance);
        }
    }
    for (const auto& bus : input.buses) {
        std::vector<std::string_view> bus_stops;
        for (const auto& stop : bus.stops) {
            bus_stops.push_back(stop);
        }
        catalogue.AddBus(bus.name, bus_stops, bus.is_roundtrip);
    }
}

renderer::RenderSettings ParseRenderSettings(const json::Document& doc) {
    renderer::RenderSettings settings;
    const auto& root = doc.GetRoot().AsDict();
    if (!root.count("render_settings"))
        return settings;
    const auto& render_settings = root.at("render_settings").AsDict();
    
    if (render_settings.count("width"))
        settings.width = render_settings.at("width").AsDouble();
    if (render_settings.count("height"))
        settings.height = render_settings.at("height").AsDouble();
    if (render_settings.count("padding"))
        settings.padding = render_settings.at("padding").AsDouble();
    if (render_settings.count("line_width"))
        settings.line_width = render_settings.at("line_width").AsDouble();
    if (render_settings.count("stop_radius"))
        settings.stop_radius = render_settings.at("stop_radius").AsDouble();
    if (render_settings.count("bus_label_font_size"))
        settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
    if (render_settings.count("bus_label_offset")) {
        const auto& offset = render_settings.at("bus_label_offset").AsArray();
        settings.bus_label_offset = std::make_pair(offset[0].AsDouble(), offset[1].AsDouble());
    }
    if (render_settings.count("stop_label_font_size"))
        settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
    if (render_settings.count("stop_label_offset")) {
        const auto& offset = render_settings.at("stop_label_offset").AsArray();
        settings.stop_label_offset = std::make_pair(offset[0].AsDouble(), offset[1].AsDouble());
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

    if (render_settings.count("underlayer_width"))
        settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();
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

json::Document ProcessRequests(const json::Document& doc, const catalogue::TransportCatalogue& catalogue, const renderer::RenderSettings& render_settings) {
    json::Builder builder;
    builder.StartArray();
    
    const auto& requests = doc.GetRoot().AsDict();
    
    if (!requests.count("stat_requests")) {
        return json::Document(builder.EndArray().Build());
    }

    const auto& stat_requests = requests.at("stat_requests").AsArray();
    request_handler::RequestHandler handler(catalogue);
    std::ostringstream map_output;
    
    for (const auto& request : stat_requests) {
        if (!request.IsDict()) {
            continue;
        }

        const auto& request_dict = request.AsDict();
        if (!request_dict.count("id") || !request_dict.count("type")) {
            continue;  
        }

        int request_id = request_dict.at("id").AsInt();
        const std::string& type = request_dict.at("type").AsString();

        if (type == "Bus") {
            if (!request_dict.count("name")) {
                builder.Value(json::Builder()
                    .StartDict()
                    .Key("request_id").Value(request_id)
                    .Key("error_message").Value("not found")
                    .EndDict()
                    .Build());
                continue;
            }
            std::string bus_name = request_dict.at("name").AsString();
            json::Node response = handler.GetBusInfo(bus_name, request_id);
            builder.Value(response);
        } else if (type == "Stop") {
            if (!request_dict.count("name")) {
                builder.Value(json::Builder()
                    .StartDict()
                    .Key("request_id").Value(request_id)
                    .Key("error_message").Value("not found")
                    .EndDict()
                    .Build());
                continue;
            }
            std::string stop_name = request_dict.at("name").AsString();
            json::Node response = handler.GetStopInfo(stop_name, request_id);
            builder.Value(response);
        } else if (type == "Map") {
            map_output.str("");
            renderer::RenderMap(catalogue, render_settings, map_output);
            builder.Value(json::Builder()
                .StartDict()
                .Key("request_id").Value(request_id)
                .Key("map").Value(map_output.str())
                .EndDict()
                .Build());
        } else {
            builder.Value(json::Builder()
                .StartDict()
                .Key("request_id").Value(request_id)
                .Key("error_message").Value("unknown request type")
                .EndDict()
                .Build());
        }
    }
    
    builder.EndArray();
    return json::Document(builder.Build());
}

} // namespace json_reader
