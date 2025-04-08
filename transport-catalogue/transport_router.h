#pragma once
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <memory>

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};

enum class EdgeType {
    Wait,
    Bus
};

struct RouteEdgeInfo {
    EdgeType type;
    std::string stop_name;
    double time;
    std::string bus;
    int span_count;
};

struct StopVertex {
    size_t wait;
    size_t bus;
};

struct RouteItem {
    std::string type;
    std::string stop_name;
    double time;
    std::string bus;
    int span_count;
};

struct RouteResult {
    double total_time;
    std::vector<RouteItem> items;
};

class TransportRouter {
public:
    struct Bus {
        std::string name;
        std::vector<std::string> stops;
        bool is_roundtrip;
    };

    // Добавлен конструктор с ссылкой на каталог и настройки
    TransportRouter(const catalogue::TransportCatalogue& catalogue, RoutingSettings settings);
    // BuildGraph() стал без параметров, данные берутся из catalogue_ и settings_
    void BuildGraph();
    // Параметры from, to стали std::string_view
    std::optional<RouteResult> GetRoute(std::string_view from, std::string_view to) const;

private:
    void AddStopVertex(const std::string& stop_name, int wait_time);
    void FillGraphWithStops();
    void FillGraphWithBuses();
    void AddBusEdges(const Bus& bus, bool reverse);

    const catalogue::TransportCatalogue& catalogue_;
    RoutingSettings settings_;
    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    std::unordered_map<std::string, StopVertex> stop_to_vertex_;
    std::vector<RouteEdgeInfo> edge_info_;
    size_t current_stop_index_ = 0;
};
