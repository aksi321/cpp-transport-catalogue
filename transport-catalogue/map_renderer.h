#pragma once
#include <algorithm>
#include <string>
#include <algorithm>
#include <vector>
#include <utility>
#include <map>
#include "transport_catalogue.h"
#include "domain.h"
#include "svg.h"

namespace renderer {
    
    
    inline const double EPSILON = 1e-6;
    inline bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    inline double RoundToPrecision(double value, int precision = 6) {
    return std::round(value * std::pow(10, precision)) / std::pow(10, precision);
}


    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding)
            : padding_(padding) {
            if (points_begin == points_end) {
                return;
            }

            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](const geo::Coordinates &lhs, const geo::Coordinates &rhs) {
                    return lhs.lng < rhs.lng;
                });
            min_lon_ = left_it->lng;
            double max_lon = right_it->lng;

            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](const geo::Coordinates &lhs, const geo::Coordinates &rhs) {
                    return lhs.lat < rhs.lat;
                });
            double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            } else {
                zoom_coeff_ = 0;
            }
        }

        svg::Point operator()(const geo::Coordinates &coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

struct RenderSettings {
    double width = 1200.0;          
    double height = 1200.0;         
    double padding = 50.0;         

    double line_width = 14.0;       
    double stop_radius = 5.0;      

    int bus_label_font_size = 20;
    std::pair<double, double> bus_label_offset = {7.0, 15.0};

    int stop_label_font_size = 20;
    std::pair<double, double> stop_label_offset = {7.0, -3.0};

    std::string underlayer_color = "rgba(255,255,255,0.85)";
    double underlayer_width = 3.0;  

    std::vector<std::string> color_palette = {"green", "rgb(255,160,0)", "red"};
};

RenderSettings GetDefaultRenderSettings();
    
std::map<std::string, std::string> AssignRouteColors(
    const std::vector<domain::Bus*>& buses,
    const std::vector<std::string>& palette);
    
void RenderRouteLines(const catalogue::TransportCatalogue& catalogue,
                      const RenderSettings& settings,
                      svg::Document& doc);

void RenderMap(const catalogue::TransportCatalogue& catalogue, 
               const RenderSettings& settings, 
               std::ostream& out);

} // namespace renderer
