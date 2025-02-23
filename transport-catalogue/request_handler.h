#pragma once
#include "transport_catalogue.h"
#include <string>

namespace request_handler {

class RequestHandler {
public:
    explicit RequestHandler(const catalogue::TransportCatalogue& catalogue);
    
    std::string GetBusInfo(const std::string& bus_name) const;
    
    std::string GetStopInfo(const std::string& stop_name) const;
    
private:
    const catalogue::TransportCatalogue& catalogue_;
};

}  // namespace request_handler


