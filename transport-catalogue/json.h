#pragma once
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node() : value_(nullptr) {}
    Node(std::nullptr_t) : value_(nullptr) {}
    Node(Array array) : value_(std::move(array)) {}
    Node(Dict map) : value_(std::move(map)) {}
    Node(bool b) : value_(b) {}
    Node(int i) : value_(i) {}
    Node(double d) : value_(d) {}
    Node(const std::string & s) : value_(s) {}
    Node(std::string && s) : value_(std::move(s)) {}

    bool IsInt() const { return std::holds_alternative<int>(value_); }
    bool IsDouble() const { return std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_); }
    bool IsPureDouble() const { return std::holds_alternative<double>(value_); }
    bool IsBool() const { return std::holds_alternative<bool>(value_); }
    bool IsString() const { return std::holds_alternative<std::string>(value_); }
    bool IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
    bool IsArray() const { return std::holds_alternative<Array>(value_); }
    bool IsMap() const { return std::holds_alternative<Dict>(value_); }

    int AsInt() const {
        if (!IsInt())
            throw std::logic_error("Node is not an int");
        return std::get<int>(value_);
    }
    bool AsBool() const {
        if (!IsBool())
            throw std::logic_error("Node is not a bool");
        return std::get<bool>(value_);
    }
    double AsDouble() const {
        if (std::holds_alternative<int>(value_))
            return static_cast<double>(std::get<int>(value_));
        if (std::holds_alternative<double>(value_))
            return std::get<double>(value_);
        throw std::logic_error("Node is not a number");
    }
    const std::string& AsString() const {
        if (!IsString())
            throw std::logic_error("Node is not a string");
        return std::get<std::string>(value_);
    }
    const Array& AsArray() const {
        if (!IsArray())
            throw std::logic_error("Node is not an array");
        return std::get<Array>(value_);
    }
    const Dict& AsMap() const {
        if (!IsMap())
            throw std::logic_error("Node is not a map");
        return std::get<Dict>(value_);
    }

    const Value& GetValue() const { return value_; }

    bool operator==(const Node& other) const { return value_ == other.value_; }
    bool operator!=(const Node& other) const { return !(*this == other); }

private:
    Value value_;
};

class Document {
public:
    explicit Document(Node root) : root_(std::move(root)) {}
    const Node& GetRoot() const { return root_; }
private:
    Node root_;
};

bool operator==(const Document& lhs, const Document& rhs);
bool operator!=(const Document& lhs, const Document& rhs);

Document Load(std::istream& input);
void Print(const Document& doc, std::ostream& output);

}  // namespace json
