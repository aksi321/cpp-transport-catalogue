#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace svg {

using Color = std::string;
inline const Color NoneColor{"none"};

struct Rgba {
    int red;
    int green;
    int blue;
    double opacity;
};

inline std::ostream& operator<<(std::ostream& os, const Rgba& color) {
    os << "rgba(" 
       << color.red << ","
       << color.green << ","
       << color.blue << ","
       << std::defaultfloat << color.opacity
       << ")";
    return os;
}

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap);

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join);

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x), y(y) {
    }
    double x = 0;
    double y = 0;
};

struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out), indent_step(0), indent(0) {
    }
    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out), indent_step(indent_step), indent(indent) {
    }
    RenderContext Indented() const {
        return { out, indent_step, indent + indent_step };
    }
    void RenderIndent() const {
        for (int i = 0; i < indent; ++i)
            out.put(' ');
    }
    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

class Object {
public:
    void Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << std::endl;
    }
    virtual ~Object() = default;
private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_line_cap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_line_join_ = line_join;
        return AsOwner();
    }
protected:
    ~PathProps() = default;
    void RenderAttrs(std::ostream& out) const {
        if (fill_color_)
            out << " fill=\"" << *fill_color_ << "\"";
        if (stroke_color_)
            out << " stroke=\"" << *stroke_color_ << "\"";
        if (stroke_width_)
            out << " stroke-width=\"" << *stroke_width_ << "\"";
        if (stroke_line_cap_)
            out << " stroke-linecap=\"" << *stroke_line_cap_ << "\"";
        if (stroke_line_join_)
            out << " stroke-linejoin=\"" << *stroke_line_join_ << "\"";
    }
private:
    Owner& AsOwner() { return static_cast<Owner&>(*this); }
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};

class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center) {
        center_ = center;
        return *this;
    }
    Circle& SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }
private:
    void RenderObject(const RenderContext& context) const override;
    Point center_;
    double radius_ = 1.0;
};

class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline& AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }
private:
    void RenderObject(const RenderContext& context) const override;
    std::vector<Point> points_;
};

class Text final : public Object, public PathProps<Text> {
public:
    Text& SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }
    Text& SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }
    Text& SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }
    Text& SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }
    Text& SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }
    Text& SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }
private:
    void RenderObject(const RenderContext& context) const override;
    Point pos_{0, 0};
    Point offset_{0, 0};
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};

std::string EscapeXML(const std::string& data);

class Drawable {
public:
    virtual void Draw(class ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};

class ObjectContainer {
public:
    template <typename T>
    void Add(T obj) {
        AddPtr(std::make_unique<T>(std::move(obj)));
    }
    virtual ~ObjectContainer() = default;
protected:
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
};

class Document final : public ObjectContainer {
public:
    void AddPtr(std::unique_ptr<Object>&& obj) override;
    void Render(std::ostream& out) const;
private:
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg
