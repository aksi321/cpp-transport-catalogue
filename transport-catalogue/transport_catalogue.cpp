#include "transport_catalogue.h"
#include <set>
#include <string>
#include <stdexcept>
#include <unordered_set>

namespace catalogue {

    void TransportCatalogue::AddStop(const std::string& name, double lat, double lng) {
        stops_.push_back({name, {lat, lng}});
        stops_ptr_[stops_.back().name] = &stops_.back();
    }

    void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string_view>& stops, bool is_roundtrip) {
        std::vector<std::string> st;
        for (auto sv : stops) {
            if (!stops_ptr_.count(sv)) {
                return;
            }
            st.push_back(std::string(sv));
        }
        buses_.push_back({name, std::move(st), is_roundtrip});
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
    
    const std::unordered_map<std::string_view, Bus*>& TransportCatalogue::GetAllBuses() const {
        return bus_ptr_;
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
    size_t n = bus->stops.size();
    if (n == 0)
        return result;

    std::unordered_set<std::string_view> uniq;
    for (const auto& stop : bus->stops) {
        uniq.insert(stop);
    }
    result.unique = uniq.size();

    double measured = 0.0;
    double geo = 0.0;

    if (bus->is_roundtrip) {
        bool is_closed = (bus->stops.front() == bus->stops.back());

        result.amount = is_closed ? n : n + 1;
        for (size_t i = 1; i < n; ++i) {
            const std::string& prev = bus->stops[i - 1];
            const std::string& cur  = bus->stops[i];
            const Stop* stop_prev = GetStop(prev);
            const Stop* stop_cur  = GetStop(cur);
            if (!stop_prev || !stop_cur)
                continue;
            int d = GetDistance(stop_prev, stop_cur);
            if (d == 0) {
                d = static_cast<int>(geo::ComputeDistance(stop_prev->coordinates, stop_cur->coordinates) + 0.5);
            }
            measured += d;
            geo += geo::ComputeDistance(stop_prev->coordinates, stop_cur->coordinates);
        }
        if (!is_closed) { 
            const Stop* stop_first = GetStop(bus->stops.front());
            const Stop* stop_last  = GetStop(bus->stops.back());
            if (stop_first && stop_last) {
                int d = GetDistance(stop_last, stop_first);
                if (d == 0) {
                    d = static_cast<int>(geo::ComputeDistance(stop_last->coordinates, stop_first->coordinates) + 0.5);
                }
                measured += d;
                geo += geo::ComputeDistance(stop_last->coordinates, stop_first->coordinates);
            }
        }
    } else {

        result.amount = n * 2 - 1;
        double measured_fwd = 0.0, geo_fwd = 0.0;
        for (size_t i = 1; i < n; ++i) {
            const std::string& prev = bus->stops[i - 1];
            const std::string& cur  = bus->stops[i];
            const Stop* stop_prev = GetStop(prev);
            const Stop* stop_cur  = GetStop(cur);
            if (!stop_prev || !stop_cur)
                continue;
            int d = GetDistance(stop_prev, stop_cur);
            if (d == 0)
                d = static_cast<int>(geo::ComputeDistance(stop_prev->coordinates, stop_cur->coordinates) + 0.5);
            measured_fwd += d;
            geo_fwd += geo::ComputeDistance(stop_prev->coordinates, stop_cur->coordinates);
        }
        double measured_rev = 0.0, geo_rev = 0.0;
        for (size_t i = n - 1; i > 0; --i) {
            const std::string& prev = bus->stops[i];
            const std::string& cur  = bus->stops[i - 1];
            const Stop* stop_prev = GetStop(prev);
            const Stop* stop_cur  = GetStop(cur);
            if (!stop_prev || !stop_cur)
                continue;
            int d = GetDistance(stop_prev, stop_cur);
            if (d == 0)
                d = static_cast<int>(geo::ComputeDistance(stop_prev->coordinates, stop_cur->coordinates) + 0.5);
            measured_rev += d;
            geo_rev += geo::ComputeDistance(stop_prev->coordinates, stop_cur->coordinates);
        }
        measured = measured_fwd + measured_rev;
        geo = geo_fwd + geo_rev;
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
        auto it = buses_for_stop_.find(stop_name);
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
            buses_for_stop_[stop].insert(bus->name);
        }
    }


} // namespace catalogue
