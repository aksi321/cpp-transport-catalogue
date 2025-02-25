#pragma once
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace json {

using Dict = std::map<std::string, class Node>;
using Array = std::vector<class Node>;

class ParsingError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Node : public std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    using std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>::variant;

    bool IsInt() const { return std::holds_alternative<int>(*this); }
    bool IsDouble() const { return std::holds_alternative<int>(*this) || std::holds_alternative<double>(*this); }
    bool IsPureDouble() const { return std::holds_alternative<double>(*this); }
    bool IsBool() const { return std::holds_alternative<bool>(*this); }
    bool IsString() const { return std::holds_alternative<std::string>(*this); }
    bool IsNull() const { return std::holds_alternative<std::nullptr_t>(*this); }
    bool IsArray() const { return std::holds_alternative<Array>(*this); }
    bool IsMap() const { return std::holds_alternative<Dict>(*this); }

    int AsInt() const {
        if (!IsInt())
            throw std::logic_error("Node is not an int");
        return std::get<int>(*this);
    }
    bool AsBool() const {
        if (!IsBool())
            throw std::logic_error("Node is not a bool");
        return std::get<bool>(*this);
    }
    double AsDouble() const {
        if (std::holds_alternative<int>(*this))
            return static_cast<double>(std::get<int>(*this));
        if (std::holds_alternative<double>(*this))
            return std::get<double>(*this);
        throw std::logic_error("Node is not a number");
    }
    const std::string& AsString() const {
        if (!IsString())
            throw std::logic_error("Node is not a string");
        return std::get<std::string>(*this);
    }
    const Array& AsArray() const {
        if (!IsArray())
            throw std::logic_error("Node is not an array");
        return std::get<Array>(*this);
    }
    const Dict& AsMap() const {
        if (!IsMap())
            throw std::logic_error("Node is not a map");
        return std::get<Dict>(*this);
    }
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
