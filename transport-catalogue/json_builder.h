#pragma once

#include "json.h"
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>
#include <utility>

namespace json {

// Предварительные объявления классов контекстов
class DictItemContext;
class DictValueContext;
class ArrayItemContext;

class Builder {
public:
    Builder();

    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& Key(const std::string& key);
    template <typename T>
    Builder& Value(T&& value);
    Builder& EndDict();
    Builder& EndArray();
    Node Build();

private:
    // Доступ для контекстов
    friend class BaseContext;
    friend class DictItemContext;
    friend class DictValueContext;
    friend class ArrayItemContext;

    void CheckNotBuilt() const;

    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> pending_key_;
    bool built_;
};

// Базовый класс для всех контекстов: предоставляет метод Build()
class BaseContext {
public:
    explicit BaseContext(Builder& builder) : builder_(builder) {}
    Node Build();
protected:
    Builder& builder_;
};

// Контекст после вызова Key() — ожидается установка значения
class DictValueContext : public BaseContext {
public:
    using BaseContext::BaseContext;
    DictItemContext Value(const Node& value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();

    DictValueContext Key(const std::string& key) = delete;
    DictValueContext EndDict() = delete;
    ArrayItemContext EndArray() = delete;
};

// Контекст работы в словаре — после установки значения можно задать новый ключ или завершить словарь
class DictItemContext : public BaseContext {
public:
    using BaseContext::BaseContext;
    DictValueContext Key(const std::string& key);
    Builder& EndDict();
};

// Контекст работы в массиве — можно добавлять элементы, начинать вложенные структуры или завершать массив
class ArrayItemContext : public BaseContext {
public:
    using BaseContext::BaseContext;
    ArrayItemContext Value(const Node& value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
    ArrayItemContext Key(const std::string& key) = delete;
};

// BaseContext
inline Node BaseContext::Build() {
    return builder_.Build();
}

// DictValueContext
inline DictItemContext DictValueContext::Value(const Node& value) {
    builder_.Value(value);
    return DictItemContext(builder_);
}

inline DictItemContext DictValueContext::StartDict() {
    return builder_.StartDict();
}

inline ArrayItemContext DictValueContext::StartArray() {
    return builder_.StartArray();
}

// DictItemContext
inline DictValueContext DictItemContext::Key(const std::string& key) {
    builder_.Key(key);
    return DictValueContext(builder_);
}

inline Builder& DictItemContext::EndDict() {
    return builder_.EndDict();
}

// ArrayItemContext
inline ArrayItemContext ArrayItemContext::Value(const Node& value) {
    builder_.Value(value);
    return *this;
}

inline DictItemContext ArrayItemContext::StartDict() {
    return builder_.StartDict();
}

inline ArrayItemContext ArrayItemContext::StartArray() {
    return builder_.StartArray();
}

inline Builder& ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

// Реализация шаблонного метода Value
template <typename T>
Builder& Builder::Value(T&& value) {
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

} // namespace json
