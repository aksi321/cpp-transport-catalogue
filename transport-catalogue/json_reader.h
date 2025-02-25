#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace json_reader {

// Оставленна только одна функция. Заполняет транспортный справочник данными из входного JSON-документа.
void ReadTransportCatalogue(const json::Document& doc, catalogue::TransportCatalogue& catalogue);

json::Document ProcessRequests(const json::Document& doc, const catalogue::TransportCatalogue& catalogue);

renderer::RenderSettings ParseRenderSettings(const json::Document& doc);

}  // namespace json_reader
