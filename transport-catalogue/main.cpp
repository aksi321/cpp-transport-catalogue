#include "json_builder.h"
#include "json_reader.h"
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <iostream>

int main() {
    auto doc = json::Load(std::cin);
    
    catalogue::TransportCatalogue catalogue;
    auto input_data = json_reader::ParseInputData(doc);
    json_reader::FillTransportCatalogue(input_data, catalogue);
    renderer::RenderSettings render_settings = json_reader::ParseRenderSettings(doc);
    auto answer = json_reader::ProcessRequests(doc, catalogue, render_settings);
    json::Print(answer, std::cout);
    
    return 0;
}
