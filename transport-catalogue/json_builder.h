#pragma once
#include "json.h"
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>
#include <utility>

namespace json {

class DictContext;
class ArrayContext;
class Builder {
public:
    Builder();

    // реализация value<T> теперь через контексты
    template <typename T>
    Builder& Value(T&& value) {
        CheckNotBuilt();
        Node* current = nodes_stack_.back();
        auto& curr_val = current->GetValue();
        if (std::holds_alternative<Dict>(curr_val)) {
            if (!pending_key_) {
                throw std::logic_error("Key must be set before value in a dictionary");
            }
            auto& dict = std::get<Dict>(curr_val);
            dict.emplace(*pending_key_, Node(std::forward<T>(value)));
            pending_key_.reset();
        } else if (std::holds_alternative<Array>(curr_val)) {
            auto& arr = std::get<Array>(curr_val);
            arr.push_back(Node(std::forward<T>(value)));
        } else if (std::holds_alternative<std::nullptr_t>(curr_val)) {
            *current = Node(std::forward<T>(value));
        } else {
            throw std::logic_error("Value() cannot be added in the current context");
        }
        return *this;
    }

    Builder& Key(const std::string& key);
    DictContext StartDict();
    Builder& EndDict();
    ArrayContext StartArray();
    Builder& EndArray();
    Node Build();

private:
    void CheckNotBuilt() const;

    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> pending_key_;
    bool built_;
};

//Добавлены вспомогательные классы контекстов для управления структурой
class DictContext {
public:
    explicit DictContext(Builder& builder) : builder_(builder) {}

    DictContext& Key(const std::string& key) { builder_.Key(key); return *this; }
    DictContext& Value(const Node& value) { builder_.Value(value); return *this; }
    DictContext& StartDict() { builder_.StartDict(); return *this; }
    ArrayContext StartArray();
    Builder& EndDict() { return builder_.EndDict(); }

private:
    Builder& builder_;
};

class ArrayContext {
public:
    explicit ArrayContext(Builder& builder) : builder_(builder) {}

    ArrayContext& Value(const Node& value) { builder_.Value(value); return *this; }
    DictContext StartDict();
    ArrayContext StartArray();
    Builder& EndArray() { return builder_.EndArray(); }

private:
    Builder& builder_;
};

} // namespace json
