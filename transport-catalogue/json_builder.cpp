#include "json_builder.h"

namespace json {

Builder::Builder() : built_(false) {
    nodes_stack_.push_back(&root_);
}

Builder& Builder::Key(const std::string& key) {
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

DictItemContext Builder::StartDict() {
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
    return DictItemContext(*this);
}

Builder& Builder::EndDict() {
    CheckNotBuilt();
    if (nodes_stack_.empty()) {
        throw std::logic_error("No dictionary to end");
    }
    nodes_stack_.pop_back();
    return *this;
}

ArrayItemContext Builder::StartArray() {
    CheckNotBuilt();
    Node* current = nodes_stack_.back();
    if (std::holds_alternative<Dict>(current->GetValue())) {
        if (!pending_key_) {
            throw std::logic_error("Key must be set before starting an array in a dictionary");
        }
        auto& dict = std::get<Dict>(current->GetValue());
        auto result = dict.emplace(*pending_key_, Node(Array{}));
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
    return ArrayItemContext(*this);
}

Builder& Builder::EndArray() {
    CheckNotBuilt();
    if (nodes_stack_.empty()) {
        throw std::logic_error("No array to end");
    }
    nodes_stack_.pop_back();
    return *this;
}

Node Builder::Build() {
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

void Builder::CheckNotBuilt() const {
    if (built_) {
        throw std::logic_error("JSON document already built");
    }
}

} // namespace json
