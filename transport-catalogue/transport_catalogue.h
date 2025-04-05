#pragma once

#include <deque>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <map>
#include "geo.cpp"
#include <iostream>
#include <functional>
#include <utility>
#include "domain.h"

using domain::Stop;
using domain::Bus;

namespace catalogue {
    
    struct BusesForStop {
        std::string request_status;
        std::set<std::string_view> buses;
    };

    struct BusCounted {
        size_t amount = 0;
        size_t unique = 0;
        double length = 0;     
        double geo_length = 0; 
        bool IsEmpty() const {
            return (amount == 0 && unique == 0 && length == 0 && geo_length == 0);
        }
    };

    struct StopPairHasher {
        size_t operator()(const std::pair<const Stop*, const Stop*>& p) const {
            auto h1 = std::hash<const void*>{}(p.first);
            auto h2 = std::hash<const void*>{}(p.second);
            return h1 ^ (h2 << 1);
        }
    };

    class TransportCatalogue {
    public:
        void AddStop(const std::string& name, double lat, double lng);
        void AddBus(const std::string& name, const std::vector<std::string_view>& stops, bool is_roundtrip);
        void AddDistance(const std::string& stop_name, const std::string& neighbor, int distance);
        const std::unordered_map<std::string_view, Bus*>& GetAllBuses() const;
        const std::unordered_map<std::string_view, Stop*>& GetAllStops() const;
        int GetDistance(const Stop* a, const Stop* b) const;
        BusCounted GetBusStatistics(std::string_view bus_name) const;
        BusesForStop GetBusesForStop(std::string_view stop_name) const;
        const Stop* GetStop(std::string_view name) const;
        const Bus* GetBus(std::string_view name) const;
    private:
        BusCounted CountStation(std::string_view bus_name) const;
        void LinkBusToStops(std::string_view bus_name);

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Stop*> stops_ptr_;
        std::unordered_map<std::string_view, Bus*> bus_ptr_;
        std::unordered_map<std::string_view, std::set<std::string_view>> buses_for_stop_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPairHasher> distances_;
    };
} // namespace catalogue
