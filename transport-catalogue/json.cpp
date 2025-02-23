#include "json.h"
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

namespace json {

namespace {

void SkipSpaces(std::istream& input) {
    while (std::isspace(input.peek()))
        input.get();
}

Node LoadNumber(std::istream& input) {
    std::string num_str;
    if (input.peek() == '-') {
        num_str.push_back(static_cast<char>(input.get()));
    }
    while (std::isdigit(input.peek())) {
        num_str.push_back(static_cast<char>(input.get()));
    }
    bool is_int = true;
    if (input.peek() == '.') {
        is_int = false;
        num_str.push_back(static_cast<char>(input.get()));
        if (!std::isdigit(input.peek()))
            throw ParsingError("Expected digit after decimal point");
        while (std::isdigit(input.peek()))
            num_str.push_back(static_cast<char>(input.get()));
    }
    if (input.peek() == 'e' || input.peek() == 'E') {
        is_int = false;
        num_str.push_back(static_cast<char>(input.get()));
        if (input.peek() == '+' || input.peek() == '-') {
            num_str.push_back(static_cast<char>(input.get()));
        }
        if (!std::isdigit(input.peek()))
            throw ParsingError("Expected digit in exponent");
        while (std::isdigit(input.peek()))
            num_str.push_back(static_cast<char>(input.get()));
    }
    try {
        if (is_int) {
            int value = std::stoi(num_str);
            return Node(value);
        } else {
            double value = std::stod(num_str);
            return Node(value);
        }
    } catch (...) {
        throw ParsingError("Invalid number: " + num_str);
    }
}

Node LoadString(std::istream& input) {
    std::string result;
    while (true) {
        if (!input)
            throw ParsingError("Unexpected end of input in string");
        char c = input.get();
        if (c == '"')
            break;
        else if (c == '\\') {
            if (!input)
                throw ParsingError("Unexpected end after escape");
            char escaped = input.get();
            switch (escaped) {
                case 'n': result.push_back('\n'); break;
                case 'r': result.push_back('\r'); break;
                case 't': result.push_back('\t'); break;
                case '"': result.push_back('"'); break;
                case '\\': result.push_back('\\'); break;
                default:
                    throw ParsingError(std::string("Unknown escape sequence: \\") + escaped);
            }
        } else if (c == '\n' || c == '\r') {
            throw ParsingError("Unexpected end of line in string");
        } else {
            result.push_back(c);
        }
    }
    return Node(result);
}

Node LoadLiteral(std::istream& input) {
    std::string literal;
    while (std::isalpha(input.peek()))
        literal.push_back(static_cast<char>(input.get()));
    if (literal == "true")
        return Node(true);
    else if (literal == "false")
        return Node(false);
    else if (literal == "null")
        return Node(nullptr);
    else
        throw ParsingError("Unknown literal: " + literal);
}

Node LoadNode(std::istream& input);

Node LoadArray(std::istream& input) {
    Array arr;
    SkipSpaces(input);
    if (input.peek() == ']') {
        input.get();
        return Node(arr);
    }
    while (true) {
        SkipSpaces(input);
        arr.push_back(LoadNode(input));
        SkipSpaces(input);
        char c = input.get();
        if (c == ']')
            break;
        if (c != ',')
            throw ParsingError("Expected ',' or ']' in array");
    }
    return Node(arr);
}

Node LoadDict(std::istream& input) {
    std::map<std::string, Node> dict;
    SkipSpaces(input);
    if (input.peek() == '}') {
        input.get();
        return Node(dict);
    }
    while (true) {
        SkipSpaces(input);
        if (input.get() != '"')
            throw ParsingError("Expected '\"' at beginning of key");
        Node key_node = LoadString(input);
        std::string key = key_node.AsString();
        SkipSpaces(input);
        if (input.get() != ':')
            throw ParsingError("Expected ':' after key");
        SkipSpaces(input);
        Node value = LoadNode(input);
        dict.insert({std::move(key), std::move(value)});
        SkipSpaces(input);
        char c = input.get();
        if (c == '}')
            break;
        if (c != ',')
            throw ParsingError("Expected ',' or '}' in object");
    }
    return Node(dict);
}

Node LoadNode(std::istream& input) {
    SkipSpaces(input);
    char c = input.peek();
    if (c == '[') {
        input.get();
        return LoadArray(input);
    } else if (c == '{') {
        input.get();
        return LoadDict(input);
    } else if (c == '"') {
        input.get();
        return LoadString(input);
    } else if (std::isdigit(c) || c == '-') {
        return LoadNumber(input);
    } else if (std::isalpha(c)) {
        return LoadLiteral(input);
    } else {
        throw ParsingError("Unexpected character while parsing node");
    }
}

} // namespace

Document Load(std::istream& input) {
    Node root = LoadNode(input);
    return Document(root);
}


struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;
};

