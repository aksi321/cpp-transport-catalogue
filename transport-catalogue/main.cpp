#include <iostream>
#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

int main() {
    auto doc = json::Load(std::cin);
    
    catalogue::TransportCatalogue catalogue;
    auto input_data = json_reader::ParseInputData(doc);
    json_reader::FillTransportCatalogue(input_data, catalogue);
    
    auto answer = json_reader::ProcessRequests(doc, catalogue);
    json::Print(answer, std::cout);
    
    return 0;
}
