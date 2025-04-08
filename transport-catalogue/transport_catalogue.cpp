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
        for (std::string_view stop : stops) {
            if (!stops_ptr_.count(stop)) {
                return;
            }
            st.push_back(std::string(stop));
        }
        buses_.push_back({name, std::move(st), is_roundtrip});
        bus_ptr_[buses_.back().name] = &buses_.back();
        LinkBusToStops(buses_.back().name);
    }

    void TransportCatalogue::SetDistance(std::string_view stop_from, std::string_view stop_to, int distance) {
        const Stop* from = GetStop(stop_from);
        const Stop* to = GetStop(stop_to);
        if (from && to) {
            distances_[{from, to}] = distance;
        }
    }

    const std::unordered_map<std::string_view, Bus*>& TransportCatalogue::GetAllBuses() const {
        return bus_ptr_;
    }

    const std::unordered_map<std::string_view, Stop*>& TransportCatalogue::GetAllStops() const {
        return stops_ptr_;
    }

    int TransportCatalogue::GetDistance(const Stop* stop_from, const Stop* stop_to) const {
        auto it = distances_.find({stop_from, stop_to});
        if (it != distances_.end()) return it->second;
        it = distances_.find({stop_to, stop_from});
        if (it != distances_.end()) return it->second;
        return 0;
    }

    BusCounted TransportCatalogue::CountStation(std::string_view bus_name) const {
        BusCounted result;
        auto it = bus_ptr_.find(bus_name);
        if (it == bus_ptr_.end()) return result;

        const Bus* bus = it->second;
        size_t n = bus->stops.size();
        if (n == 0) return result;

        std::unordered_set<std::string_view> uniq_stops;
        for (const auto& stop : bus->stops) {
            uniq_stops.insert(stop);
        }
        result.unique = uniq_stops.size();

        double measured = 0.0;
        double geo = 0.0;

        if (bus->is_roundtrip) {
            bool is_closed = (bus->stops.front() == bus->stops.back());
            result.amount = is_closed ? n : n + 1;
            for (size_t i = 1; i < n; ++i) {
                const Stop* from = GetStop(bus->stops[i - 1]);
                const Stop* to = GetStop(bus->stops[i]);
                if (!from || !to) continue;
                int d = GetDistance(from, to);
                if (d == 0) d = static_cast<int>(geo::ComputeDistance(from->coordinates, to->coordinates) + 0.5);
                measured += d;
                geo += geo::ComputeDistance(from->coordinates, to->coordinates);
            }
            if (!is_closed) {
                const Stop* from = GetStop(bus->stops.back());
                const Stop* to = GetStop(bus->stops.front());
                if (from && to) {
                    int d = GetDistance(from, to);
                    if (d == 0) d = static_cast<int>(geo::ComputeDistance(from->coordinates, to->coordinates) + 0.5);
                    measured += d;
                    geo += geo::ComputeDistance(from->coordinates, to->coordinates);
                }
            }
        } else {
            result.amount = n * 2 - 1;
            for (size_t i = 1; i < n; ++i) {
                const Stop* from = GetStop(bus->stops[i - 1]);
                const Stop* to = GetStop(bus->stops[i]);
                if (!from || !to) continue;
                int d = GetDistance(from, to);
                if (d == 0) d = static_cast<int>(geo::ComputeDistance(from->coordinates, to->coordinates) + 0.5);
                measured += d;
                geo += geo::ComputeDistance(from->coordinates, to->coordinates);
            }
            for (size_t i = n - 1; i > 0; --i) {
                const Stop* from = GetStop(bus->stops[i]);
                const Stop* to = GetStop(bus->stops[i - 1]);
                if (!from || !to) continue;
                int d = GetDistance(from, to);
                if (d == 0) d = static_cast<int>(geo::ComputeDistance(from->coordinates, to->coordinates) + 0.5);
                measured += d;
                geo += geo::ComputeDistance(from->coordinates, to->coordinates);
            }
        }

        result.length = measured;
        result.geo_length = geo;
        return result;
    }

    BusCounted TransportCatalogue::GetBusStatistics(std::string_view bus_name) const {
        return CountStation(bus_name);
    }

    const std::set<std::string_view>& TransportCatalogue::GetBusesForStop(std::string_view stop_name) const {
        auto it = buses_for_stop_.find(stop_name);
        if (it != buses_for_stop_.end()) {
            return it->second;
        } else {
            static const std::set<std::string_view> empty_set;
            return empty_set;
        }
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
        if (!bus) return;
        for (const auto& stop : bus->stops) {
            buses_for_stop_[stop].insert(bus->name);
        }
    }

} // namespace catalogue
