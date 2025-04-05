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
    struct Stop {
        std::string name;
        std::unordered_map<std::string, int> road_distances; 
    };

    struct Bus {
        std::string name;
        std::vector<std::string> stops; 
        bool is_roundtrip;              
    };

    void BuildGraph(const std::vector<const domain::Stop*>& stops,
                const std::vector<Bus>& buses,
                const RoutingSettings& settings,
                const catalogue::TransportCatalogue& catalogue);

    std::optional<RouteResult> GetRoute(const std::string& from, const std::string& to) const;

private:
    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    std::unordered_map<std::string, StopVertex> stop_to_vertex_;
    std::vector<RouteEdgeInfo> edge_info_;
    size_t current_stop_index_ = 0;
    void AddStopVertex(const std::string& stop_name, int wait_time);
};
