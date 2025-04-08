#include "transport_router.h"
#include <unordered_map>
#include <cmath>
#include <algorithm>

using namespace std;

// Введены kMetersPerKm и kMinutesPerHour, именованные константы 
const double kMetersPerKm = 1000.0;
const double kMinutesPerHour = 60.0;

TransportRouter::TransportRouter(const catalogue::TransportCatalogue& catalogue, RoutingSettings settings)
    : catalogue_(catalogue), settings_(settings) {}

void TransportRouter::AddStopVertex(const string& stop_name, int wait_time) {
    StopVertex sv;
    sv.wait = current_stop_index_++;
    sv.bus = current_stop_index_++;
    stop_to_vertex_[stop_name] = sv;
    graph_.AddEdge({sv.wait, sv.bus, static_cast<double>(wait_time)});
    edge_info_.push_back({EdgeType::Wait, stop_name, static_cast<double>(wait_time), "", 0});
}

void TransportRouter::FillGraphWithStops() {
    for (const auto& [name, stop_ptr] : catalogue_.GetAllStops()) {
        AddStopVertex(std::string(name), settings_.bus_wait_time);
    }
}

void TransportRouter::AddBusEdges(const TransportRouter::Bus& bus, bool reverse) {
    int n = static_cast<int>(bus.stops.size());
    for (int i = 0; i < n; ++i) {
        int total_distance = 0;
        for (int j = i + 1; j < n; ++j) {
            const auto* from = catalogue_.GetStop(bus.stops[j - 1]);
            const auto* to = catalogue_.GetStop(bus.stops[j]);
            total_distance += catalogue_.GetDistance(from, to);
            double time = total_distance / kMetersPerKm * kMinutesPerHour / settings_.bus_velocity;

            graph_.AddEdge({stop_to_vertex_.at(bus.stops[i]).bus,
                            stop_to_vertex_.at(bus.stops[j]).wait,
                            time});
            edge_info_.push_back({EdgeType::Bus, bus.stops[i], time, bus.name, j - i});
        }
    }

    if (reverse) {
        for (int i = n - 1; i >= 0; --i) {
            int total_distance = 0;
            for (int j = i - 1; j >= 0; --j) {
                const auto* from = catalogue_.GetStop(bus.stops[j + 1]);
                const auto* to = catalogue_.GetStop(bus.stops[j]);
                total_distance += catalogue_.GetDistance(from, to);
                double time = total_distance / kMetersPerKm * kMinutesPerHour / settings_.bus_velocity;

                graph_.AddEdge({stop_to_vertex_.at(bus.stops[i]).bus,
                                stop_to_vertex_.at(bus.stops[j]).wait,
                                time});
                edge_info_.push_back({EdgeType::Bus, bus.stops[i], time, bus.name, i - j});
            }
        }
    }
}

void TransportRouter::FillGraphWithBuses() {
    for (const auto& [name, bus_ptr] : catalogue_.GetAllBuses()) {
        const auto& bus = *bus_ptr;
        if (bus.stops.size() < 2) {
            continue;
        }
        AddBusEdges({std::string(bus.name), {bus.stops.begin(), bus.stops.end()}, bus.is_roundtrip}, !bus.is_roundtrip);
    }
}

void TransportRouter::BuildGraph() { // Добавлены вспомогательные методы: FillGraphWithStops, FillGraphWithBuses, AddBusEdges
    current_stop_index_ = 0;
    graph_ = graph::DirectedWeightedGraph<double>(catalogue_.GetAllStops().size() * 2);
    FillGraphWithStops();
    FillGraphWithBuses();
    router_ = make_unique<graph::Router<double>>(graph_);
}

optional<RouteResult> TransportRouter::GetRoute(std::string_view from, std::string_view to) const {
    if (from == to) return RouteResult{0.0, {}};
    if (!stop_to_vertex_.count(std::string(from)) || !stop_to_vertex_.count(std::string(to))) return nullopt;

    size_t start = stop_to_vertex_.at(std::string(from)).wait;
    size_t finish = stop_to_vertex_.at(std::string(to)).wait;
    auto route_info_opt = router_->BuildRoute(start, finish);
    if (!route_info_opt) return nullopt;

    const auto& route_info = *route_info_opt;
    RouteResult result;
    result.total_time = route_info.weight;

    for (auto edge_id : route_info.edges) {
        const auto& info = edge_info_[edge_id];
        if (info.type == EdgeType::Wait) {
            result.items.push_back({"Wait", info.stop_name, info.time, "", 0});
        } else {
            result.items.push_back({"Bus", info.stop_name, info.time, info.bus, info.span_count});
        }
    }

    return result;
}
