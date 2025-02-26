#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <vector>
#include <string>
#include <map>

namespace json_reader {

// Структура для хранения данных об остановке, полученных из JSON.
struct StopData {
    std::string name;
    double latitude;
    double longitude;
    std::map<std::string, int> road_distances;
};

// Структура для хранения данных о маршруте, полученных из JSON.
struct BusData {
    std::string name;
    std::vector<std::string> stops;
    bool is_roundtrip = false;
};

// Структура, инкапсулирующая весь входной набор данных из JSON.
struct InputData {
    std::vector<StopData> stops;
    std::vector<BusData> buses;
    renderer::RenderSettings render_settings;
};

// Преобразует входной JSON в структуру InputData
InputData ParseInputData(const json::Document& doc);
// Принимает уже подготовленный объект InputData и заполняет транспортный справочник.
void FillTransportCatalogue(const InputData& input_data, catalogue::TransportCatalogue& catalogue);

json::Document ProcessRequests(const json::Document& doc, const catalogue::TransportCatalogue& catalogue);

renderer::RenderSettings ParseRenderSettings(const json::Document& doc);

} // namespace json_reader
