#include "transport_catalogue.h"
#include "geo.h" 

#include <set>
#include <string>
#include <stdexcept>

namespace catalogue {

    void TransportCatalogue::AddStop(const std::string& name, double lat, double lng) {
        stops_.push_back({name, {lat, lng}});
        stops_ptr_[stops_.back().name] = &stops_.back();
    }

    void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string_view>& stops) {
        std::vector<std::string> st;
        for (auto sv : stops) {
            if (stops_ptr_.count(sv)) {
                st.push_back(std::string(sv));
            }
        }
        buses_.push_back({name, std::move(st)});
        bus_ptr_[buses_.back().name] = &buses_.back();
        LinkBusToStops(buses_.back().name);
    }

    void TransportCatalogue::AddDistance(const std::string& stop_name, const std::string& neighbor, int distance) {
        const Stop* from = GetStop(stop_name);
        const Stop* to = GetStop(neighbor);
        if (from && to) {
            distances_[{from, to}] = distance;
        }
    }

    int TransportCatalogue::GetDistance(const Stop* a, const Stop* b) const {
        auto it = distances_.find({a, b});
        if (it != distances_.end())
            return it->second;
        it = distances_.find({b, a});
        if (it != distances_.end())
            return it->second;
        return 0;
    }

    BusCounted TransportCatalogue::CountStation(std::string_view bus_name) const {
        BusCounted result;
        auto it = bus_ptr_.find(bus_name);
        if (it == bus_ptr_.end())
            return result;
        const Bus* bus = it->second;
        result.amount = bus->stops.size();
        std::set<std::string> uniq(bus->stops.begin(), bus->stops.end());
        result.unique = uniq.size();
        double measured = 0.0;
        double geo = 0.0;
        if (bus->stops.empty())
            return result;
        for (size_t i = 1; i < bus->stops.size(); ++i) {
            const std::string& prev = bus->stops[i - 1];
            const std::string& cur  = bus->stops[i];
            const Stop* stop_prev = GetStop(prev);
            const Stop* stop_cur  = GetStop(cur);
            if (!stop_prev || !stop_cur)
                continue;
            int d = GetDistance(stop_prev, stop_cur);
            if (d == 0) {
                d = static_cast<int>(ComputeDistance(stop_prev->coordinates, stop_cur->coordinates) + 0.5);
            }
            measured += d;
            geo += ComputeDistance(stop_prev->coordinates, stop_cur->coordinates);
        }
        result.length = measured;
        result.geo_length = geo;
        return result;
    }

    BusCounted TransportCatalogue::GetBusStatistics(std::string_view bus_name) const {
        return CountStation(bus_name);
    }

    BusesForStop TransportCatalogue::GetBusesForStop(std::string_view stop_name) const {
        BusesForStop result;
        auto it = buses_for_stop_.find(std::string(stop_name));
        if (it == buses_for_stop_.end()) {
            if (stops_ptr_.count(stop_name))
                result.request_status = "empty";
            else
                result.request_status = "not found";
        } else {
            result.request_status = "found";
            result.buses = it->second;
        }
        return result;
    }

    const Stop* TransportCatalogue::GetStop(std::string_view name) const {
        auto it = stops_ptr_.find(name);
        return (it != stops_ptr_.end()) ? it->second : nullptr;
    }

    const Bus* TransportCatalogue::GetBus(std::string_view name) const {
        auto it = bus_ptr_.find(name);
        return (it != bus_ptr_.end()) ? it->second : nullptr;
    }

    void TransportCatalogue::LinkBusToStops(std::string_view bus_name) {
        const Bus* bus = GetBus(bus_name);
        if (!bus)
            return;
        for (const auto& stop : bus->stops) {
            buses_for_stop_[stop].insert(std::string_view(bus->name));
        }
    }
} // namespace catalogue
