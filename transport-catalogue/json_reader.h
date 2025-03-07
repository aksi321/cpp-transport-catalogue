#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <vector>
#include <string>
#include <map>

namespace json_reader {

struct StopData {
    std::string name;
    double latitude;
    double longitude;
    std::map<std::string, int> road_distances;
};

struct BusData {
    std::string name;
    std::vector<std::string> stops;
    bool is_roundtrip = false;
};

struct InputData {
    std::vector<StopData> stops;
    std::vector<BusData> buses;
    renderer::RenderSettings render_settings;
};

InputData ParseInputData(const json::Document& doc);
void FillTransportCatalogue(const InputData& input_data, catalogue::TransportCatalogue& catalogue);

json::Document ProcessRequests(const json::Document& doc, const catalogue::TransportCatalogue& catalogue, const renderer::RenderSettings& render_settings);

renderer::RenderSettings ParseRenderSettings(const json::Document& doc);

} // namespace json_reader