void PrintIndent(const PrintContext& ctx) {
    for (int i = 0; i < ctx.indent; ++i)
        ctx.out.put(' ');
}

void PrintNode(const Node& node, const PrintContext& ctx);

void PrintArray(const Array& arr, const PrintContext& ctx) {
    ctx.out << "[\n";
    PrintContext new_ctx { ctx.out, ctx.indent_step, ctx.indent + ctx.indent_step };
    bool first = true;
    for (const auto& elem : arr) {
        if (!first)
            new_ctx.out << ",\n";
        first = false;
        PrintIndent(new_ctx);
        PrintNode(elem, new_ctx);
    }
    ctx.out << "\n";
    PrintIndent(ctx);
    ctx.out << "]";
}

void PrintDict(const Dict& dict, const PrintContext& ctx) {
    ctx.out << "{\n";
    PrintContext new_ctx { ctx.out, ctx.indent_step, ctx.indent + ctx.indent_step };
    bool first = true;
    for (const auto& [key, value] : dict) {
        if (!first)
            new_ctx.out << ",\n";
        first = false;
        PrintIndent(new_ctx);
        new_ctx.out << "\"";
        for (char ch : key) {
            switch (ch) {
                case '"': new_ctx.out << "\\\""; break;
                case '\\': new_ctx.out << "\\\\"; break;
                case '\n': new_ctx.out << "\\n"; break;
                case '\r': new_ctx.out << "\\r"; break;
                case '\t': new_ctx.out << "\\t"; break;
                default: new_ctx.out << ch;
            }
        }
        new_ctx.out << "\": ";
        PrintNode(value, new_ctx);
    }
    ctx.out << "\n";
    PrintIndent(ctx);
    ctx.out << "}";
}

void PrintString(const std::string& s, const PrintContext& ctx) {
    (void)ctx;
    ctx.out << "\"";
    for (char ch : s) {
        switch (ch) {
            case '"': ctx.out << "\\\""; break;
            case '\\': ctx.out << "\\\\"; break;
            case '\n': ctx.out << "\\n"; break;
            case '\r': ctx.out << "\\r"; break;
            case '\t': ctx.out << "\\t"; break;
            default: ctx.out << ch;
        }
    }
    ctx.out << "\"";
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit([&ctx](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            ctx.out << "null";
        } else if constexpr (std::is_same_v<T, bool>) {
            ctx.out << (arg ? "true" : "false");
        } else if constexpr (std::is_same_v<T, int>) {
            ctx.out << arg;
        } else if constexpr (std::is_same_v<T, double>) {
            ctx.out << arg;
        } else if constexpr (std::is_same_v<T, std::string>) {
            PrintString(arg, ctx);
        } else if constexpr (std::is_same_v<T, Array>) {
            PrintArray(arg, ctx);
        } else if constexpr (std::is_same_v<T, Dict>) {
            PrintDict(arg, ctx);
        }
    }, node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintContext ctx { output, 4, 0 };
    PrintNode(doc.GetRoot(), ctx);
}

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}
bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

} // namespace json
