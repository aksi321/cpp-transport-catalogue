#pragma once
#include "transport_catalogue.h"
#include "json.h"

namespace request_handler {

class RequestHandler {
public:
    explicit RequestHandler(const catalogue::TransportCatalogue& catalogue);
    
    json::Node GetBusInfo(const std::string& bus_name, int request_id) const;
    json::Node GetStopInfo(const std::string& stop_name, int request_id) const;
    
private:
    const catalogue::TransportCatalogue& catalogue_;
};

}  // namespace request_handler
