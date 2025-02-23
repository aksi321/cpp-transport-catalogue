#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace json_reader {

    void ReadTransportCatalogue(const json::Document& doc, catalogue::TransportCatalogue& catalogue);

    json::Document ProcessRequests(const json::Document& doc, const catalogue::TransportCatalogue& catalogue);
    
    void ReadTransportCatalogue(const json::Document& doc, catalogue::TransportCatalogue& catalogue);
    
    renderer::RenderSettings ParseRenderSettings(const json::Document& doc); 
}  // namespace json_reader

