#include "map_renderer.h"
#include "transport_catalogue.h"
#include <algorithm>
#include <set>
#include <sstream>
#include <unordered_set>

namespace renderer {

RenderSettings GetDefaultRenderSettings() {
    RenderSettings settings;
    return settings;
}

std::map<std::string, std::string> AssignRouteColors(
    const std::vector<domain::Bus*>& buses,
    const std::vector<std::string>& palette) {
    
    std::vector<domain::Bus*> sorted_buses = buses;
    std::sort(sorted_buses.begin(), sorted_buses.end(), [](const domain::Bus* a, const domain::Bus* b) {
        return a->name < b->name;
    });
    
    std::map<std::string, std::string> route_colors;
    size_t color_index = 0;
    for (const auto* bus : sorted_buses) {
        if (!bus->stops.empty()) {
            route_colors[bus->name] = palette[color_index % palette.size()];
            ++color_index;
        }
    }
    return route_colors;
}

void RenderRouteLines(const catalogue::TransportCatalogue& catalogue,
                      const RenderSettings& settings,
                      svg::Document& doc,
                      const SphereProjector& projector,
                      const std::map<std::string, std::string>& route_colors) {
    const auto& buses = catalogue.GetAllBuses();
    std::vector<const domain::Bus*> sorted_buses;
    for (const auto& [name, bus] : buses) {
        sorted_buses.push_back(bus);
    }
    std::sort(sorted_buses.begin(), sorted_buses.end(), [](const domain::Bus* lhs, const domain::Bus* rhs) {
        return lhs->name < rhs->name;
    });

    for (const auto* bus : sorted_buses) {
        if (bus->stops.size() < 2)
            continue;
        std::vector<svg::Point> points;
        
        for (const auto& stop_name : bus->stops) {
            const auto* stop = catalogue.GetStop(stop_name);
            if (stop)
                points.push_back(projector(stop->coordinates));
        }
        if (bus->is_roundtrip) {
            if (bus->stops.front() != bus->stops.back())
                points.push_back(points.front());
        } else {
            
            for (size_t i = bus->stops.size() - 1; i > 0; --i) {
                const auto* stop = catalogue.GetStop(bus->stops[i - 1]);
                if (stop)
                    points.push_back(projector(stop->coordinates));
            }
        }
        svg::Polyline polyline;
        for (const auto& pt : points)
            polyline.AddPoint(pt);
        polyline.SetFillColor(svg::NoneColor)
                .SetStrokeColor(route_colors.at(bus->name))
                .SetStrokeWidth(settings.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        doc.AddPtr(std::make_unique<svg::Polyline>(polyline));
    }
}

void RenderRouteLabels(const catalogue::TransportCatalogue& catalogue,
                         const RenderSettings& settings,
                         svg::Document& doc,
                         const SphereProjector& projector,
                         const std::map<std::string, std::string>& route_colors) {
    const auto& buses = catalogue.GetAllBuses();
    std::vector<const domain::Bus*> sorted_buses;
    for (const auto& [name, bus] : buses) {
        sorted_buses.push_back(bus);
    }
    std::sort(sorted_buses.begin(), sorted_buses.end(), [](const domain::Bus* lhs, const domain::Bus* rhs) {
        return lhs->name < rhs->name;
    });
    
    for (const auto* bus : sorted_buses) {
        if (bus->stops.empty())
            continue;
        
        std::vector<svg::Point> endpoints;
        const auto* first_stop = catalogue.GetStop(bus->stops.front());
        if (first_stop)
            endpoints.push_back(projector(first_stop->coordinates));
        if (!bus->is_roundtrip && bus->stops.front() != bus->stops.back()) {
            const auto* last_stop = catalogue.GetStop(bus->stops.back());
            if (last_stop)
                endpoints.push_back(projector(last_stop->coordinates));
        }
        
        for (const auto& pt : endpoints) {
            
            svg::Text underlayer;
            underlayer.SetPosition(pt)
                      .SetOffset({settings.bus_label_offset.first, settings.bus_label_offset.second})
                      .SetFontSize(settings.bus_label_font_size)
                      .SetFontFamily("Verdana")
                      .SetFontWeight("bold")
                      .SetData(bus->name)
                      .SetFillColor(settings.underlayer_color)
                      .SetStrokeColor(settings.underlayer_color)
                      .SetStrokeWidth(settings.underlayer_width)
                      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            
            svg::Text label;
            label.SetPosition(pt)
                 .SetOffset({settings.bus_label_offset.first, settings.bus_label_offset.second})
                 .SetFontSize(settings.bus_label_font_size)
                 .SetFontFamily("Verdana")
                 .SetFontWeight("bold")
                 .SetData(bus->name)
                 .SetFillColor(route_colors.at(bus->name));
            doc.AddPtr(std::make_unique<svg::Text>(underlayer));
            doc.AddPtr(std::make_unique<svg::Text>(label));
        }
    }
}

void RenderStopCircles(const catalogue::TransportCatalogue& catalogue,
                       const RenderSettings& settings,
                       svg::Document& doc,
                       const SphereProjector& projector) {
    std::unordered_set<std::string> stop_names;
    const auto& buses = catalogue.GetAllBuses();
    for (const auto& [bus_name, bus] : buses) {
        for (const auto& stop : bus->stops)
            stop_names.insert(stop);
    }
    std::vector<std::string> sorted_stops(stop_names.begin(), stop_names.end());
    std::sort(sorted_stops.begin(), sorted_stops.end());
    for (const auto& stop_name : sorted_stops) {
        const auto* stop = catalogue.GetStop(stop_name);
        if (!stop)
            continue;
        svg::Circle circle;
        circle.SetCenter(projector(stop->coordinates))
              .SetRadius(settings.stop_radius)
              .SetFillColor("white");
        doc.AddPtr(std::make_unique<svg::Circle>(circle));
    }
}

void RenderStopLabels(const catalogue::TransportCatalogue& catalogue,
                      const RenderSettings& settings,
                      svg::Document& doc,
                      const SphereProjector& projector) {
    std::unordered_set<std::string> stop_names;
    const auto& buses = catalogue.GetAllBuses();
    for (const auto& [bus_name, bus] : buses) {
        for (const auto& stop : bus->stops)
            stop_names.insert(stop);
    }
    std::vector<std::string> sorted_stops(stop_names.begin(), stop_names.end());
    std::sort(sorted_stops.begin(), sorted_stops.end());
    for (const auto& stop_name : sorted_stops) {
        const auto* stop = catalogue.GetStop(stop_name);
        if (!stop)
            continue;
        svg::Point pt = projector(stop->coordinates);
        
        svg::Text underlayer;
        underlayer.SetPosition(pt)
                  .SetOffset({settings.stop_label_offset.first, settings.stop_label_offset.second})
                  .SetFontSize(settings.stop_label_font_size)
                  .SetFontFamily("Verdana")
                  .SetData(stop_name)
                  .SetFillColor(settings.underlayer_color)
                  .SetStrokeColor(settings.underlayer_color)
                  .SetStrokeWidth(settings.underlayer_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text label;
        label.SetPosition(pt)
             .SetOffset({settings.stop_label_offset.first, settings.stop_label_offset.second})
             .SetFontSize(settings.stop_label_font_size)
             .SetFontFamily("Verdana")
             .SetData(stop_name)
             .SetFillColor("black");
        doc.AddPtr(std::make_unique<svg::Text>(underlayer));
        doc.AddPtr(std::make_unique<svg::Text>(label));
    }
}

void RenderMap(const catalogue::TransportCatalogue& catalogue, 
               const RenderSettings& settings, 
               std::ostream& out) {
    svg::Document svg_doc;

    const auto& bus_map = catalogue.GetAllBuses();
    std::unordered_set<std::string> all_stop_names;
    for (const auto& [bus_name, bus] : bus_map) {
        for (const auto& stop : bus->stops)
            all_stop_names.insert(stop);
    }
    std::vector<geo::Coordinates> geo_coords;
    for (const auto& stop_name : all_stop_names) {
        const auto* stop = catalogue.GetStop(stop_name);
        if (stop)
            geo_coords.push_back(stop->coordinates);
    }
    SphereProjector projector(geo_coords.begin(), geo_coords.end(),
                              settings.width, settings.height, settings.padding);
    
    std::vector<domain::Bus*> domain_buses;
    std::vector<const domain::Bus*> sorted_buses;
    for (const auto& [bus_name, bus] : bus_map)
        sorted_buses.push_back(bus);
    std::sort(sorted_buses.begin(), sorted_buses.end(), [](const domain::Bus* lhs, const domain::Bus* rhs) {
        return lhs->name < rhs->name;
    });
    for (const auto* bus : sorted_buses)
        domain_buses.push_back(const_cast<domain::Bus*>(bus));
    auto route_colors = AssignRouteColors(domain_buses, settings.color_palette);
    

    RenderRouteLines(catalogue, settings, svg_doc, projector, route_colors);
    RenderRouteLabels(catalogue, settings, svg_doc, projector, route_colors);
    RenderStopCircles(catalogue, settings, svg_doc, projector);
    RenderStopLabels(catalogue, settings, svg_doc, projector);
    
    svg_doc.Render(out);
}

} // namespace renderer
