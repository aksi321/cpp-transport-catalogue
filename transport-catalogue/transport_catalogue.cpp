#include "transport_catalogue.h"

namespace catalogue{
void TransportCatalogue::AddStop(std::string id ,double lat, double lag){
    stops_.push_back({id, lat, lag});
    stops_ptr_[stops_.back().name] = &stops_.back();

}

void TransportCatalogue::AddBus(std::string id, std::vector<std::string_view>stops){
    std::vector<std::string> st;   
    for (auto& stop:stops){
        if (stops_ptr_.count(stop)){
            st.push_back(std::string(stop));     
        }    
    }
    buses_.push_back({id, std::move(st)});
    bus_ptr_[buses_.back().name] = &buses_.back();
    LinkBusToStops(buses_.back().name);
}

void TransportCatalogue::LinkBusToStops(std::string_view id){
    const auto& bus = bus_ptr_.at(id);

    for (const auto& stop : bus->stops) {
        bases_for_stops_[stop].insert(id);
    } 
}

BusCounted TransportCatalogue::CountStation(std::string_view id ) const{

    int count = 0;
    size_t amount = bus_ptr_.at(id)->stops.size();
    size_t doubl =0 ;
    std::map<std::string, size_t> repeat;

    Coordinates from;
    Coordinates to;
    double distance = 0;

    for (auto s : bus_ptr_.at(id)->stops ){
        ++repeat[s];
        to={stops_ptr_.at(s)->coordinates.lat, stops_ptr_.at(s)->coordinates.lng};    
        if (!count){
            from={stops_ptr_.at(s)->coordinates.lat, stops_ptr_.at(s)->coordinates.lng};
        }
        if (repeat[s] >1 ){
             doubl++;
        }
        distance+=ComputeDistance(to, from);
        from = to;
        count++;
    }

    return {amount, amount-doubl, distance};
}

BusCounted TransportCatalogue::GetBusStatistics(std::string_view id) const {
    BusCounted ret;
    if (!bus_ptr_.count(id)){ 
        return ret;
    }
    ret = CountStation(id);
    return ret;
}

BusesForStop TransportCatalogue::GetBusesForStop(std::string_view id ) const{
    BusesForStop result;

    if (!bases_for_stops_.count(std::string(id))){
        if (stops_ptr_.count(std::string(id))){
            result.request_status="empty";
        } else {
            result.request_status = "not found";
        }     
    } else {
        std::set<std::string_view> collect = bases_for_stops_.at(std::string(id));
             result.request_status = "found";
             result.buses = std::move(collect);
    }
    return result;
}

const Stop* TransportCatalogue::GetStop(std::string_view name) const {
    auto it = stops_ptr_.find(name);
    if (it != stops_ptr_.end()) {
        return it->second;
    }
    return nullptr;  
}

const Bus* TransportCatalogue::GetBus(std::string_view name) const {
    auto it = bus_ptr_.find(name);
    if (it != bus_ptr_.end()) {
        return it->second;
    }
    return nullptr; 
}

}
