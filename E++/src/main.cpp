#include <array>
#include <cctype>
#include <unordered_map>
#include <variant>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace epp {

using RuntimeValue = std::variant<std::nullptr_t, bool, double, std::string>;

std::string runtimeText(const RuntimeValue& value) {
    if (auto text = std::get_if<std::string>(&value)) return *text;
    if (auto number = std::get_if<double>(&value)) {
        std::ostringstream output;
        output << *number;
        return output.str();
    }
    if (auto boolean = std::get_if<bool>(&value)) return *boolean ? "true" : "false";
    return "null";
}

struct RuntimeToken {
    enum class Type { Number, String, Identifier, Equal, NotEqual, Less, Greater, Plus, Minus, Star, Slash, LeftParen, RightParen, End } type;
    std::string text;
    RuntimeValue value;
};

class RuntimeExpression {
public:
    RuntimeExpression(std::string source, std::unordered_map<std::string, RuntimeValue>& variables)
        : source_(std::move(source)), variables_(variables) {
        tokenize();
    }

    RuntimeValue evaluate() {
        position_ = 0;
        RuntimeValue value = comparison();
        if (current().type != RuntimeToken::Type::End) throw std::runtime_error("unexpected expression token");
        return value;
    }

private:
    std::string source_;
    std::unordered_map<std::string, RuntimeValue>& variables_;
    std::vector<RuntimeToken> tokens_;
    std::size_t position_ = 0;

    RuntimeToken current() const { return position_ < tokens_.size() ? tokens_[position_] : tokens_.back(); }
    RuntimeToken advance() { return tokens_[position_++]; }

    void tokenize() {
        std::size_t index = 0;
        while (index < source_.size()) {
            if (std::isspace(static_cast<unsigned char>(source_[index]))) {
                ++index;
            } else if (source_[index] == '"') {
                ++index;
                std::string text;
                while (index < source_.size() && source_[index] != '"') {
                    if (source_[index] == '\\' && index + 1 < source_.size()) ++index;
                    text += source_[index++];
                }
                if (index >= source_.size()) throw std::runtime_error("unterminated string in expression");
                ++index;
                tokens_.push_back({RuntimeToken::Type::String, text, text});
            } else if (std::isdigit(static_cast<unsigned char>(source_[index]))) {
                std::size_t start = index;
                while (index < source_.size() && (std::isdigit(static_cast<unsigned char>(source_[index])) || source_[index] == '.')) ++index;
                std::string number = source_.substr(start, index - start);
                tokens_.push_back({RuntimeToken::Type::Number, number, std::stod(number)});
            } else if (std::isalpha(static_cast<unsigned char>(source_[index])) || source_[index] == '_') {
                std::size_t start = index++;
                while (index < source_.size() && (std::isalnum(static_cast<unsigned char>(source_[index])) || source_[index] == '_')) ++index;
                std::string identifier = source_.substr(start, index - start);
                if (identifier == "true") tokens_.push_back({RuntimeToken::Type::Identifier, identifier, true});
                else if (identifier == "false") tokens_.push_back({RuntimeToken::Type::Identifier, identifier, false});
                else if (identifier == "null") tokens_.push_back({RuntimeToken::Type::Identifier, identifier, nullptr});
                else tokens_.push_back({RuntimeToken::Type::Identifier, identifier, identifier});
            } else {
                std::string two = source_.substr(index, 2);
                if (two == "==") { tokens_.push_back({RuntimeToken::Type::Equal, two, two}); index += 2; }
                else if (two == "!=") { tokens_.push_back({RuntimeToken::Type::NotEqual, two, two}); index += 2; }
                else if (source_[index] == '<') { tokens_.push_back({RuntimeToken::Type::Less, "<", "<"}); ++index; }
                else if (source_[index] == '>') { tokens_.push_back({RuntimeToken::Type::Greater, ">", ">"}); ++index; }
                else if (source_[index] == '+') { tokens_.push_back({RuntimeToken::Type::Plus, "+", "+"}); ++index; }
                else if (source_[index] == '-') { tokens_.push_back({RuntimeToken::Type::Minus, "-", "-"}); ++index; }
                else if (source_[index] == '*') { tokens_.push_back({RuntimeToken::Type::Star, "*", "*"}); ++index; }
                else if (source_[index] == '/') { tokens_.push_back({RuntimeToken::Type::Slash, "/", "/"}); ++index; }
                else if (source_[index] == '(') { tokens_.push_back({RuntimeToken::Type::LeftParen, "(", "("}); ++index; }
                else if (source_[index] == ')') { tokens_.push_back({RuntimeToken::Type::RightParen, ")", ")"}); ++index; }
                else throw std::runtime_error("invalid expression character: " + std::string(1, source_[index]));
            }
        }
        tokens_.push_back({RuntimeToken::Type::End, "", nullptr});
    }

