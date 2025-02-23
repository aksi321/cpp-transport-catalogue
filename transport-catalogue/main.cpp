#include <iostream>
#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

int main() {
    auto doc = json::Load(std::cin);
    
    catalogue::TransportCatalogue catalogue;
    json_reader::ReadTransportCatalogue(doc, catalogue);  

    auto answer = json_reader::ProcessRequests(doc, catalogue);
    json::Print(answer, std::cout);
    
    return 0;
}
