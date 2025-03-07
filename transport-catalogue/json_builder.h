#pragma once
#include "json.h"
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>
#include <utility>

namespace json {

class Builder {
public:
    Builder() : built_(false) {
        nodes_stack_.push_back(&root_);
    }

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

    Builder& Key(const std::string& key) {
        CheckNotBuilt();
        Node* current = nodes_stack_.back();
        auto& curr_val = current->GetValue();
        if (!std::holds_alternative<Dict>(curr_val)) {
            throw std::logic_error("Key() can only be used inside a dictionary");
        }
        if (pending_key_) {
            throw std::logic_error("Previous key has no corresponding value");
        }
        pending_key_ = key;
        return *this;
    }

    Builder& StartDict() {
        CheckNotBuilt();
        Node* current = nodes_stack_.back();
        if (std::holds_alternative<Dict>(current->GetValue())) {
            if (!pending_key_) {
                throw std::logic_error("Key must be set before starting a dictionary in a dict");
            }
            auto& dict = std::get<Dict>(current->GetValue());
            auto result = dict.emplace(*pending_key_, Node(Dict{}));
            if (!result.second) {
                throw std::logic_error("Duplicate key insertion in dictionary");
            }
            pending_key_.reset();
            nodes_stack_.push_back(&result.first->second);
        } else if (std::holds_alternative<Array>(current->GetValue())) {
            auto& arr = std::get<Array>(current->GetValue());
            arr.push_back(Node(Dict{}));
            nodes_stack_.push_back(&arr.back());
        } else if (std::holds_alternative<std::nullptr_t>(current->GetValue())) {

            *current = Node(Dict{});
            nodes_stack_.push_back(current);
        } else {
            throw std::logic_error("StartDict() called in invalid context");
        }
        return *this;
    }

    Builder& EndDict() {
        CheckNotBuilt();
        Node* current = nodes_stack_.back();
        if (!std::holds_alternative<Dict>(current->GetValue())) {
            throw std::logic_error("EndDict() called but current container is not a dictionary");
        }
        if (pending_key_) {
            throw std::logic_error("A key was set but no value provided in dictionary");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& StartArray() {
        CheckNotBuilt();
        Node* current = nodes_stack_.back();
        if (std::holds_alternative<Dict>(current->GetValue())) {
            if (!pending_key_) {
                throw std::logic_error("Key must be set before starting an array in a dictionary");
            }
            auto& dict = std::get<Dict>(current->GetValue());
            auto result = dict.emplace(*pending_key_, Node(Array{}));
            if (!result.second) {
                throw std::logic_error("Duplicate key insertion in dictionary");
            }
            pending_key_.reset();
            nodes_stack_.push_back(&result.first->second);
        } else if (std::holds_alternative<Array>(current->GetValue())) {
            auto& arr = std::get<Array>(current->GetValue());
            arr.push_back(Node(Array{}));
            nodes_stack_.push_back(&arr.back());
        } else if (std::holds_alternative<std::nullptr_t>(current->GetValue())) {
            *current = Node(Array{});
            nodes_stack_.push_back(current);
        } else {
            throw std::logic_error("StartArray() called in invalid context");
        }
        return *this;
    }

    Builder& EndArray() {
        CheckNotBuilt();
        Node* current = nodes_stack_.back();
        if (!std::holds_alternative<Array>(current->GetValue())) {
            throw std::logic_error("EndArray() called but current container is not an array");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node Build() {
        CheckNotBuilt();
        if (nodes_stack_.size() != 1) {
            throw std::logic_error("Not all containers have been closed");
        }
        if (pending_key_) {
            throw std::logic_error("A key was set but no value provided");
        }
        built_ = true;
        return root_;
    }

private:
    void CheckNotBuilt() const {
        if (built_) {
            throw std::logic_error("JSON document already built");
        }
    }

    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> pending_key_;
    bool built_;
};

} // namespace json