    RuntimeValue primary() {
        RuntimeToken token = current();
        if (token.type == RuntimeToken::Type::Number || token.type == RuntimeToken::Type::String) { advance(); return token.value; }
        if (token.type == RuntimeToken::Type::Identifier) {
            advance();
            if (token.text == "true" || token.text == "false" || token.text == "null") return token.value;
            auto found = variables_.find(token.text);
            if (found == variables_.end()) throw std::runtime_error("unknown variable '" + token.text + "'");
            return found->second;
        }
        if (token.type == RuntimeToken::Type::LeftParen) { advance(); RuntimeValue value = comparison(); if (current().type != RuntimeToken::Type::RightParen) throw std::runtime_error("missing ')' in expression"); advance(); return value; }
        throw std::runtime_error("expected expression value");
    }

    RuntimeValue factor() {
        RuntimeValue left = primary();
        while (current().type == RuntimeToken::Type::Star || current().type == RuntimeToken::Type::Slash) {
            RuntimeToken operation = advance(); RuntimeValue right = primary();
            auto a = std::get_if<double>(&left); auto b = std::get_if<double>(&right);
            if (!a || !b) throw std::runtime_error("* and / require numbers");
            left = operation.type == RuntimeToken::Type::Star ? *a * *b : *a / *b;
        }
        return left;
    }

    RuntimeValue addition() {
        RuntimeValue left = factor();
        while (current().type == RuntimeToken::Type::Plus || current().type == RuntimeToken::Type::Minus) {
            RuntimeToken operation = advance(); RuntimeValue right = factor();
            if (operation.type == RuntimeToken::Type::Plus) {
                if (auto stringLeft = std::get_if<std::string>(&left); stringLeft && std::holds_alternative<std::string>(right)) left = *stringLeft + std::get<std::string>(right);
                else { auto numberLeft = std::get_if<double>(&left); auto numberRight = std::get_if<double>(&right); if (!numberLeft || !numberRight) throw std::runtime_error("+ requires strings or numbers"); left = *numberLeft + *numberRight; }
            } else {
                auto a = std::get_if<double>(&left); auto b = std::get_if<double>(&right); if (!a || !b) throw std::runtime_error("- requires numbers"); left = *a - *b;
            }
        }
        return left;
    }

    RuntimeValue comparison() {
        RuntimeValue left = addition();
        while (current().type == RuntimeToken::Type::Equal || current().type == RuntimeToken::Type::NotEqual || current().type == RuntimeToken::Type::Less || current().type == RuntimeToken::Type::Greater) {
            RuntimeToken operation = advance(); RuntimeValue right = addition();
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                double a = std::get<double>(left), b = std::get<double>(right);
                if (operation.type == RuntimeToken::Type::Equal) left = a == b; else if (operation.type == RuntimeToken::Type::NotEqual) left = a != b; else if (operation.type == RuntimeToken::Type::Less) left = a < b; else left = a > b;
            } else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                std::string a = std::get<std::string>(left), b = std::get<std::string>(right);
                left = operation.type == RuntimeToken::Type::Equal ? a == b : operation.type == RuntimeToken::Type::NotEqual ? a != b : operation.type == RuntimeToken::Type::Less ? a < b : a > b;
            } else {
                throw std::runtime_error("comparison requires matching string or number values");
            }
        }
        return left;
    }
};

class Runtime {
public:
    void run(const std::string& source, std::ostream& output) {
        std::istringstream input(source);
        std::string line;
        std::vector<std::string> lines;
        while (std::getline(input, line)) lines.push_back(line);
        std::size_t position = 0;
        executeBlock(lines, position, output);
        if (position != lines.size()) throw std::runtime_error("unexpected runtime block end");
    }
private:
    std::unordered_map<std::string, RuntimeValue> variables_;

