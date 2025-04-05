#include "transport_router.h"
#include "graph.h"
#include "router.h"
#include "json_reader.h"
#include <unordered_map>
#include <cmath>
#include <algorithm>
using namespace std;

void TransportRouter::AddStopVertex(const string& stop_name, int wait_time) {
    StopVertex sv;
    sv.wait = current_stop_index_++;
    sv.bus = current_stop_index_++;
    stop_to_vertex_[stop_name] = sv;
    graph::Edge<double> wait_edge {sv.wait, sv.bus, static_cast<double>(wait_time)};
    graph_.AddEdge(wait_edge);
    edge_info_.push_back({EdgeType::Wait, stop_name, static_cast<double>(wait_time), "", 0});
}

void TransportRouter::BuildGraph(const std::vector<const domain::Stop*>& stops,
                                 const std::vector<Bus>& buses,
                                 const RoutingSettings& settings,
                                 const catalogue::TransportCatalogue& catalogue) {
    current_stop_index_ = 0;
    graph_ = graph::DirectedWeightedGraph<double>(stops.size() * 2);

    for (const auto& stop : stops) {
        AddStopVertex(stop->name, settings.bus_wait_time);
    }

    std::unordered_map<std::string, const domain::Stop*> stop_by_name;
    for (const auto& stop : stops) {
        stop_by_name[stop->name] = stop;
    }

    for (const auto& bus : buses) {
        int n = static_cast<int>(bus.stops.size());
        if (n < 2) continue;

        for (int i = 0; i < n; ++i) {
            int total_distance = 0;
            for (int j = i + 1; j < n; ++j) {
                const auto* stop_from = stop_by_name.at(bus.stops[j - 1]);
                const auto* stop_to = stop_by_name.at(bus.stops[j]);

                total_distance += catalogue.GetDistance(stop_from, stop_to);

                double travel_time = total_distance / 1000.0 * 60.0 / settings.bus_velocity;

                graph::Edge<double> edge{
                    stop_to_vertex_[bus.stops[i]].bus,
                    stop_to_vertex_[bus.stops[j]].wait,
                    travel_time
                };
                graph_.AddEdge(edge);

                edge_info_.push_back({
                    EdgeType::Bus,
                    bus.stops[i],  
                    travel_time,
                    bus.name,
                    j - i
                });
            }
        }

        if (!bus.is_roundtrip) {
            for (int i = n - 1; i >= 0; --i) {
                int total_distance = 0;
                for (int j = i - 1; j >= 0; --j) {
                    const auto* stop_from = stop_by_name.at(bus.stops[j + 1]);
                    const auto* stop_to = stop_by_name.at(bus.stops[j]);

                    total_distance += catalogue.GetDistance(stop_from, stop_to);

                    double travel_time = total_distance / 1000.0 * 60.0 / settings.bus_velocity;

                    graph::Edge<double> edge{
                        stop_to_vertex_[bus.stops[i]].bus,
                        stop_to_vertex_[bus.stops[j]].wait,
                        travel_time
                    };
                    graph_.AddEdge(edge);

                    edge_info_.push_back({
                        EdgeType::Bus,
                        bus.stops[i],  
                        travel_time,
                        bus.name,
                        i - j
                    });
                }
            }
        }
    }

    router_ = std::make_unique<graph::Router<double>>(graph_);
}



std::optional<RouteResult> TransportRouter::GetRoute(const std::string& from, const std::string& to) const {
    if (from == to) return RouteResult{0.0, {}};
    if (!stop_to_vertex_.count(from) || !stop_to_vertex_.count(to)) return std::nullopt;

    size_t start = stop_to_vertex_.at(from).wait;
    size_t finish = stop_to_vertex_.at(to).wait;
    auto route_info_opt = router_->BuildRoute(start, finish);
    if (!route_info_opt) return std::nullopt;

    const auto& route_info = *route_info_opt;
    RouteResult result;
    result.total_time = route_info.weight;

    for (auto edge_id : route_info.edges) {
        const auto& info = edge_info_[edge_id];
        if (info.type == EdgeType::Wait) {
            result.items.push_back({
                "Wait",
                info.stop_name,
                info.time,
                "",
                0
            });
        } else if (info.type == EdgeType::Bus) {
            result.items.push_back({
                "Bus",
                info.stop_name,  
                info.time,
                info.bus,
                info.span_count
            });
        }
    }

    return result;
}

