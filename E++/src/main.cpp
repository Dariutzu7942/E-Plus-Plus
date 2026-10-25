#include <array>
#include <cctype>
#include <cmath>
#include <unordered_map>
#include <variant>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace epp {

struct Element {
    std::string kind;
    std::vector<std::string> values;
};

struct Document {
    std::string title = "E++ App";
    std::vector<Element> elements;
};

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

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    bool quoted = false;
    bool escaped = false;

    for (char character : line) {
        if (escaped) {
            token += character;
            escaped = false;
        } else if (character == '\\' && quoted) {
            escaped = true;
        } else if (character == '"') {
            quoted = !quoted;
        } else if (!quoted && character == '#') {
            break;
        } else if (!quoted && std::isspace(static_cast<unsigned char>(character))) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += character;
        }
    }

    if (quoted) {
        throw std::runtime_error("unterminated quoted value");
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string lower(std::string value) {
    for (char& character : value) {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return value;
}

std::string urlScheme(const std::string& url) {
    const std::size_t separator = url.find(':');
    if (separator == std::string::npos || url.find('/') < separator || url.find('?') < separator || url.find('#') < separator) {
        return "";
    }
    return lower(url.substr(0, separator));
}

std::string htmlEscape(const std::string& value, bool quote = false) {
    std::ostringstream output;
    for (char character : value) {
        switch (character) {
            case '&': output << "&amp;"; break;
            case '<': output << "&lt;"; break;
            case '>': output << "&gt;"; break;
            case '"': output << (quote ? "&quot;" : "\""); break;
            case '\'': output << (quote ? "&#39;" : "'"); break;
            default: output << character; break;
        }
    }
    return output.str();
}

Document parse(const std::string& source) {
    Document document;
    std::istringstream input(source);
    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(input, line)) {
        ++lineNumber;
        std::vector<std::string> parts;
        try {
            parts = tokenize(line);
        } catch (const std::runtime_error& error) {
            throw std::runtime_error("line " + std::to_string(lineNumber) + ": " + error.what());
        }
        if (parts.empty()) {
            continue;
        }

        std::string command = parts[0];
        if (!command.empty() && command.back() == ':') {
            command.pop_back();
        }
        if (command == "title" && parts.size() == 2) {
            document.title = parts[1];
        } else if (command == "heading" && parts.size() == 2) {
            document.elements.push_back({"heading", {parts[1]}});
        } else if (command == "text" && parts.size() == 2) {
            document.elements.push_back({"text", {parts[1]}});
        } else if (command == "button" && parts.size() == 3) {
            document.elements.push_back({"button", {parts[1], parts[2]}});
        } else if (command == "link" && parts.size() == 3) {
            const std::string scheme = urlScheme(parts[2]);
            if (scheme != "" && scheme != "http" && scheme != "https" && scheme != "mailto") {
                throw std::runtime_error("line " + std::to_string(lineNumber) + ": unsupported link URL scheme");
            }
            document.elements.push_back({"link", {parts[1], parts[2]}});
        } else {
            throw std::runtime_error("line " + std::to_string(lineNumber) + ": invalid statement or arguments for '" + command + "'");
        }
    }
    return document;
}

std::string render(const Document& document) {
    std::ostringstream output;
    std::string body;
    for (const Element& element : document.elements) {
        if (element.kind == "heading") {
            body += "      <h1>" + htmlEscape(element.values[0]) + "</h1>\n";
        } else if (element.kind == "text") {
            body += "      <p>" + htmlEscape(element.values[0]) + "</p>\n";
        } else if (element.kind == "button") {
            body += "      <button type=\"button\" data-message=\"" + htmlEscape(element.values[1], true) + "\">" + htmlEscape(element.values[0]) + "</button>\n";
        } else if (element.kind == "link") {
            body += "      <a href=\"" + htmlEscape(element.values[1], true) + "\">" + htmlEscape(element.values[0]) + "</a>\n";
        }
    }

    output << "<!doctype html>\n"
           << "<html lang=\"en\">\n"
           << "  <head>\n"
           << "    <meta charset=\"utf-8\">\n"
           << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
           << "    <title>" << htmlEscape(document.title) << "</title>\n"
           << "    <style>\n"
           << "      :root { color-scheme: light; font-family: \"Trebuchet MS\", sans-serif; color: #183b35; background: #f3f6ed; }\n"
           << "      body { box-sizing: border-box; max-width: 760px; min-height: 100vh; margin: 0 auto; padding: 12vh 24px; }\n"
           << "      main { border-left: 5px solid #e15a36; padding: 8px 0 8px 28px; }\n"
           << "      h1 { font-family: Georgia, serif; font-size: 2.5rem; margin: 0 0 16px; }\n"
           << "      p { line-height: 1.6; max-width: 52ch; }\n"
           << "      button, a { display: inline-block; margin: 12px 12px 0 0; border: 0; border-radius: 4px; padding: 11px 16px; background: #176b5b; color: white; font: inherit; text-decoration: none; cursor: pointer; }\n"
           << "      a { background: #e15a36; }\n"
           << "      button:focus-visible, a:focus-visible { outline: 3px solid #183b35; outline-offset: 3px; }\n"
           << "    </style>\n"
           << "  </head>\n"
           << "  <body>\n"
           << "    <main>\n"
           << body
           << "    </main>\n"
           << "    <script>\n"
           << "      document.addEventListener(\"click\", (event) => {\n"
           << "        const button = event.target.closest(\"button[data-message]\");\n"
           << "        if (button) window.alert(button.dataset.message);\n"
           << "      });\n"
           << "    </script>\n"
           << "  </body>\n"
           << "</html>\n";
    return output.str();
}

} // namespace epp

#ifndef EPP_NO_MAIN
int main(int argc, char** argv) {
    std::string outputPath;
    std::string sourcePath;
    bool runMode = false;

    for (int index = 1; index < argc; ++index) {
        std::string argument = argv[index];
        if (argument == "run") {
            runMode = true;
        } else if (argument == "--out") {
            if (++index >= argc) {
                std::cerr << "epp: --out requires a path\n";
                return 1;
            }
            outputPath = argv[index];
        } else if (argument == "--help" || argument == "-h") {
            std::cout << "Usage: epp <source.epp> [--out <output.html>]\n       epp run <source.epp>\n";
            return 0;
        } else if (argument.rfind("-", 0) == 0) {
            std::cerr << "epp: unknown option '" << argument << "'\n";
            return 1;
        } else {
            sourcePath = argument;
        }
    }

    if (sourcePath.empty()) {
        std::cerr << "epp: missing source file\nUsage: epp <source.epp> [--out <output.html>]\n       epp run <source.epp>\n";
        return 1;
    }

    try {
        std::ifstream sourceFile(sourcePath);
        if (!sourceFile) {
            throw std::runtime_error("cannot open source file: " + sourcePath);
        }
        std::ostringstream sourceBuffer;
        sourceBuffer << sourceFile.rdbuf();

        if (runMode) {
            epp::Runtime runtime;
            runtime.run(sourceBuffer.str(), std::cout);
            return 0;
        }

        epp::Document document = epp::parse(sourceBuffer.str());
        if (outputPath.empty()) {
            std::filesystem::path source(sourcePath);
            outputPath = source.replace_extension(".html").string();
        }

        std::ofstream outputFile(outputPath);
        if (!outputFile) {
            throw std::runtime_error("cannot write output file: " + outputPath);
        }
        outputFile << epp::render(document);
        std::cout << "Built " << outputPath << "\n";
    } catch (const std::exception& error) {
        std::cerr << "epp: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
#endif