    RuntimeValue expression(const std::string& text) { return RuntimeExpression(text, variables_).evaluate(); }
    bool isTruth(const RuntimeValue& value) {
        if (auto boolean = std::get_if<bool>(&value)) return *boolean;
        if (std::holds_alternative<std::nullptr_t>(value)) return false;
        if (auto number = std::get_if<double>(&value)) return *number != 0;
        if (auto text = std::get_if<std::string>(&value)) return !text->empty();
        return false;
    }
    std::string trimmed(std::string line) {
        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front()))) line.erase(line.begin());
        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back()))) line.pop_back();
        return line;
    }

    void executeBlock(const std::vector<std::string>& lines, std::size_t& position, std::ostream& output, bool stopAtElse = false) {
        while (position < lines.size()) {
            std::string line = trimmed(lines[position]);
            if (line.empty() || line[0] == '#') { ++position; continue; }
            if (line == "else" || line == "endif" || line == "endrepeat") return;
            std::vector<std::string> parts;
            std::istringstream runtimeLine(line);
            std::string runtimeToken;
            while (runtimeLine >> runtimeToken) parts.push_back(runtimeToken);
            std::string command = parts.empty() ? "" : parts[0];
            if (!command.empty() && command.back() == ':') command.pop_back();
            try {
                if (command == "let" && parts.size() >= 4 && parts[2] == "=") {
                    std::size_t equals = line.find('='); variables_[parts[1]] = expression(trimmed(line.substr(equals + 1))); ++position;
                } else if (command == "print") {
                    std::size_t space = line.find_first_of(" \t"); if (space == std::string::npos) throw std::runtime_error("print requires an expression"); output << runtimeText(expression(trimmed(line.substr(space + 1)))) << '\n'; ++position;
                } else if (command == "if") {
                    std::size_t space = line.find_first_of(" \t"); if (space == std::string::npos) throw std::runtime_error("if requires an expression"); bool truth = isTruth(expression(trimmed(line.substr(space + 1)))); ++position;
                    if (truth) executeBlock(lines, position, output, true); else skipBlock(lines, position, "endif");
                    if (position < lines.size() && trimmed(lines[position]) == "else") { ++position; if (!truth) executeBlock(lines, position, output, true); else skipBlock(lines, position, "endif"); }
                    if (position < lines.size() && trimmed(lines[position]) == "endif") ++position;
                } else if (command == "repeat") {
                    if (parts.size() < 2) throw std::runtime_error("repeat requires an expression"); std::size_t space = line.find_first_of(" \t"); RuntimeValue countValue = expression(trimmed(line.substr(space + 1))); auto count = std::get_if<double>(&countValue); if (!count || *count < 0) throw std::runtime_error("repeat count must be a non-negative number");
                    ++position; std::size_t bodyStart = position; for (double iteration = 0; iteration < *count; ++iteration) { position = bodyStart; executeBlock(lines, position, output, false); }
                    while (position < lines.size() && trimmed(lines[position]) != "endrepeat") ++position; if (position < lines.size()) ++position;
                } else {
                    throw std::runtime_error("runtime does not support statement '" + command + "'");
                }
            } catch (const std::exception& error) {
                throw std::runtime_error("line " + std::to_string(position + 1) + ": " + error.what());
            }
        }
    }

    void skipBlock(const std::vector<std::string>& lines, std::size_t& position, const std::string& terminator) {
        while (position < lines.size()) {
            std::string line = trimmed(lines[position]); ++position;
            if (line == terminator) return;
            if (line.rfind("if ", 0) == 0) skipBlock(lines, position, "endif");
        }
        throw std::runtime_error("missing endif block");
    }
};

} // namespace epp

#ifndef EPP_NO_MAIN
int main(int argc, char** argv) {
    std::string sourcePath;

    for (int index = 1; index < argc; ++index) {
        std::string argument = argv[index];
        if (argument == "run") {
            continue;
        } else if (argument == "--help" || argument == "-h") {
            std::cout << "Usage: epp run <source.epp>\n";
            return 0;
        } else if (argument.rfind("-", 0) == 0) {
            std::cerr << "epp: unknown option '" << argument << "'\n";
            return 1;
        } else {
            sourcePath = argument;
        }
    }

    if (sourcePath.empty()) {
        std::cerr << "epp: missing source file\nUsage: epp run <source.epp>\n";
        return 1;
    }

    try {
        std::ifstream sourceFile(sourcePath);
        if (!sourceFile) {
            throw std::runtime_error("cannot open source file: " + sourcePath);
        }
        std::ostringstream sourceBuffer;
        sourceBuffer << sourceFile.rdbuf();

        epp::Runtime runtime;
        runtime.run(sourceBuffer.str(), std::cout);
    } catch (const std::exception& error) {
        std::cerr << "epp: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
#endif
