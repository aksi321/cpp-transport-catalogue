#include "svg.h"
#include <string>
#include <iomanip>
#include <utility>

namespace svg {

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
    switch (line_cap) {
        case StrokeLineCap::BUTT: out << "butt"; break;
        case StrokeLineCap::ROUND: out << "round"; break;
        case StrokeLineCap::SQUARE: out << "square"; break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
    switch (line_join) {
        case StrokeLineJoin::ARCS: out << "arcs"; break;
        case StrokeLineJoin::BEVEL: out << "bevel"; break;
        case StrokeLineJoin::MITER: out << "miter"; break;
        case StrokeLineJoin::MITER_CLIP: out << "miter-clip"; break;
        case StrokeLineJoin::ROUND: out << "round"; break;
    }
    return out;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle "
        << "cx=\"" << center_.x << "\" "
        << "cy=\"" << center_.y << "\" "
        << "r=\"" << radius_ << "\"";
    RenderAttrs(out);
    out << "/>";
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\"";
    for (size_t i = 0; i < points_.size(); ++i) {
        out << points_[i].x << "," << points_[i].y;
        if (i + 1 < points_.size())
            out << " ";
    }
    out << "\"";
    RenderAttrs(out);
    out << "/>";
}

std::string EscapeXML(const std::string& data) {
    std::string result;
    result.reserve(data.size());
    for (char ch : data) {
        switch (ch) {
            case '&': result.append("&amp;"); break;
            case '"': result.append("&quot;"); break;
            case '\'': result.append("&apos;"); break;
            case '<': result.append("&lt;"); break;
            case '>': result.append("&gt;"); break;
            default: result.push_back(ch);
        }
    }
    return result;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text "
        << "x=\"" << pos_.x << "\" "
        << "y=\"" << pos_.y << "\" "
        << "dx=\"" << offset_.x << "\" "
        << "dy=\"" << offset_.y << "\" "
        << "font-size=\"" << font_size_ << "\" ";
    if (!font_family_.empty())
        out << "font-family=\"" << font_family_ << "\" ";
    if (!font_weight_.empty())
        out << "font-weight=\"" << font_weight_ << "\" ";
    RenderAttrs(out);
    out << ">" << EscapeXML(data_) << "</text>";
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;
    RenderContext ctx(out, 2, 2);
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }
    out << "</svg>";
}


}  // namespace svg
