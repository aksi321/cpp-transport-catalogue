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
    bus_ptr_[buses_.back().first] = &buses_.back().second;
    BusForStop(buses_.back().first);
}

void TransportCatalogue::BusForStop(std::string_view id){
     for (auto stop : *bus_ptr_.at(id)){
        bases_for_stops_[stop].insert(id);
     }
}

BusCounted TransportCatalogue::CountStation(std::string_view id ) const{

    int count = 0;
    size_t amount = bus_ptr_.at(id)->size();
    size_t doubl =0 ;
    std::map<std::string, size_t> repeat;

    Coordinates from;
    Coordinates to;
    double distance = 0;

    for (auto s : *bus_ptr_.at(id) ){
        ++repeat[s];
        to={stops_ptr_.at(s)->latitude, stops_ptr_.at(s)->longitude};    
        if (!count){
            from={stops_ptr_.at(s)->latitude, stops_ptr_.at(s)->longitude};
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

BusCounted TransportCatalogue::ReturnBus(std::string_view id) const {
    BusCounted ret;
    if (!bus_ptr_.count(id)){ 
        return ret;
    }
    ret = CountStation(id);
    return ret;
}

AllBussForStop  TransportCatalogue::ReturnStop(std::string_view id ) const{
    AllBussForStop rez;

    if (!bases_for_stops_.count(std::string(id))){
        if (stops_ptr_.count(std::string(id))){
            rez.reqest="emty";
        } else {
            rez.reqest = "not answer";
        }     
    } else {
        std::set<std::string_view> collect = bases_for_stops_.at(std::string(id));
             rez.reqest = "full";
             rez.collect = std::move(collect);
    }
    return rez;
}
}
